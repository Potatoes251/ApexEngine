#include "HudEditor.h"
#include "EditorIcon.h"
#include "AssetPicker.h"

using namespace Apex::UserInterface;

namespace Apex::Editor
{

    HudEditor::HudEditor(IGUI* gui,
        Apex::Resources::ResourceManager* resourceManager, Apex::Rendering::IRHI* rhi)
        : m_gui(gui), m_resourceManager(resourceManager), m_rhi(rhi)
    {
    }

    void HudEditor::Open(const Fs::path& path, int instanceIndex)
    {
        m_path = path;
        m_instanceIndex = instanceIndex;
        m_open = true;
        m_dirty = false;
        m_selection = {};
        m_canvas.Clear();

        if (Fs::exists(path)) m_canvas.Load(path, *m_resourceManager);
        ScanTextures();
    }

    void HudEditor::Close() { m_open = false; }
    void HudEditor::Focus() { m_pendingFocus = true; }

    void HudEditor::Draw()
    {
        if (!m_open) return;

        m_gui->PushFont(FontID::Default);

        constexpr float CASCADE = 30.f;
        float offset = static_cast<float>(m_instanceIndex) * CASCADE;
        LibMath::Vector2 screen = m_gui->GetScreenSize();
        m_gui->SetNextWindowPos(LibMath::Vector2(offset, offset));
        m_gui->SetNextWindowSize(LibMath::Vector2(screen[0] - offset, screen[1] - offset));

        if (m_pendingFocus) 
        { 
            m_gui->SetNextWindowFocus(); 
            m_pendingFocus = false; 
        }

        std::string title = "HUD Editor - " + m_path.stem().string() + (m_dirty ? " *" : "");

        m_gui->BeginPanel(title, &m_open, false, true);

        if (!m_open)
        {
            Close();
            m_gui->EndPanel();
            m_gui->PopFont();
            return;
        }

        DrawToolbar();
        m_gui->Separator();

        float availableWidth = m_gui->GetAvailableSize()[0];
        float availableHeight = m_gui->GetAvailableSize()[1];

        float spacing = 4.f;
        float canvasWidth = availableWidth - TREE_WIDTH - PROPS_WIDTH - spacing * 4.f - 8.f;
        if (canvasWidth < 100.f) canvasWidth = 100.f;

        m_gui->BeginChildPanel("##he_tree", TREE_WIDTH, availableHeight, true);
        DrawWidgetTree();
        m_gui->EndChildPanel();

        m_gui->SameLine(0.f, spacing);

        m_gui->BeginChildPanel("##he_canvas", canvasWidth, availableHeight, true);
        DrawCanvas();
        m_gui->EndChildPanel();

        m_gui->SameLine(0.f, spacing);

        m_gui->BeginChildPanel("##he_props", PROPS_WIDTH, availableHeight, true);
        DrawProperties();
        m_gui->EndChildPanel();

        if (m_showTexturePicker) DrawTexturePicker();

        ApplyPendingTreeOp();

        m_gui->EndPanel();
        m_gui->PopFont();
    }

    void HudEditor::DrawToolbar()
    {
        m_gui->PushStyleVariable(StyleVariable::FramePadding, 6.f, 4.f);

        DrawToolbarSave();
        m_gui->SameLine(0.f, 8.f);
        DrawToolbarLoad();

        m_gui->SameLine(0.f, 16.f);
        m_gui->VerticalSeparator();
        m_gui->SameLine(0.f, 16.f);

        m_gui->Text("Add:");
        m_gui->SameLine(0.f, 12.f);

        struct WType 
        { 
            const char* label; 
            UIWidgetType type; 
        };

        WType types[] = 
        {
            {"Text",UIWidgetType::Text},{"Image",UIWidgetType::Image},
            {"Button",UIWidgetType::Button},{"ProgressBar",UIWidgetType::ProgressBar},
            {"Panel",UIWidgetType::Panel}
        };
        for (auto& t : types)
        {
            if (m_gui->Button(t.label)) AddWidget(t.type);
            m_gui->SameLine(0.f, 8.f);
        }

        if (m_selection.IsValid() && m_gui->IsKeyPressed(UIKey::Delete)) DeleteSelected();

        m_gui->PopStyleVariable();
    }

