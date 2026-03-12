#include "ImguiManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

namespace Apex::UserInterface
{
    // Font Table
    const std::vector<FontInitializer> ImGUIManager::m_fontDefinitions = {
    { FontID::Small,   "Assets/Fonts/Roboto-Regular.ttf", FontSize::SMALL   },
    { FontID::Default, "Assets/Fonts/Roboto-Regular.ttf", FontSize::DEFAULT },
    { FontID::Large,   "Assets/Fonts/Roboto-Bold.ttf",    FontSize::LARGE   },
    { FontID::Title,   "Assets/Fonts/Roboto-Bold.ttf",    FontSize::TITLE   },
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
        case StyleColor::Text:    return ImGuiCol_Text;
        case StyleColor::Button:  return ImGuiCol_Button;
        case StyleColor::ChildBackground: return ImGuiCol_ChildBg;
        }
        return ImGuiCol_Text;
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
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
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

    bool ImGUIManager::WantCaptureMouse()    const { return ImGui::GetIO().WantCaptureMouse; }
    bool ImGUIManager::WantCaptureKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }

    void ImGUIManager::PushID(const std::string& id) { ImGui::PushID(id.c_str()); }
    void ImGUIManager::PushID(int id) { ImGui::PushID(id); }
    void ImGUIManager::PopID() { ImGui::PopID(); }

    void ImGUIManager::PushColor(StyleColor target, Color color)
    {
        ImGui::PushStyleColor(ToImGuiCol(target), ImVec4(color.m_r, color.m_g, color.m_b, color.m_a));
    }
    void ImGUIManager::PopColor() { ImGui::PopStyleColor(); }

    void ImGUIManager::Separator() { ImGui::Separator(); }
    void ImGUIManager::SameLine(float offsetFromLeft, float spacing) { ImGui::SameLine(offsetFromLeft, spacing); }
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

    void ImGUIManager::BeginPanel(const std::string& title, bool* open)
    {
        ImGui::Begin(title.c_str(), open, ImGuiWindowFlags_NoScrollbar);
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

    bool ImGUIManager::BeginContextMenu() { return ImGui::BeginPopupContextItem("##ctx"); }
    void ImGUIManager::EndContextMenu() { ImGui::EndPopup(); }

    bool ImGUIManager::MenuItem(const std::string& label, bool enabled)
    {
        return ImGui::MenuItem(label.c_str(), nullptr, false, enabled);
    }

    void ImGUIManager::OpenPopup(const std::string& id) { ImGui::OpenPopup(id.c_str()); }
    bool ImGUIManager::BeginPopup(const std::string& id) { return ImGui::BeginPopup(id.c_str()); }
    void ImGUIManager::EndPopup() { ImGui::EndPopup(); }

    void ImGUIManager::BeginTooltip() { ImGui::BeginTooltip(); }
    void ImGUIManager::EndTooltip() { ImGui::EndTooltip(); }

    void ImGUIManager::Text(const std::string& text) { ImGui::TextUnformatted(text.c_str()); }
    void ImGUIManager::TextDisabled(const std::string& text) { ImGui::TextDisabled("%s", text.c_str()); }
    void ImGUIManager::TextColored(Color color, const std::string& t)
    {
        ImGui::TextColored(ImVec4(color.m_r, color.m_g, color.m_b, color.m_a), "%s", t.c_str());
    }

    bool ImGUIManager::Button(const std::string& label) { return ImGui::Button(label.c_str()); }
    bool ImGUIManager::SmallButton(const std::string& label) { return ImGui::SmallButton(label.c_str()); }
    bool ImGUIManager::Checkbox(const std::string& label, bool* value) { return ImGui::Checkbox(label.c_str(), value); }

    bool ImGUIManager::SliderFloat(const std::string& label, float* value, float min, float max)
    {
        return ImGui::SliderFloat(label.c_str(), value, min, max);
    }
    bool ImGUIManager::SliderInt(const std::string& label, int* value, int min, int max)
    {
        return ImGui::SliderInt(label.c_str(), value, min, max);
    }
    bool ImGUIManager::ColorEdit3(const std::string& label, float colors[3])
    {
        return ImGui::ColorEdit3(label.c_str(), colors);
    }

    bool ImGUIManager::InputText(const std::string& label, char* buffer,
        size_t bufferSize, const std::string& hint)
    {
        return ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer, bufferSize);
    }

    bool ImGUIManager::InvisibleButton(const std::string& id, LibMath::Vector2 size)
    {
        return ImGui::InvisibleButton(id.c_str(), ImVec2(size[0], size[1]));
    }

    bool ImGUIManager::IsItemHovered() const { return ImGui::IsItemHovered(); }
    bool ImGUIManager::IsItemClicked(MouseButton button) const { return ImGui::IsItemClicked(ToImGuiMouseButton(button)); }
    bool ImGUIManager::IsItemDoubleClicked(MouseButton button) const 
    { 
        return ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ToImGuiMouseButton(button)); 
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
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        ImFont* font = m_fonts[static_cast<int>(id)];

        if (!font) return;

        drawList->AddText(font, size, ImVec2(x, y),
            IM_COL32(255, 255, 255, 255), text.c_str());
    }

    IDrawList* ImGUIManager::GetDrawList()
    {
        m_drawListWrapper.Reset(ImGui::GetWindowDrawList());
        return &m_drawListWrapper;
    }
}


