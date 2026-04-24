#include "ImguiManager.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

namespace Apex::UserInterface
{
    // Font Table
    const std::vector<FontInitializer> ImGUIManager::m_fontDefinitions = {
    { FontID::Small,   "ApexAssets/Fonts/Roboto-Regular.ttf", FontSize::SMALL   },
    { FontID::Default, "ApexAssets/Fonts/Roboto-Regular.ttf", FontSize::DEFAULT },
    { FontID::Large,   "ApexAssets/Fonts/Roboto-Bold.ttf",    FontSize::LARGE   },
    { FontID::Title,   "ApexAssets/Fonts/Roboto-Bold.ttf",    FontSize::TITLE   },
    };

    // Helpers: flag/enum translation 
    int ImGUIManager::ToImGuiMouseButton(MouseButton button)
    {
        switch (button) 
        {
        case MouseButton::Left:   return ImGuiMouseButton_Left;
        case MouseButton::Right:  return ImGuiMouseButton_Right;
        case MouseButton::Middle: return ImGuiMouseButton_Middle;
        }
        return ImGuiMouseButton_Left;
    }

    int ImGUIManager::ToImGuiTreeFlags(TreeNodeFlags flags)
    {
        int out = 0;
        if (flags & TreeNodeFlags::OpenOnArrow)    out |= ImGuiTreeNodeFlags_OpenOnArrow;
        if (flags & TreeNodeFlags::Selected)       out |= ImGuiTreeNodeFlags_Selected;
        if (flags & TreeNodeFlags::Leaf)           out |= ImGuiTreeNodeFlags_Leaf;
        if (flags & TreeNodeFlags::SpanAvailWidth) out |= ImGuiTreeNodeFlags_SpanAvailWidth;
        return out;
    }

    int ImGUIManager::ToImGuiCol(StyleColor target)
    {
        switch (target) 
        {
        case StyleColor::Text:              return ImGuiCol_Text;
        case StyleColor::Button:            return ImGuiCol_Button;
        case StyleColor::ChildBackground:   return ImGuiCol_ChildBg;
        }
        return ImGuiCol_Text;
    }

    static ImGuiKey ToImGuiKey(UIKey key)
    {
        switch (key)
        {
        case UIKey::Delete:    return ImGuiKey_Delete;
        case UIKey::Backspace: return ImGuiKey_Backspace;
        case UIKey::Enter:     return ImGuiKey_Enter;
        case UIKey::Escape:    return ImGuiKey_Escape;
        case UIKey::Tab:       return ImGuiKey_Tab;
        case UIKey::Space:     return ImGuiKey_Space;
        case UIKey::A:         return ImGuiKey_A;
        case UIKey::C:         return ImGuiKey_C;
        case UIKey::V:         return ImGuiKey_V;
        case UIKey::X:         return ImGuiKey_X;
        case UIKey::Z:         return ImGuiKey_Z;
        case UIKey::Y:         return ImGuiKey_Y;
        case UIKey::F2:        return ImGuiKey_F2;
        case UIKey::F5:        return ImGuiKey_F5;
        default:               return ImGuiKey_None;
        }
    }

    IGUI* CreateGUI(void* nativeWindow)
    {
        return new ImGUIManager(static_cast<GLFWwindow*>(nativeWindow));
    }

    FontInitializer::FontInitializer(FontID id, const std::string& file, float fontSize) :
        m_id(id), m_file(file), m_fontSize(fontSize) {}

    ImGUIManager::ImGUIManager(GLFWwindow* nativeWindow) :
        m_window(nativeWindow)
    {
        Initialize();
    }

    ImGUIManager::~ImGUIManager()
    {
        Shutdown();
    }