    void HudEditor::DrawToolbarSave()
    {
        uint32_t saveIcon = EditorIcon::Get("ApexAssets/Icons/save.png");
        if (saveIcon != 0)
        {
            if (m_gui->ImageButton("##save", saveIcon, LibMath::Vector2(22.f, 22.f)))
            {
                if (m_canvas.Save(m_path))
                    m_dirty = false;
            }
        }
        else
        {
            if (m_gui->Button("Save"))
            {
                if (m_canvas.Save(m_path))
                    m_dirty = false;
            }
        }
        if (m_gui->IsItemHovered())
        {
            m_gui->BeginTooltip();
            m_gui->Text("Save Canvas...");
            m_gui->EndTooltip();
        }
    }

    void HudEditor::DrawToolbarLoad()
    {
        uint32_t loadIcon = EditorIcon::Get("ApexAssets/Icons/load.png");
        if (loadIcon != 0)
        {
            if (m_gui->ImageButton("##load", loadIcon, LibMath::Vector2(22.f, 22.f)))
            {
                m_canvas.Load(m_path, *m_resourceManager);
                m_selection = {};
                m_dirty = false;
            }
        }
        else
        {
            if (m_gui->Button("Load"))
            {
                m_canvas.Load(m_path, *m_resourceManager);
                m_selection = {};
                m_dirty = false;
            }
        }
        if (m_gui->IsItemHovered())
        {
            m_gui->BeginTooltip();
            m_gui->Text("Load Canvas...");
            m_gui->EndTooltip();
        }
    }

    void HudEditor::DrawCanvas()
    {
        LibMath::Vector2 panelPos = m_gui->GetPanelPos();
        LibMath::Vector2 panelSize = m_gui->GetAvailableSize();

        float canvasWidth = panelSize[0];
        float canvasHeight = canvasWidth / CANVAS_ASPECT;
        if (canvasHeight > panelSize[1]) 
        { 
            canvasHeight = panelSize[1]; 
            canvasWidth = canvasHeight * CANVAS_ASPECT; 
        }

        m_canvasSize = { canvasWidth, canvasHeight };

        float offX = panelPos[0] + (panelSize[0] - canvasWidth) * 0.5f;
        float offY = panelPos[1] + (panelSize[1] - canvasHeight) * 0.5f;
        m_canvasMin = { offX, offY };

        m_gui->SetCursorPos(panelPos);
        m_gui->InvisibleButton("##canvas", panelSize);

        IDrawList* drawList = m_gui->GetDrawList();

        LibMath::Vector2 drawMin = m_canvasMin;
        LibMath::Vector2 drawMax = 
        {
            drawMin[0] + canvasWidth, drawMin[1] + canvasHeight
        };

        drawList->DrawRectFilled(drawMin, drawMax, 0xFF1A1A1A);
        m_canvas.PrepareForRender(m_rhi);

        size_t count = m_canvas.GetElementCount();
        for (size_t i = 0; i < count; ++i)
        {
            UIElement* element = m_canvas.GetElement(i);
            if (!element || !element->m_visible) continue;

            WidgetSelection thisSelection{ nullptr, static_cast<int>(i) };

            UIDrawContext context;
            context.m_drawList = drawList;
            context.m_screenSize = m_canvasSize;
            context.m_origin = { drawMin[0], drawMin[1] };
            context.m_isEditor = true;
            context.m_selected = (m_selection == thisSelection);

            element->Draw(context);
        }
        UIElement* selected = ResolveSelected();
        if (selected)
        {
            LibMath::Vector2 anchorOrigin = AnchorOrigin(selected->m_anchor);
            LibMath::Vector2 anchorSelect = { drawMin[0] + anchorOrigin[0], drawMin[1] + anchorOrigin[1] };
            drawList->DrawRectFilled({ anchorSelect[0] - 4,anchorSelect[1] - 4 }, { anchorSelect[0] + 4,anchorSelect[1] + 4 }, 0xFF00FFFF, 2.f);
        }
        drawList->DrawRect(drawMin, drawMax, 0xFF555555, 0.f, 1.f);
        HandleCanvasInput();
    }

