#include "HudEditor.h"
#include "EditorIcon.h"

using namespace Apex::UserInterface;

namespace Apex::Editor
{

    HudEditor::HudEditor(IGUI& gui,
        Apex::Windowing::IWindow& window,
        Apex::Resources::ResourceManager& resourceManager)
        : m_gui(gui), m_window(window), m_resourceManager(resourceManager) 
    {
    }

    void HudEditor::Open(const fs::path& path, int instanceIndex)
    {
        m_path = path;
        m_instanceIndex = instanceIndex;
        m_open = true;
        m_dirty = false;
        m_selectedIndex = -1;
        m_canvas.Clear();

        if (fs::exists(path))
            m_canvas.Load(path, m_resourceManager);
    }

    void HudEditor::Close()
    {
        m_open = false;
    }

    void HudEditor::Focus()
    {
        m_pendingFocus = true;
    }

    void HudEditor::Draw()
    {
        if (!m_open) return;

        m_gui.PushFont(FontID::Default);

        constexpr float CASCADE = 30.f;
        float offset = static_cast<float>(m_instanceIndex) * CASCADE;
        m_gui.SetNextWindowPos(LibMath::Vector2(offset + 100.f, offset + 80.f));
        m_gui.SetNextWindowSize(LibMath::Vector2(1100.f, 700.f));

        if (m_pendingFocus) 
        { 
            m_gui.SetNextWindowFocus(); 
            m_pendingFocus = false; 
        }

        std::string title = "HUD Editor - " + m_path.stem().string();
        if (m_dirty) title += " *";

        m_gui.BeginPanel(title, &m_open);

        if (!m_open)
        {
            Close();
            m_gui.EndPanel();
            m_gui.PopFont();
            return;
        }

        DrawToolbar();
        m_gui.Separator();

        float availableWidth = m_gui.GetAvailableSize()[0];
        float availableHeight = m_gui.GetAvailableSize()[1];

        float canvasWidth = availableWidth - PROPS_WIDTH - 8.f;
        float canvasHeight = availableHeight - TREE_HEIGHT - 8.f;
        float canvasAspectHeight = canvasWidth / CANVAS_ASPECT;
        if (canvasAspectHeight < canvasHeight) canvasHeight = canvasAspectHeight;

        m_gui.BeginChildPanel("##he_left", canvasWidth, availableHeight, false);
        {
            m_gui.BeginChildPanel("##he_canvas_wrap", canvasWidth, canvasHeight, true);
            {
                LibMath::Vector2 panelPos = m_gui.GetPanelPos();
                LibMath::Vector2 panelSize = m_gui.GetAvailableSize();

                float cw = panelSize[0];
                float ch = cw / CANVAS_ASPECT;
                if (ch > panelSize[1]) { ch = panelSize[1]; cw = ch * CANVAS_ASPECT; }

                LibMath::Vector2 canvasMin = {
                    panelPos[0] + (panelSize[0] - cw) * 0.5f,
                    panelPos[1] + (panelSize[1] - ch) * 0.5f };
                LibMath::Vector2 canvasSize = { cw, ch };

                IDrawList* drawList = m_gui.GetDrawList();

                drawList->DrawRectFilled(canvasMin, { canvasMin[0] + canvasSize[0], canvasMin[1] + canvasSize[1] }, 0xFF1A1A1A, 4.f);

                DrawCanvas(drawList, canvasMin, canvasSize);
                HandleCanvasInput(canvasMin, canvasSize);

                drawList->DrawRect(canvasMin, { canvasMin[0] + canvasSize[0], canvasMin[1] + canvasSize[1] }, 0xFF555555, 4.f, 1.f);
            }
            m_gui.EndChildPanel();

            m_gui.BeginChildPanel("##he_tree", canvasWidth, TREE_HEIGHT, true);
            DrawWidgetTree();
            m_gui.EndChildPanel();
        }
        m_gui.EndChildPanel();

        m_gui.SameLine();

        m_gui.BeginChildPanel("##he_props", PROPS_WIDTH, availableHeight, true);
        DrawProperties();
        m_gui.EndChildPanel();

        m_gui.EndPanel();
        m_gui.PopFont();
    }

