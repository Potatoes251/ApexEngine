#include "ImguiManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

namespace Apex::UserInterface   //to prevent a bug
{
    IGUI* CreateGUI(void* nativeWindow)
    {
        return new ImGUIManager(static_cast<GLFWwindow*>(nativeWindow));
    }
}

using namespace Apex::UserInterface;

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

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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

        m_fonts[static_cast<int>(fontDef.m_id)] = font;
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

bool ImGUIManager::WantCaptureMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGUIManager::WantCaptureKeyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

void ImGUIManager::BeginPanel(const std::string& title)
{
    ImGui::Begin(title.c_str());
}

void ImGUIManager::EndPanel()
{
    ImGui::End();
}

void ImGUIManager::Text(const std::string& text)
{
    ImGui::TextUnformatted(text.c_str());
}

bool ImGUIManager::Button(const std::string& label)
{
    return ImGui::Button(label.c_str());
}

bool ImGUIManager::Checkbox(const std::string& label, bool* v)
{
    return ImGui::Checkbox(label.c_str(), v);
}

bool ImGUIManager::SliderFloat(const char* label, float* v, float min, float max)
{
    return ImGui::SliderFloat(label, v, min, max);
}

bool ImGUIManager::SliderInt(const char* label, int* v, int min, int max)
{
    return ImGui::SliderInt(label, v, min, max);
}

bool ImGUIManager::ColorEdit3(const char* label, float color[3])
{
    return ImGui::ColorEdit3(label, color);
}

void ImGUIManager::Separator()
{
    ImGui::Separator();
}

void ImGUIManager::SameLine()
{
    ImGui::SameLine();
}

void ImGUIManager::DrawText2D(const std::string& text, float x, float y, float size, FontID id)
{
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    ImFont* font = m_fonts[static_cast<int>(id)];

    if (!font) return;

    drawList->AddText(font, size, ImVec2(x, y),
        IM_COL32(255, 255, 255, 255), text.c_str());
}