    void HudEditor::HandleCanvasInput()
    {
        bool active = m_gui->IsItemActive();
        bool clicked = m_gui->IsItemClicked();

        LibMath::Vector2 mouse = m_gui->GetMousePos();

        if (clicked)
        {
            m_selection = {};
            m_dragging = false;
            int count = static_cast<int>(m_canvas.GetElementCount());
            for (int i = count - 1; i >= 0; --i)
            {
                UIElement* element = m_canvas.GetElement(i);
                if (!element || !element->m_visible) continue;

                LibMath::Vector2 screenPos = WidgetToCanvas(*element);
                LibMath::Vector2 size = 
                {
                    element->m_size[0] * m_canvasSize[0],
                    element->m_size[1] * m_canvasSize[1] 
                };

                if (mouse[0] >= screenPos[0] && mouse[0] <= screenPos[0] + size[0] &&
                    mouse[1] >= screenPos[1] && mouse[1] <= screenPos[1] + size[1])
                {
                    m_selection = { nullptr, i };
                    m_dragging = true;
                    m_dragStartMouse = mouse;
                    m_dragStartPos = element->m_position;
                    break;
                }
            }
        }

        if (active && m_dragging && m_selection.IsValid())
        {
            UIElement* element = ResolveSelected();
            if (element)
            {
                LibMath::Vector2 delta = 
                {
                    (mouse[0] - m_dragStartMouse[0]) / (m_canvasSize[0]),
                    (mouse[1] - m_dragStartMouse[1]) / (m_canvasSize[1]) 
                };
                LibMath::Vector2 newPos = 
                {
                    m_dragStartPos[0] + delta[0],
                    m_dragStartPos[1] + delta[1]
                };
                LibMath::Vector2 posOffset = { newPos[0] - element->m_position[0], newPos[1] - element->m_position[1] };

                element->m_position = newPos;

                if (UIPanel* panel = dynamic_cast<UIPanel*>(element))
                {
                    for (auto& child : panel->m_children)
                        if (child)
                        {
                            child->m_position[0] += posOffset[0];
                            child->m_position[1] += posOffset[1];
                        }
                }

                m_dirty = true;
            }
        }

        if (!active) m_dragging = false;
    }

    void HudEditor::DrawWidgetTree()
    {
        m_gui->PushFont(FontID::Large);
        m_gui->Text("Hierarchy");
        m_gui->PopFont();
        m_gui->Separator();

        m_showInsertLine = false;

        for (size_t i = 0; i < m_canvas.GetElementCount(); ++i)
            DrawTreeElement(m_canvas.GetElement(i), static_cast<int>(i), nullptr);

        LibMath::Vector2 availableSize = m_gui->GetAvailableSize();
        float dummyHeight = (availableSize[1] > 40.f) ? availableSize[1] : 40.f;
        m_gui->Dummy({ availableSize[0], dummyHeight });

        if (m_gui->BeginDropTarget())
        {
            if (m_gui->AcceptDragPayload("WIDGET_TREE"))
            {
                m_pendingOp.kind = PendingTreeOp::Kind::Reparent;
                m_pendingOp.dstParent = nullptr;
                m_pendingOp.dstIndex = -1;
            }
            m_gui->EndDropTarget();
        }

        if (m_showInsertLine)
        {
			DrawInsertLine(m_insertLineY);
        }
    }

    void HudEditor::DrawTreeElement(UIElement* element, size_t index, UIPanel* parent)
    {
        if (!element) return;

        UIPanel* asPanel = dynamic_cast<UIPanel*>(element);
        WidgetSelection thisSelection{ parent, static_cast<int>(index) };

        TreeNodeFlags flags = GetTreeNodeFlags(element, thisSelection);
        std::string label = ShortenName(element->m_name) + "##" + std::to_string(reinterpret_cast<uintptr_t>(element));

        bool nodeOpen = m_gui->BeginTreeNode(label, flags);

        HandleTreeInteraction(element, index, parent, asPanel, thisSelection);

        if (nodeOpen)
        {
            if (asPanel)
            {
                for (int ci = 0; ci < static_cast<int>(asPanel->m_children.size()); ++ci)
                    DrawTreeElement(asPanel->m_children[ci].get(), ci, asPanel);
            }
            m_gui->EndTreeNode();
        }
    }

    TreeNodeFlags HudEditor::GetTreeNodeFlags(UIElement* element, const WidgetSelection& selection)
    {
        UIPanel* asPanel = dynamic_cast<UIPanel*>(element);
        bool hasChildren = asPanel && !asPanel->m_children.empty();

        TreeNodeFlags flags = TreeNodeFlags::OpenOnArrow | TreeNodeFlags::SpanAvailWidth;
        if (m_selection == selection) flags = flags | TreeNodeFlags::Selected;
        if (!hasChildren)             flags = flags | TreeNodeFlags::Leaf;

        return flags;
    }