    void HudEditor::DrawToolbar()
    {
        m_gui.PushStyleVariable(StyleVariable::FramePadding, 6.f, 4.f);

        DrawToolbarSave();
        m_gui.SameLine(0.f, 4.f);
        DrawToolbarLoad();

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        m_gui.Text("Add:");
        m_gui.SameLine(0.f, 6.f);

        auto addButton = [&](const char* label, UIWidgetType type)
            {
                if (m_gui.Button(label))
                    AddWidget(type);
                m_gui.SameLine(0.f, 4.f);
            };

        addButton("Text", UIWidgetType::Text);
        addButton("Image", UIWidgetType::Image);
        addButton("Button", UIWidgetType::Button);
        addButton("ProgressBar", UIWidgetType::ProgressBar);
        addButton("Panel", UIWidgetType::Panel);

        if (m_selectedIndex >= 0 && m_gui.IsKeyPressed(UIKey::F2))
            DeleteSelected();

        m_gui.PopStyleVariable();
    }

    void HudEditor::DrawToolbarSave()
    {
        uint32_t saveIcon = EditorIcon::Get("ApexAssets/Icons/save.png");
        if (saveIcon != 0)
        {
            if (m_gui.ImageButton("##save", saveIcon, LibMath::Vector2(22.f, 22.f)))
            {
                if (m_canvas.Save(m_path))
                    m_dirty = false;
                if (m_gui.IsItemHovered())
                {
                    m_gui.BeginTooltip();
                    m_gui.Text("Save Canvas...");
                    m_gui.EndTooltip();
                }
            }
        }
        else
        {
            if (m_gui.Button("Save"))
            {
                if (m_canvas.Save(m_path))
                    m_dirty = false;
            }
        }
    }

    void HudEditor::DrawToolbarLoad()
    {
        uint32_t loadIcon = EditorIcon::Get("ApexAssets/Icons/load.png");
        if (loadIcon != 0)
        {
            if (m_gui.ImageButton("##load", loadIcon, LibMath::Vector2(22.f, 22.f)))
            {
                m_canvas.Load(m_path, m_resourceManager);
                m_selectedIndex = -1;
                m_dirty = false;
                if (m_gui.IsItemHovered())
                {
                    m_gui.BeginTooltip();
                    m_gui.Text("Load Canvas...");
                    m_gui.EndTooltip();
                }
            }
        }
        else
        {
            if (m_gui.Button("Load"))
            {
                m_canvas.Load(m_path, m_resourceManager);
                m_selectedIndex = -1;
                m_dirty = false;
            }
        }
    }

    void HudEditor::DrawCanvas(IDrawList* drawList,
        LibMath::Vector2 canvasMin,
        LibMath::Vector2 canvasSize)
    {
        size_t count = m_canvas.GetElementCount();
        for (size_t i = 0; i < count; ++i)
        {
            UIElement* element = m_canvas.GetElement(i);
            if (!element || !element->m_visible) continue;

            UIDrawContext context;
            context.m_drawList = drawList;
            context.m_screenSize = canvasSize;
            context.m_isEditor = true;
            context.m_selected = (static_cast<int>(i) == m_selectedIndex);

            LibMath::Vector2 savedPos = element->m_position;
            element->m_position = 
            {
                canvasMin[0] / canvasSize[0] + savedPos[0],
                canvasMin[1] / canvasSize[1] + savedPos[1] 
            };

            element->Draw(context);

            element->m_position = savedPos;
        }
    }

    void HudEditor::HandleCanvasInput(LibMath::Vector2 canvasMin, LibMath::Vector2 canvasSize)
    {
        m_gui.SetCursorPos(canvasMin);
        m_gui.InvisibleButton("##canvas_input", canvasSize);

        bool hovered = m_gui.IsItemHovered();
        bool clicked = m_gui.IsItemClicked();
        bool held = m_gui.IsItemActive();

        LibMath::Vector2 mouse = m_gui.GetMousePos();

        if (clicked)
        {
            m_selectedIndex = -1;
            m_dragging = false;

            for (int i = static_cast<int>(m_canvas.GetElementCount()) - 1; i >= 0; --i)
            {
                UIElement* element = m_canvas.GetElement(i);
                if (!element) continue;

                LibMath::Vector2 screenPos = CanvasToScreen(element->m_position, canvasMin, canvasSize);
                LibMath::Vector2 screenSize = 
                {
                    element->m_size[0] * canvasSize[0],
                    element->m_size[1] * canvasSize[1] 
                };

                if (mouse[0] >= screenPos[0] && mouse[0] <= screenPos[0] + screenSize[0] &&
                    mouse[1] >= screenPos[1] && mouse[1] <= screenPos[1] + screenSize[1])
                {
                    m_selectedIndex = i;
                    m_dragOffset = { mouse[0] - screenPos[0], mouse[1] - screenPos[1] };
                    m_dragging = true;
                    break;
                }
            }
        }

        if (held && m_dragging && m_selectedIndex >= 0)
        {
            UIElement* element = m_canvas.GetElement(m_selectedIndex);
            if (element)
            {
                LibMath::Vector2 newScreen = 
                {
                    mouse[0] - m_dragOffset[0],
                    mouse[1] - m_dragOffset[1] 
                };
                element->m_position = ScreenToCanvas(newScreen, canvasMin, canvasSize);
                m_dirty = true;
            }
        }

        if (!held) m_dragging = false;
    }