    void ImGUIManager::Initialize()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
        ImGui::GetIO().ConfigDockingAlwaysTabBar = true;
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.FrameRounding = 6.f;   // rounded buttons, inputs, sliders
        style.GrabRounding = 6.f;   // rounded slider grab
        style.PopupRounding = 6.f;   // rounded dropdowns
        style.WindowRounding = 6.f;   // rounded windows
        style.ScrollbarRounding = 6.f;   // rounded scrollbars
        style.TabRounding = 4.f;   // rounded tabs
        style.FramePadding = ImVec2(8.f, 4.f);   // comfortable button padding
        style.ItemSpacing = ImVec2(8.f, 6.f);   // breathing room between items
        style.WindowPadding = ImVec2(10.f, 10.f); // panel inner margin
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        LoadFonts();
    }

    void ImGUIManager::Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGUIManager::LoadFonts()
    {
        ImGuiIO& io = ImGui::GetIO();

        m_fonts.resize(static_cast<int>(FontID::Count));

        for (const auto& fontDef : m_fontDefinitions)
        {
            ImFont* font = io.Fonts->AddFontFromFileTTF(fontDef.m_file.c_str(), fontDef.m_fontSize);

            if (font)
            {
                m_fonts[static_cast<int>(fontDef.m_id)] = font;
            }
        }
    }

    void ImGUIManager::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGUIManager::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGUIManager::StartDockSpace()
    {
        // Configure window flags for main dockspace
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGui::Begin("MainDockSpace", nullptr, window_flags);
        ImGui::DockSpace(ImGui::GetID("MainDockSpace"));
        ImGui::End();
    }

    bool ImGUIManager::WantCaptureMouse()    const { return ImGui::GetIO().WantCaptureMouse; }
    bool ImGUIManager::WantCaptureKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }

    bool ImGUIManager::IsKeyPressed(UIKey key) const
    {
        return ImGui::IsKeyPressed(ToImGuiKey(key));
    }

    bool ImGUIManager::IsPanelFocused() const
    {
        return ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    }

    void ImGUIManager::PushID(const std::string& id) { ImGui::PushID(id.c_str()); }
    void ImGUIManager::PushID(int id) { ImGui::PushID(id); }
    void ImGUIManager::PopID() { ImGui::PopID(); }

    void ImGUIManager::PushColor(StyleColor target, Color color)
    {
        ImGui::PushStyleColor(ToImGuiCol(target), ImVec4(color.m_r, color.m_g, color.m_b, color.m_a));
    }
    void ImGUIManager::PopColor(int count) { ImGui::PopStyleColor(count); }

    void ImGUIManager::PushStyleVariable(StyleVariable var, float x, float y)
    {
        switch (var)
        {
        case StyleVariable::FramePadding:  
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(x, y)); 
            break;
        case StyleVariable::ItemSpacing:   
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(x, y)); 
            break;
        case StyleVariable::WindowPadding: 
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(x, y)); 
            break;
        case StyleVariable::IndentSpacing: 
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, x);            
            break;
        case StyleVariable::FrameRounding: 
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, x);            
            break;
        case StyleVariable::GrabRounding:  
            ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, x);            
            break;
        }
    }

    void ImGUIManager::PopStyleVariable(int count) { ImGui::PopStyleVar(count); }

    void ImGUIManager::PushFont(FontID id)
    {
        int idx = static_cast<int>(id);
        if (idx >= 0 && idx < static_cast<int>(m_fonts.size()) && m_fonts[idx])
            ImGui::PushFont(m_fonts[idx]);
        else
            ImGui::PushFont(nullptr); // falls back to default
    }

    void ImGUIManager::PopFont() { ImGui::PopFont(); }

    float ImGUIManager::GetTextLineHeight() const { return ImGui::GetTextLineHeight(); }

    void ImGUIManager::Separator() { ImGui::Separator(); }
    void ImGUIManager::VerticalSeparator() { ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical); }
    void ImGUIManager::SameLine(float offsetFromLeft, float spacing) { ImGui::SameLine(offsetFromLeft, spacing); }
    void ImGUIManager::NewLine() { ImGui::NewLine(); }
    void ImGUIManager::AlignTextToFramePadding() { ImGui::AlignTextToFramePadding(); }
    void ImGUIManager::SetNextItemWidth(float w) { ImGui::SetNextItemWidth(w); }

    LibMath::Vector2 ImGUIManager::GetAvailableSize() const
    {
        auto size = ImGui::GetContentRegionAvail();
        return LibMath::Vector2(size.x, size.y);
    }

    LibMath::Vector2 ImGUIManager::GetCursorPos() const
    {
        auto position = ImGui::GetCursorScreenPos();
        return LibMath::Vector2(position.x, position.y);
    }

    void ImGUIManager::SetCursorPos(LibMath::Vector2 pos)
    {
		ImGui::SetCursorScreenPos(ImVec2(pos[0], pos[1]));
    }

    LibMath::Vector2 ImGUIManager::GetPanelPos() const
    {
        auto pos = ImGui::GetWindowPos();
        return LibMath::Vector2(pos.x, pos.y);
    }

    LibMath::Vector2 ImGUIManager::GetPanelSize() const
    {
        auto size = ImGui::GetWindowSize();
        return LibMath::Vector2(size.x, size.y);
    }

    void ImGUIManager::BeginPanel(const std::string& title, bool* open, bool dock, bool titleBar, bool scrollable)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
        if (!dock)
            flags |= ImGuiWindowFlags_NoDocking;
        if (m_hasModal && title != m_modalName)
            flags |= ImGuiWindowFlags_NoInputs;
        if (!titleBar) 
            flags |= ImGuiWindowFlags_NoTitleBar;
        if (!scrollable)
            flags |= ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::Begin(title.c_str(), open, flags);
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!window->DockIsActive)
            ImGui::DockBuilderDockWindow(title.c_str(), ImGui::GetID("MainDockSpace"));
    }

    Vector2 ImGUIManager::GetScreenSize()
    {
        int width;
        int height;
        glfwGetWindowSize(m_window, &width, &height);

        return Vector2(width, height);
    }

    void ImGUIManager::EndPanel() { ImGui::End(); }

    void ImGUIManager::BeginChildPanel(const std::string& id,
        float width, float height, bool border)
    {
        ImGui::BeginChild(id.c_str(), ImVec2(width, height), border);
    }

    void ImGUIManager::EndChildPanel() { ImGui::EndChild(); }

    bool ImGUIManager::BeginTreeNode(const std::string& label, TreeNodeFlags flags)
    {
        return ImGui::TreeNodeEx(label.c_str(), ToImGuiTreeFlags(flags));
    }

    void ImGUIManager::EndTreeNode() { ImGui::TreePop(); }

    bool ImGUIManager::BeginContextMenu(const std::string& id) { return ImGui::BeginPopupContextItem(id.c_str()); }
    bool ImGUIManager::BeginGridContextMenu()
    {
        return ImGui::BeginPopupContextWindow("##grid_ctx", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems);
    }
    void ImGUIManager::EndContextMenu() { ImGui::EndPopup(); }

    bool ImGUIManager::MenuItem(const std::string& label, bool enabled)
    {
        return ImGui::MenuItem(label.c_str(), nullptr, false, enabled);
    }

    void ImGUIManager::SetKeyboardFocusHere() { ImGui::SetKeyboardFocusHere(); }
    void ImGUIManager::SetClipboardText(const std::string& text) { ImGui::SetClipboardText(text.c_str()); }
    void ImGUIManager::SetNextWindowFocus() { ImGui::SetNextWindowFocus(); }

    void ImGUIManager::OpenPopup(const std::string& id) { ImGui::OpenPopup(id.c_str()); }
    bool ImGUIManager::BeginPopup(const std::string& id) { return ImGui::BeginPopup(id.c_str()); }
    void ImGUIManager::EndPopup() { ImGui::EndPopup(); }

    bool ImGUIManager::BeginPopupModal(const std::string& title, LibMath::Vector2 size)
    {
        if (size[0] > 0.f || size[1] > 0.f)
            ImGui::SetNextWindowSize(ImVec2(size[0], size[1]), ImGuiCond_Appearing);
        return ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    }

    void ImGUIManager::CloseCurrentPopup() { ImGui::CloseCurrentPopup(); }

    void ImGUIManager::SetNextWindowSize(LibMath::Vector2 size)
    {
        ImGui::SetNextWindowSize(ImVec2(size[0], size[1]), ImGuiCond_Always);
    }

    void ImGUIManager::SetNextWindowPos(LibMath::Vector2 pos)
    {
        ImGui::SetNextWindowPos(ImVec2(pos[0], pos[1]), ImGuiCond_Always);
    }

    void ImGUIManager::BeginTooltip() { ImGui::BeginTooltip(); }
    void ImGUIManager::EndTooltip() { ImGui::EndTooltip(); }

    void ImGUIManager::Text(const std::string& text) { ImGui::TextUnformatted(text.c_str()); }
    void ImGUIManager::TextDisabled(const std::string& text) { ImGui::TextDisabled("%s", text.c_str()); }
    void ImGUIManager::TextColored(Color color, const std::string& t)
    {
        ImGui::TextColored(ImVec4(color.m_r, color.m_g, color.m_b, color.m_a), "%s", t.c_str());
    }

    bool ImGUIManager::Button(const std::string& label) { return ImGui::Button(label.c_str()); }

    bool ImGUIManager::ButtonSized(const std::string& label, LibMath::Vector2 size)
    {
        return ImGui::Button(label.c_str(), ImVec2(size[0], size[1]));
    }

    bool ImGUIManager::ImageButton(const std::string& id, uint32_t textureID, LibMath::Vector2 size, Color tint, float padding)
    {
        ImVec4 tintV(tint.m_r, tint.m_g, tint.m_b, tint.m_a);
        ImVec4 bgV(0.f, 0.f, 0.f, 0.f);  // transparent behind the image pixels
        bool result = ImGui::ImageButton(id.c_str(),
            (ImTextureID)(uintptr_t)textureID,
            ImVec2(size[0], size[1]),
            ImVec2(0, 1), ImVec2(1, 0),
            bgV, tintV);
        return result;
    }

    bool ImGUIManager::SmallButton(const std::string& label) { return ImGui::SmallButton(label.c_str()); }
    bool ImGUIManager::Checkbox(const std::string& label, bool* value) { return ImGui::Checkbox(label.c_str(), value); }

    bool ImGUIManager::CheckboxFormat(const std::string& label, bool* value)
    {
        ImGui::Text(label.c_str());
        ImGui::SameLine();

        std::string checkboxLabel = "##" + label;

        return ImGui::Checkbox(checkboxLabel.c_str(), value);
    }

    bool ImGUIManager::SliderFloat(const std::string& label, float* value, float min, float max)
    {
        return ImGui::SliderFloat(label.c_str(), value, min, max);
    }
    bool ImGUIManager::SliderInt(const std::string& label, int* value, int min, int max)
    {
        return ImGui::SliderInt(label.c_str(), value, min, max);
    }
    bool ImGUIManager::SliderFloatFormat(const std::string& label, float* value, float min, float max, float size, float sliderPos, float spacing)
    {
        ImGui::Text(label.c_str());

        if (sliderPos >= 0.f)
            ImGui::SameLine(sliderPos);
        else if (spacing >= 0.f)
            ImGui::SameLine(0.f, spacing);
        else
            ImGui::SameLine();

        ImGui::SetNextItemWidth(size);
        std::string sliderLabel = "##" + label;
        return ImGui::SliderFloat(sliderLabel.c_str(), value, min, max);
    }
    bool ImGUIManager::SliderIntFormat(const std::string& label, int* value, int min, int max, float size, float sliderPos, float spacing)
    {
        ImGui::Text(label.c_str());

        if (sliderPos >= 0.f)
            ImGui::SameLine(sliderPos);
        else if (spacing >= 0.f)
            ImGui::SameLine(0.f, spacing);
        else
            ImGui::SameLine();

        ImGui::SetNextItemWidth(size);
        std::string sliderLabel = "##" + label;
        return ImGui::SliderInt(sliderLabel.c_str(), value, min, max);
    }
    bool ImGUIManager::ColorEdit3(const std::string& label, float color[3])
    {
        return ImGui::ColorEdit3(label.c_str(), color);
    }

    bool ImGUIManager::ColorEdit4(const std::string& label, float color[4])
    {
		return ImGui::ColorEdit4(label.c_str(), color);
    }

    bool ImGUIManager::InputText(const std::string& label, char* buffer,
        size_t bufferSize, const std::string& hint)
    {
        return ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer, bufferSize);
    }

    bool ImGUIManager::InputFloat(const std::string& label, float* value)
    {
        return ImGui::InputFloat(label.c_str(), value);
    }

    bool ImGUIManager::InputInt(const std::string& label, int* value)
    {
		return ImGui::InputInt(label.c_str(), value);
    }

    bool ImGUIManager::Combo(const std::string& label, int* currentItem, const char* items)
    {
        return ImGui::Combo(label.c_str(), currentItem, items);
    }

    bool ImGUIManager::ComboFormat(const std::string& label, int* currentItem, const char* items)
    {
        ImGui::Text(label.c_str());
        ImGui::SameLine();

        std::string comboLabel = "##" + label;

        return ImGui::Combo(comboLabel.c_str(), currentItem, items);
    }

    bool ImGUIManager::CollapsingHeader(const std::string& label, bool defaultOpen)
    {
        ImGuiTreeNodeFlags flags = defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0;
        return ImGui::CollapsingHeader(label.c_str(), flags);
    }

    bool ImGUIManager::InvisibleButton(const std::string& id, LibMath::Vector2 size)
    {
        return ImGui::InvisibleButton(id.c_str(), ImVec2(size[0], size[1]));
    }

    bool ImGUIManager::IsItemHovered() const { return ImGui::IsItemHovered(); }
    bool ImGUIManager::IsItemActive()  const { return ImGui::IsItemActive(); }
    bool ImGUIManager::IsAnyItemActive() const { return ImGui::IsAnyItemHovered(); }

    LibMath::Vector2 ImGUIManager::GetItemRectSize() const
    {
        auto size = ImGui::GetItemRectSize();
        return LibMath::Vector2(size.x, size.y);
    }

    bool ImGUIManager::IsItemClicked(MouseButton button) const { return ImGui::IsItemClicked(ToImGuiMouseButton(button)); }
    bool ImGUIManager::IsItemDoubleClicked(MouseButton button) const 
    { 
        return ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ToImGuiMouseButton(button)); 
    }

    bool ImGUIManager::IsMouseReleased(MouseButton button) const
    {
        return ImGui::IsMouseReleased(ToImGuiMouseButton(button));
    }

    bool ImGUIManager::IsMouseClicked(MouseButton button) const
    {
        return ImGui::IsMouseClicked(ToImGuiMouseButton(button));
    }

    bool ImGUIManager::IsWindowHovered() const
    {
        return ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_NoPopupHierarchy);
    }

    bool ImGUIManager::IsWindowDocked() const
    {
        return ImGui::IsWindowDocked();
    }

    bool ImGUIManager::IsAnyItemHovered() const
    {
        return ImGui::IsAnyItemHovered();
    }

    bool ImGUIManager::IsDragDropActive() const
    {
        return ImGui::GetDragDropPayload() != nullptr;
    }

    const char* ImGUIManager::GetDragDropPayloadType() const
    {
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        return payload ? payload->DataType : nullptr;
    }

    bool ImGUIManager::IsMouseDown(MouseButton button) const
    {
        return ImGui::IsMouseDown(static_cast<int>(button));
    }

    LibMath::Vector2 ImGUIManager::GetMouseDelta() const
    {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        return LibMath::Vector2(delta.x, delta.y);
    }

    float ImGUIManager::GetMouseWheelDelta() const
    {
        return ImGui::GetIO().MouseWheel;
    }

    LibMath::Vector2 ImGUIManager::GetMousePos() const
    {
        ImVec2 pos = ImGui::GetMousePos();
        return LibMath::Vector2(pos.x, pos.y);
    }

    bool ImGUIManager::BeginDragSource()
    {
        return ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID);
    }

    void ImGUIManager::SetDragPayload(const char* type, const void* data, size_t size)
    {
        ImGui::SetDragDropPayload(type, data, size);
    }

    void ImGUIManager::EndDragSource() { ImGui::EndDragDropSource(); }

    bool ImGUIManager::BeginDropTarget() { return ImGui::BeginDragDropTarget(); }

    const void* ImGUIManager::AcceptDragPayload(const char* type)
    {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type);
        return payload ? payload->Data : nullptr;
    }

    void ImGUIManager::EndDropTarget() { ImGui::EndDragDropTarget(); }

    void ImGUIManager::DrawText2D(const std::string& text, float x, float y, float size, FontID id)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        ImFont* font = m_fonts[static_cast<int>(id)];

        if (!font) return;

        drawList->AddText(font, size, ImVec2(x, y),
            IM_COL32(255, 255, 255, 255), text.c_str());
    }

    void ImGUIManager::DrawImageBackground(unsigned int textureId, Vector2 topLeftCorner, Vector2 bottomRightCorner)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddImage((ImTextureID)(uintptr_t)(textureId),
            ImVec2(topLeftCorner[0], topLeftCorner[1]),
            ImVec2(bottomRightCorner[0], bottomRightCorner[1]), ImVec2(0, 1),
            ImVec2(1, 0));
    }

    void ImGUIManager::DrawImage(uint32_t textureID, LibMath::Vector2 size, bool flipY)
    {
        ImVec2 uv0 = flipY ? ImVec2(0, 1) : ImVec2(0, 0);
        ImVec2 uv1 = flipY ? ImVec2(1, 0) : ImVec2(1, 1);
        ImGui::Image((ImTextureID)(uintptr_t)textureID, ImVec2(size[0], size[1]), uv0, uv1);
    }

    void ImGUIManager::DrawImageTinted(uint32_t textureID, LibMath::Vector2 size, Color tint, bool flipY)
    {
        ImVec2 uv0 = flipY ? ImVec2(0, 1) : ImVec2(0, 0);
        ImVec2 uv1 = flipY ? ImVec2(1, 0) : ImVec2(1, 1);
        ImVec4 tintColor = ImVec4(tint.m_r, tint.m_g, tint.m_b, tint.m_a);
        ImVec4 border = ImVec4(0, 0, 0, 0);
        ImGui::Image((ImTextureID)(uintptr_t)textureID, ImVec2(size[0], size[1]), uv0, uv1, tintColor, border);
    }

    IDrawList* ImGUIManager::GetDrawList()
    {
        m_drawListWrapper.Reset(ImGui::GetWindowDrawList());
        return &m_drawListWrapper;
    }

    IDrawList* ImGUIManager::GetBackgroundDrawList()
    {
        m_drawListWrapper.Reset(ImGui::GetBackgroundDrawList());
        return &m_drawListWrapper;
    }

    Vector2 ImGUIManager::GetItemRectMin() const
    {
        ImVec2 v = ImGui::GetItemRectMin();
        return { v.x, v.y };
    }

    Vector2 ImGUIManager::GetItemRectMax() const
    {
        ImVec2 v = ImGui::GetItemRectMax();
        return { v.x, v.y };
    }

    void ImGUIManager::SetManagesCursor(bool manages)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (manages)
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
        else
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }

    void ImGUIManager::BeginModalBlock(const std::string& name)
    {
        m_hasModal = true;
        m_modalName = name;
    }

    void ImGUIManager::EndModalBlock()
    {
        m_hasModal = false;
        m_modalName.clear();
    }

    void ImGUIManager::BeginDisabled(bool disabled)
    {
        ImGui::BeginDisabled(disabled);
    }

    void ImGUIManager::EndDisabled()
    {
        ImGui::EndDisabled();
    }

    void ImGUIManager::Dummy(LibMath::Vector2 size)
    {
		ImGui::Dummy(ImVec2(size[0], size[1]));
    }
}