    void HudEditor::HandleTreeInteraction(UIElement* element, size_t index, UIPanel* parent, UIPanel* asPanel, const WidgetSelection& selection)
    {
        if (m_gui->IsItemClicked() && !m_gui->IsDragDropActive())
            m_selection = selection;

        if (m_gui->IsDragDropActive() && m_gui->IsItemHovered())
        {
            LibMath::Vector2 minRect = m_gui->GetItemRectMin();
            LibMath::Vector2 maxRect = m_gui->GetItemRectMax();
            float mouseY = m_gui->GetMousePos()[1];
            float height = maxRect[1] - minRect[1];

            if (mouseY < minRect[1] + height * 0.25f)
            {
                m_showInsertLine = true;
                m_insertLineY = minRect[1];
            }
            else if (mouseY > maxRect[1] - height * 0.25f)
            {
                m_showInsertLine = true;
                m_insertLineY = maxRect[1];
            }
        }

        // Drag Logic
        if (m_gui->BeginDragSource())
        {
            m_pendingOp.srcParent = parent;
            m_pendingOp.srcIndex = index;
            m_gui->SetDragPayload("WIDGET_TREE", &index, sizeof(int));
            m_gui->Text(element->m_name.c_str());
            m_gui->EndDragSource();
        }

        // Drop Logic
        if (m_gui->BeginDropTarget())
        {
            HandleDropOnElement(index, parent, asPanel);
            m_gui->EndDropTarget();
        }
    }

    void HudEditor::HandleDropOnElement(size_t index, UIPanel* parent, UIPanel* asPanel)
    {
        if (!m_gui->AcceptDragPayload("WIDGET_TREE")) return;

        LibMath::Vector2 minRect = m_gui->GetItemRectMin();
        LibMath::Vector2 maxRect = m_gui->GetItemRectMax();
        float mouseY = m_gui->GetMousePos()[1];
        float height = maxRect[1] - minRect[1];

        if (mouseY < minRect[1] + height * 0.25f)
        {
            m_pendingOp.kind = PendingTreeOp::Kind::ReorderBefore;
            m_pendingOp.dstParent = parent;
            m_pendingOp.dstIndex = index;
        }
        else if (mouseY > maxRect[1] - height * 0.25f || !asPanel)
        {
            m_pendingOp.kind = PendingTreeOp::Kind::ReorderAfter;
            m_pendingOp.dstParent = parent;
            m_pendingOp.dstIndex = index;
        }
        else if (asPanel)
        {
            m_pendingOp.kind = PendingTreeOp::Kind::Reparent;
            m_pendingOp.dstParent = asPanel;
            m_pendingOp.dstIndex = -1;
        }
    }

    void HudEditor::DrawInsertLine(float y)
    {
        IDrawList* drawList = m_gui->GetDrawList();
        LibMath::Vector2 panelPos = m_gui->GetPanelPos();
        LibMath::Vector2 panelSize = m_gui->GetPanelSize();
        drawList->AddLine({ panelPos[0], y }, { panelPos[0] + panelSize[0], y }, 0xFF50A0FF, 2.f);
    }

    void HudEditor::DrawTexturePicker()
    {
        if (!m_texPickerOpened)
        {
            m_gui->OpenPopup("Pick Texture");
            m_texPickerOpened = true;
        }

        if (!m_gui->BeginPopupModal("Pick Texture", { 540.f, 420.f }))
            return;

        m_gui->Text("Click a texture to select:");
        m_gui->Separator();

        if (m_textureFiles.empty())
        {
            m_gui->TextDisabled("No textures found in Assets/");
        }
        else
        {
            int cols = Apex::Resources::PickerColumnCount(m_gui->GetAvailableSize()[0]);

            std::string picked = DrawTextureGrid(cols);
            if (!picked.empty())
            {
                CommitTextureSelection(picked);
                m_showTexturePicker = false;
                m_texPickerOpened = false;
                m_gui->CloseCurrentPopup();
                m_gui->EndPopup();
                return;
            }
        }

        m_gui->Separator();
        if (m_gui->Button("Cancel##tpcancel"))
        {
            m_showTexturePicker = false;
            m_texPickerOpened = false;
            m_gui->CloseCurrentPopup();
        }
        m_gui->EndPopup();
    }