    void HudEditor::DrawWidgetTree()
    {
        m_gui.PushFont(FontID::Large);
        m_gui.Text("Widgets");
        m_gui.PopFont();
        m_gui.Separator();

        size_t count = m_canvas.GetElementCount();
        for (size_t i = 0; i < count; ++i)
        {
            UIElement* element = m_canvas.GetElement(i);
            if (!element) continue;

            const char* typeLabel = "?";
            switch (element->GetType())
            {
            case UIWidgetType::Text:        typeLabel = "[T]"; break;
            case UIWidgetType::Image:       typeLabel = "[I]"; break;
            case UIWidgetType::Button:      typeLabel = "[B]"; break;
            case UIWidgetType::ProgressBar: typeLabel = "[P]"; break;
            case UIWidgetType::Panel:       typeLabel = "[C]"; break;
            }

            bool isSelected = (static_cast<int>(i) == m_selectedIndex);
            std::string label = std::string(typeLabel) + " " + element->m_name + "##tree" + std::to_string(i);

            if (isSelected)
                m_gui.PushColor(StyleColor::Button, { 0.26f, 0.42f, 0.60f, 0.8f });

            if (m_gui.SmallButton(label))
                m_selectedIndex = static_cast<int>(i);

            if (isSelected)
                m_gui.PopColor();
        }
    }