    std::string HudEditor::DrawTextureGrid(int cols)
    {
        m_gui->BeginChildPanel("##tp_grid", 0.f, -50.f, false);

        int col = 0;
        for (const auto& path : m_textureFiles)
        {
            std::string key = path.generic_string();

            if (col > 0 && col % cols != 0)
                m_gui->SameLine(0.f, Apex::Resources::PICKER_SPACING);

            m_gui->PushID(key);
            uint32_t tid = EditorIcon::Get(key);
            bool picked = Apex::Resources::DrawPickerCell(m_gui, key, tid);
            m_gui->PopID();

            if (picked)
            {
                m_gui->EndChildPanel();
                return key;
            }
            col++;
        }

        m_gui->EndChildPanel();
        return {};
    }

    void HudEditor::CommitTextureSelection(const std::string& key)
    {
        UIElement* sel = ResolveSelected();
        UIImage* img = sel ? dynamic_cast<UIImage*>(sel) : nullptr;
        if (!img) return;

        img->m_texturePath = key;
        img->m_glTexID = 0;
        img->LoadTexture(*m_resourceManager);
        m_dirty = true;
    }

    void HudEditor::DrawProperties()
    {
        m_gui->PushFont(FontID::Large);
        m_gui->Text("Properties");
        m_gui->PopFont();
        m_gui->Separator();

        UIElement* element = ResolveSelected();
        if (!element) 
        { 
            m_gui->TextDisabled("No widget selected"); 
            return; 
        }

        m_gui->PushStyleVariable(StyleVariable::ItemSpacing, 4.f, 6.f);
       
        DrawBaseProperties(element);
        m_gui->Separator();
        DrawTypeSpecificProperties(element);

        m_gui->PopStyleVariable();
    }

    void HudEditor::DrawRow(const char* label)
    {
        m_gui->AlignTextToFramePadding();
        m_gui->Text(label);
        m_gui->SameLine(LABEL_WIDTH);
        m_gui->SetNextItemWidth(-1.f);
    }

    void HudEditor::DrawColorRow(const char* label, const char* id, UIColor& color)
    {
        DrawRow(label);
        float col[4] = { color.m_r, color.m_g, color.m_b, color.m_a };
        if (m_gui->ColorEdit4(id, col))
        {
            color.m_r = col[0];
            color.m_g = col[1];
            color.m_b = col[2];
            color.m_a = col[3];
            m_dirty = true;
        }
    }

    void HudEditor::DrawBaseProperties(UIElement* element)
    {
        DrawRow("Name:");
        char buf[128]; std::strncpy(buf, element->m_name.c_str(), 127); buf[127] = '\0';
        if (m_gui->InputText("##name", buf, 128)) 
        { 
            element->m_name = buf; 
            m_dirty = true; 
        }

        DrawRow("Visible:");
        if (m_gui->Checkbox("##vis", &element->m_visible)) m_dirty = true;

        m_gui->Separator();

        DrawRow("Anchor:");
        static const char* items =
            "TopLeft\0TopCenter\0TopRight\0"
            "MiddleLeft\0Center\0MiddleRight\0"
            "BottomLeft\0BottomCenter\0BottomRight\0";
        int cur = static_cast<int>(element->m_anchor);
        if (m_gui->Combo("##anch", &cur, items))
        {
            element->m_anchor = static_cast<UIAnchor>(cur);
            m_dirty = true;
        }

        DrawRow("Pos X:");
        if (m_gui->InputFloat("##px", &element->m_position[0])) m_dirty = true;
        DrawRow("Pos Y:");
        if (m_gui->InputFloat("##py", &element->m_position[1])) m_dirty = true;
        DrawRow("Width:");
        if (m_gui->InputFloat("##sw", &element->m_size[0])) m_dirty = true;
        DrawRow("Height:");
        if (m_gui->InputFloat("##sh", &element->m_size[1])) m_dirty = true;
    }

    void HudEditor::DrawTypeSpecificProperties(UIElement* element)
    {
        if (auto* text = dynamic_cast<UIText*>(element)) DrawTextProperties(text);
        else if (auto* image = dynamic_cast<UIImage*>(element)) DrawImageProperties(image);
        else if (auto* button = dynamic_cast<UIButton*>(element)) DrawButtonProperties(button);
        else if (auto* progressBar = dynamic_cast<UIProgressBar*>(element)) DrawProgressBarProperties(progressBar);
        else if (auto* panel = dynamic_cast<UIPanel*>(element)) DrawPanelProperties(panel);
    }

    void HudEditor::DrawTextProperties(UIText* text)
    {
        m_gui->Text("Text");
        m_gui->Separator();
        DrawRow("Content:");
        
        char buffer[256];
        std::strncpy(buffer, text->m_text.c_str(), 255);
        buffer[255] = '\0';
        if (m_gui->InputText("##txt", buffer, 256))
        {
            text->m_text = buffer;
            m_dirty = true;
        }

        DrawRow("FontSize:");
        if (m_gui->InputFloat("##fs", &text->m_fontSize)) m_dirty = true;
        DrawColorRow("Color:", "##tc", text->m_textColor);
        DrawRow("H-Align:");
        {
            static const char* hAlign = "Left\0Center\0Right\0";
            int current = static_cast<int>(text->m_hAlign);
            if (m_gui->Combo("##ha", &current, hAlign))
            {
                text->m_hAlign = static_cast<TextHAlign>(current);
                m_dirty = true;
            }
        }
        DrawRow("V-Align:");
        {
            static const char* vAlign = "Top\0Middle\0Bottom\0";
            int current = static_cast<int>(text->m_vAlign);
            if (m_gui->Combo("##va", &current, vAlign))
            {
                text->m_vAlign = static_cast<TextVAlign>(current);
                m_dirty = true;
            }
        }
    }

    void HudEditor::DrawImageProperties(UIImage* image)
    {
        m_gui->Text("Image");
        m_gui->Separator();
        DrawColorRow("Tint:", "##imgcol", image->m_color);
        m_gui->AlignTextToFramePadding();
        m_gui->Text("Texture:");
        m_gui->SameLine(LABEL_WIDTH);
        m_gui->SetNextItemWidth(-1.f);
        if (m_gui->Button("Browse...##texbrowse"))
        {
            m_showTexturePicker = true;
            m_texPickerOpened = false;
            ScanTextures();
        }

        if (!image->m_texturePath.empty())
        {
            DrawRow("File:");
            m_gui->TextDisabled(Fs::path(image->m_texturePath).filename().string());

            m_gui->AlignTextToFramePadding();
            m_gui->Text("Preview:");
            m_gui->SameLine(LABEL_WIDTH);

            if (image->m_glTexID == 0 && image->m_texture.IsReady())
            {
                auto handle = image->m_texture->GetId();
                if (handle != 0) image->m_glTexID = m_rhi->GetTexture(handle);
            }

            if (image->m_glTexID != 0)
                m_gui->DrawImageTinted(image->m_glTexID, { 80.f,80.f }, { 1,1,1,1 }, true);
            else if (image->m_texture.IsFailed())
                m_gui->TextDisabled("(load failed)");
            else
                m_gui->TextDisabled("(loading...)");
        }
    }

    void HudEditor::DrawPanelProperties(UIPanel* panel)
    {
        m_gui->Text("Panel");
        m_gui->Separator();
        DrawRow("Rounding:");
        if (m_gui->InputFloat("##pr", &panel->m_rounding)) m_dirty = true;
        DrawColorRow("Background:", "##pbc", panel->m_colorBg);
    }

    void HudEditor::DrawButtonProperties(UIButton* button)
    {
        m_gui->Text("Button");
        m_gui->Separator();
        DrawColorRow("Background:", "##bgc", button->m_color);
        m_gui->Separator();
        DrawRow("Label:");

        char buf[128]; std::strncpy(buf, button->m_labelText.m_text.c_str(), 127); buf[127] = '\0';
        if (m_gui->InputText("##bl", buf, 128)) 
        { 
            button->m_labelText.m_text = buf;
            m_dirty = true; 
        }

        DrawRow("FontSize:");
        if (m_gui->InputFloat("##bfs", &button->m_labelText.m_fontSize)) m_dirty = true;
        DrawColorRow("Text:", "##btc", button->m_labelText.m_textColor);
        DrawColorRow("Hover:", "##bhc", button->m_colorHover);
        DrawColorRow("Press:", "##bpc", button->m_colorPressed);
    }

    void HudEditor::DrawProgressBarProperties(UIProgressBar* progressBar)
    {
        m_gui->Text("ProgressBar");
        m_gui->Separator();
        DrawRow("Value:");
        if (m_gui->SliderFloat("##pbv", &progressBar->m_value, 0.f, 1.f)) m_dirty = true;
        DrawRow("Rounding:");
        if (m_gui->InputFloat("##pbr", &progressBar->m_rounding)) m_dirty = true;
        DrawColorRow("Fill:", "##pbf", progressBar->m_colorFill);
        DrawColorRow("Background:", "##pbb", progressBar->m_colorBg);
    }