    void HudEditor::DrawProperties()
    {
        m_gui.PushFont(FontID::Large);
        m_gui.Text("Properties");
        m_gui.PopFont();
        m_gui.Separator();

        if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_canvas.GetElementCount()))
        {
            m_gui.TextDisabled("No widget selected");
            return;
        }

        UIElement* element = m_canvas.GetElement(m_selectedIndex);
        if (!element) return;

        m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 4.f, 6.f);

        constexpr float LABEL_WIDTH = 90.f;

        auto label = [&](const char* txt)
            {
                m_gui.AlignTextToFramePadding();
                m_gui.Text(txt);
                m_gui.SameLine(LABEL_WIDTH);
                m_gui.SetNextItemWidth(-1.f);
            };

        label("Name:");
        {
            char buffer[128];
            std::strncpy(buffer, element->m_name.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            if (m_gui.InputText("##name", buffer, sizeof(buffer)))
            {
                element->m_name = buffer; m_dirty = true;
            }
        }

        label("Visible:");
        if (m_gui.Checkbox("##vis", &element->m_visible)) m_dirty = true;

        m_gui.Separator();

        label("Anchor:");
        {
            static const char* anchorItems =
                "TopLeft\0TopCenter\0TopRight\0"
                "MiddleLeft\0Center\0MiddleRight\0"
                "BottomLeft\0BottomCenter\0BottomRight\0";
            int cur = static_cast<int>(element->m_anchor);
            if (m_gui.Combo("##anch", &cur, anchorItems))
            {
                element->m_anchor = static_cast<UIAnchor>(cur); m_dirty = true;
            }
        }

        label("Pos X:");
        if (m_gui.InputFloat("##px", &element->m_position[0])) m_dirty = true;
        label("Pos Y:");
        if (m_gui.InputFloat("##py", &element->m_position[1])) m_dirty = true;
        label("Size W:");
        if (m_gui.InputFloat("##sw", &element->m_size[0])) m_dirty = true;
        label("Size H:");
        if (m_gui.InputFloat("##sh", &element->m_size[1])) m_dirty = true;

        m_gui.Separator();

        auto colorEdit = [&](const char* lbl, const char* id, UIColor& c)
            {
                label(lbl);
                float col[3] = { c.m_r, c.m_g, c.m_b };
                if (m_gui.ColorEdit3(id, col))
                {
                    c.m_r = col[0]; c.m_g = col[1]; c.m_b = col[2]; m_dirty = true;
                }
                std::string alphaId = std::string(id) + "_a";
                m_gui.SameLine(0.f, 4.f);
                m_gui.SetNextItemWidth(60.f);
                if (m_gui.SliderFloat(alphaId, &c.m_a, 0.f, 1.f)) m_dirty = true;
            };

        colorEdit("Color:", "##col", element->m_color);

        m_gui.Separator();

        if (auto* t = dynamic_cast<UIText*>(element))
        {
            m_gui.Text("-- Text --");
            label("Content:");
            {
                char buf[256];
                std::strncpy(buf, t->m_text.c_str(), sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                if (m_gui.InputText("##txt", buf, sizeof(buf)))
                {
                    t->m_text = buf; m_dirty = true;
                }
            }
            label("Font size:");
            if (m_gui.InputFloat("##fs", &t->m_fontSize)) m_dirty = true;
        }
        else if (auto* img = dynamic_cast<UIImage*>(element))
        {
            m_gui.Text("-- Image --");
            label("Texture:");
            {
                char buf[512];
                std::strncpy(buf, img->m_texturePath.c_str(), sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                if (m_gui.InputText("##tex", buf, sizeof(buf)))
                {
                    img->m_texturePath = buf;
                    img->LoadTexture(m_rm);
                    m_dirty = true;
                }
            }
        }
        else if (auto* btn = dynamic_cast<UIButton*>(element))
        {
            m_gui.Text("-- Button --");
            label("Label:");
            {
                char buf[128];
                std::strncpy(buf, btn->m_label.c_str(), sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                if (m_gui.InputText("##blbl", buf, sizeof(buf)))
                {
                    btn->m_label = buf; m_dirty = true;
                }
            }
            label("Font size:");
            if (m_gui.InputFloat("##bfs", &btn->m_fontSize)) m_dirty = true;
            colorEdit("Text col:", "##btc", btn->m_colorText);
            colorEdit("Hover col:", "##bhc", btn->m_colorHover);
            colorEdit("Press col:", "##bpc", btn->m_colorPressed);
        }
        else if (auto* pb = dynamic_cast<UIProgressBar*>(element))
        {
            m_gui.Text("-- Progress Bar --");
            label("Value:");
            if (m_gui.SliderFloat("##pbv", &pb->m_value, 0.f, 1.f)) m_dirty = true;
            label("Rounding:");
            if (m_gui.InputFloat("##pbr", &pb->m_rounding)) m_dirty = true;
            colorEdit("Fill col:", "##pbfc", pb->m_colorFill);
            colorEdit("BG col:", "##pbbc", pb->m_colorBg);
        }
        else if (auto* panel = dynamic_cast<UIPanel*>(element))
        {
            m_gui.Text("-- Panel --");
            label("Rounding:");
            if (m_gui.InputFloat("##pr", &panel->m_rounding)) m_dirty = true;
            colorEdit("BG col:", "##pbc", panel->m_colorBg);
        }

        m_gui.PopStyleVariable();
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

        if (element)
        {
            element->m_position = { 0.1f, 0.1f };
            m_selectedIndex = static_cast<int>(m_canvas.GetElementCount());
            m_canvas.AddElement(std::move(element));
            m_dirty = true;
        }
    }

    void HudEditor::DeleteSelected()
    {
        if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_canvas.GetElementCount()))
            return;

        m_canvas.RemoveElement(static_cast<size_t>(m_selectedIndex));
        m_selectedIndex = -1;
        m_dirty = true;
    }

    LibMath::Vector2 HudEditor::CanvasToScreen(LibMath::Vector2 normalized, LibMath::Vector2 canvasMin, LibMath::Vector2 canvasSize) const
    {
        return 
        {
            canvasMin[0] + normalized[0] * canvasSize[0],
            canvasMin[1] + normalized[1] * canvasSize[1] 
        };
    }

    LibMath::Vector2 HudEditor::ScreenToCanvas(LibMath::Vector2 screen, LibMath::Vector2 canvasMin, LibMath::Vector2 canvasSize) const
    {
        return 
        {
            std::max(0.f, std::min(1.f, (screen[0] - canvasMin[0]) / canvasSize[0])),
            std::max(0.f, std::min(1.f, (screen[1] - canvasMin[1]) / canvasSize[1])) 
        };
    }

}