    void HudEditor::AddWidget(UIWidgetType type)
    {
        std::unique_ptr<UIElement> element;
        switch (type)
        {
            case UIWidgetType::Text:
            {
                auto text = std::make_unique<UIText>();
                text->m_name = "Text";
                element = std::move(text);
                break;
            }
            case UIWidgetType::Image:
            {
                auto image = std::make_unique<UIImage>();
                image->m_name = "Image";
                image->m_size = { 0.15f, 0.15f };
                element = std::move(image);
                break;
            }
            case UIWidgetType::Button:
            {
                auto button = std::make_unique<UIButton>();
                button->m_name = "Button";
                button->m_size = { 0.15f, 0.05f };
                button->m_color = { 0.2f, 0.4f, 0.8f, 1.f };
                element = std::move(button);
                break;
            }
            case UIWidgetType::ProgressBar:
            {
                auto progressBar = std::make_unique<UIProgressBar>();
                progressBar->m_name = "ProgressBar";
                progressBar->m_size = { 0.3f, 0.03f };
                progressBar->m_value = 0.7f;
                element = std::move(progressBar);
                break;
            }
            case UIWidgetType::Panel:
            {
                auto panel = std::make_unique<UIPanel>();
                panel->m_name = "Panel";
                panel->m_size = { 0.25f, 0.2f };
                element = std::move(panel);
                break;
            }
        }

        HandleAddingWidget(element);
    }

    void HudEditor::HandleAddingWidget(std::unique_ptr<Apex::UserInterface::UIElement>& element)
    {
        if (element)
        {
            element->m_position = { 0.1f, 0.1f };
            UIPanel* selectedPanel = nullptr;
            if (m_selection.IsValid() && m_selection.IsTopLevel())
            {
                UIElement* selectedElement = m_canvas.GetElement(static_cast<size_t>(m_selection.m_index));
                selectedPanel = dynamic_cast<UIPanel*>(selectedElement);
            }

            if (selectedPanel)
            {
                int childIdx = static_cast<int>(selectedPanel->m_children.size());
                selectedPanel->AddChild(std::move(element));
                m_selection = { selectedPanel, childIdx };
            }
            else
            {
                int newIdx = static_cast<int>(m_canvas.GetElementCount());
                m_canvas.AddElement(std::move(element));
                m_selection = { nullptr, newIdx };
            }
            m_dirty = true;
        }
    }

    void HudEditor::DeleteSelected()
    {
        if (!m_selection.IsValid()) return;

        if (m_selection.IsTopLevel())
        {
            m_canvas.RemoveElement(static_cast<size_t>(m_selection.m_index));
        }
        else if (m_selection.m_panel)
        {
            m_selection.m_panel->ExtractChild(static_cast<size_t>(m_selection.m_index));
        }

        m_selection = {};
        m_dirty = true;
    }

    Apex::UserInterface::UIElement* HudEditor::ResolveSelected() const
    {
        return ResolveElement(m_selection);
    }

    Apex::UserInterface::UIElement* HudEditor::ResolveElement(const WidgetSelection& selection) const
    {
        if (!selection.IsValid()) return nullptr;
        if (selection.IsTopLevel())
            return m_canvas.GetElement(static_cast<size_t>(selection.m_index));

        UIElement* panelElement = nullptr;

        for (size_t i = 0; i < m_canvas.GetElementCount(); ++i)
        {
            if (m_canvas.GetElement(i) == selection.m_panel)
            {
                panelElement = selection.m_panel; 
                break;
            }
        }

        if (!panelElement) return nullptr;
        if (selection.m_index >= static_cast<int>(selection.m_panel->m_children.size()))
            return nullptr;
        return selection.m_panel->m_children[selection.m_index].get();
    }

    void HudEditor::ApplyPendingTreeOp()
    {
        if (m_pendingOp.kind == PendingTreeOp::Kind::None) return;

        auto& op = m_pendingOp;

        if (op.srcParent == op.dstParent && op.srcIndex == op.dstIndex)
        {
            op.kind = PendingTreeOp::Kind::None;
            return;
        }

        std::unique_ptr<UIElement> moved;
        if (op.srcParent)
            moved = op.srcParent->ExtractChild(static_cast<size_t>(op.srcIndex));
        else
            moved = m_canvas.ExtractElement(static_cast<size_t>(op.srcIndex));

        if (!moved) 
        { 
            op.kind = PendingTreeOp::Kind::None; 
            return; 
        }

        if (op.srcParent == op.dstParent &&
            op.kind != PendingTreeOp::Kind::Reparent &&
            op.srcIndex < op.dstIndex)
        {
            op.dstIndex--;
        }

        if (op.kind == PendingTreeOp::Kind::Reparent)
        {
            if (op.dstParent)
				op.dstParent->AddChild(std::move(moved));
            else
                m_canvas.AddElement(std::move(moved));
        }
        else 
        {
            int insertAt = op.dstIndex;
            if (op.kind == PendingTreeOp::Kind::ReorderAfter) insertAt++;

            if (op.dstParent)
            {
                auto& children = op.dstParent->m_children;
                insertAt = std::max(0, std::min(insertAt, static_cast<int>(children.size())));
                children.insert(children.begin() + insertAt, std::move(moved));
            }
            else
            {
                m_canvas.AddElement(std::move(moved));
                int from = static_cast<int>(m_canvas.GetElementCount()) - 1;
                insertAt = std::max(0, std::min(insertAt, from));
                while (from > insertAt)
                {
                    m_canvas.MoveElement(static_cast<size_t>(from), static_cast<size_t>(from - 1));
                    --from;
                }
            }
        }

        m_selection = {};
        m_dirty = true;
        op.kind = PendingTreeOp::Kind::None;
    }


    LibMath::Vector2 HudEditor::WidgetToCanvas(const Apex::UserInterface::UIElement& element) const
    {
        LibMath::Vector2 anchor = AnchorOrigin(element.m_anchor);
        return 
        {
            m_canvasMin[0] + (anchor[0] + element.m_position[0] * m_canvasSize[0]),
            m_canvasMin[1] + (anchor[1] + element.m_position[1] * m_canvasSize[1])
        };
    }

    LibMath::Vector2 HudEditor::CanvasToWidget(LibMath::Vector2 screenPos, Apex::UserInterface::UIAnchor anchor) const
    {
        LibMath::Vector2 local = 
        {
            (screenPos[0] - m_canvasMin[0]),
            (screenPos[1] - m_canvasMin[1])
        };
        LibMath::Vector2 anchorPx = AnchorOrigin(anchor);
        return 
        {
            (local[0] - anchorPx[0]) / m_canvasSize[0],
            (local[1] - anchorPx[1]) / m_canvasSize[1]
        };
    }

    LibMath::Vector2 HudEditor::AnchorOrigin(Apex::UserInterface::UIAnchor anchor) const
    {
        float ax = 0.f, ay = 0.f;
        switch (anchor)
        {
        case UIAnchor::TopCenter:    
            ax = 0.5f; 
            ay = 0.f; 
            break;
        case UIAnchor::TopRight:     
            ax = 1.f;  
            ay = 0.f;  
            break;
        case UIAnchor::MiddleLeft:   
            ax = 0.f;  
            ay = 0.5f; 
            break;
        case UIAnchor::Center:       
            ax = 0.5f; 
            ay = 0.5f; 
            break;
        case UIAnchor::MiddleRight:  
            ax = 1.f;  
            ay = 0.5f; 
            break;
        case UIAnchor::BottomLeft:   
            ax = 0.f;  
            ay = 1.f;  
            break;
        case UIAnchor::BottomCenter: 
            ax = 0.5f;
            ay = 1.f;  
            break;
        case UIAnchor::BottomRight:  
            ax = 1.f;  
            ay = 1.f;  
            break;
        default: break;
        }
        return { ax * m_canvasSize[0], ay * m_canvasSize[1] };
    }

    void HudEditor::ScanTextures()
    {
        m_textureFiles.clear();
        static const std::vector<std::string> exts = { ".png",".jpg",".jpeg",".tga",".hdr" };
        std::error_code error;
        for (const auto& e : Fs::recursive_directory_iterator("Assets/", error))
        {
            if (!e.is_regular_file()) continue;
            std::string ext = e.path().extension().string();
            for (auto& c : ext) c = static_cast<char>(::tolower(c));
            for (const auto& x : exts)
                if (ext == x) 
                { 
                    m_textureFiles.push_back(e.path()); 
                    break; 
                }
        }
    }

    std::string HudEditor::ShortenName(const std::string& name, size_t maxLen)
    {
        if (name.length() <= maxLen) return name;
        return name.substr(0, maxLen - 3) + "...";
    }
}