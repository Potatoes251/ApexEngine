#include "GLFWWindow.h"

#include "InputSystem.h"
#include "Log.h"

#include "stb_image.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

// windows.h defines CreateWindow as a macro which conflicts with our IWindow* CreateWindow
#undef CreateWindow

#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace Apex::Input;

namespace Apex::Windowing 
{
	// ── Factory ─────────────────────────────────────────────────
	IWindow* CreateWindow(const std::string& title, int width, int height, IWindow* shareContext)
	{
        GLFWwindow* share = shareContext ?
             static_cast<GLFWwindow*>(shareContext->NativeHandle()) :
             nullptr;
        return new GLFWWindow(title, width, height, share);
	}

    void PollAllEvents()
    {
        glfwPollEvents();
    }

    void MakeContextCurrent(IWindow& window)
    {
        glfwMakeContextCurrent(static_cast<GLFWwindow*>(window.NativeHandle()));
    }

    // ── Constructor ──────────────────────────────────────────────
    GLFWWindow::GLFWWindow(const std::string& title, int width, int height, GLFWwindow* shareContext)
        : m_width(width), m_height(height), m_isPrimary(shareContext == nullptr)
    {
        if (m_isPrimary)
        {
            if (!glfwInit())
                LOG_ERROR("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, shareContext);
        if (!m_window) 
        {
            if (m_isPrimary) glfwTerminate();
            LOG_ERROR("Failed to create GLFW window");
            return;
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);

        if (m_isPrimary)
        {
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
                LOG_ERROR("Failed to initialize GLAD");
        }

        // Store pointer for static callbacks
        glfwSetWindowUserPointer(m_window, this);
        glfwSetKeyCallback(m_window, GLFWKeyCallback);
        glfwSetCursorPosCallback(m_window, GLFWCursorCallback);
        glfwSetMouseButtonCallback(m_window, GLFWMouseButtonCallback);
        glfwSetScrollCallback(m_window, GLFWScrollCallback);
        glfwSetFramebufferSizeCallback(m_window, GLFWResizeCallback);
        glfwSetDropCallback(m_window, GLFWDropCallback);

        m_lastFrameTime = (float)glfwGetTime();
        m_crosshairCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    }

    GLFWWindow::~GLFWWindow() 
    {
        if (m_crosshairCursor) glfwDestroyCursor(m_crosshairCursor);
        if (m_window)          glfwDestroyWindow(m_window);
        if (m_isPrimary)       glfwTerminate();
    }

    void GLFWWindow::SetWindowIcon(const std::string& path)
    {
        // Title bar (GLFW)
        int width, height, channels;
        unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
        if (pixels)
        {
            GLFWimage image{ width, height, pixels };
            glfwSetWindowIcon(m_window, 1, &image);
            stbi_image_free(pixels);
        }
    }

    // ── Lifecycle ────────────────────────────────────────────────
    void GLFWWindow::MakeContextCurrent() { glfwMakeContextCurrent(m_window); }
    bool GLFWWindow::ShouldClose()  const { return glfwWindowShouldClose(m_window); }
    void GLFWWindow::SwapBuffers() { glfwSwapBuffers(m_window); }
    void GLFWWindow::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void GLFWWindow::Close() { glfwSetWindowShouldClose(m_window, true); }
    void GLFWWindow::Focus() { glfwFocusWindow(m_window); }
    void GLFWWindow::Maximize() { glfwMaximizeWindow(m_window); }

    void GLFWWindow::UpdateDeltaTime()
    {
        float currentTime = (float)glfwGetTime();
        m_deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
    }

	// ── Properties ───────────────────────────────────────────────
	int GLFWWindow::GetWidth() const { return m_width; }
	int GLFWWindow::GetHeight() const { return m_height; }
    float GLFWWindow::GetAspectRatio() const { return m_height ? (float)m_width / m_height : 1.f; }
	float GLFWWindow::GetDeltaTime() const { return m_deltaTime; }

    void GLFWWindow::SetWindowTitle(const std::string& title)
    {
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void GLFWWindow::SetCursorCaptured(bool captured)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		m_firstMouseMove = true; // Reset mouse movement tracking when toggling capture
	}

    void GLFWWindow::SetCrosshairCursor()
    {
        if (m_crosshairCursor)
            glfwSetCursor(m_window, m_crosshairCursor);
    }

    void GLFWWindow::ResetCursorShape()
    {
        glfwSetCursor(m_window, nullptr);  // nullptr restores the default arrow
    }

    // ── Callback setters ─────────────────────────────────────────
	void GLFWWindow::SetKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
	void GLFWWindow::SetMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = callback; }
	void GLFWWindow::SetMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
	void GLFWWindow::SetScrollCallback(ScrollCallback callback) { m_scrollCallback = callback; }
	void GLFWWindow::SetResizeCallback(ResizeCallback callback) { m_resizeCallback = callback; }
	void GLFWWindow::SetExternalDropCallback(ExternalDropCallback callback) { m_dropCallback = callback; }

    std::vector<std::filesystem::path> GLFWWindow::OpenFileDialog(bool allowMultiple)
    {
        std::vector<std::filesystem::path> result;

        constexpr int BUFFER = 32768;
        std::wstring buffer(BUFFER, L'\0');

        OPENFILENAMEW openFileName = {};
        openFileName.lStructSize = sizeof(openFileName);
        openFileName.hwndOwner = GetActiveWindow();
        openFileName.lpstrFile = buffer.data();
        openFileName.nMaxFile = BUFFER;
        openFileName.lpstrTitle = L"Import Assets";
        openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
        if (allowMultiple)
            openFileName.Flags |= OFN_ALLOWMULTISELECT;

        if (!GetOpenFileNameW(&openFileName))
            return result; // cancelled

        // Multi-select: buffer is "dir\0file1\0file2\0\0"
        // Single file:  buffer is just the full path
        const wchar_t* path = buffer.data();
        std::filesystem::path dir(path);
        path += dir.native().size() + 1;

        if (*path == L'\0')
        {
            // Single selection — dir is actually the full path
            result.push_back(dir);
        }
        else
        {
            while (*path != L'\0')
            {
                result.push_back(dir / path);
                path += std::wcslen(path) + 1;
            }
        }
        return result;
    }

    std::filesystem::path GLFWWindow::OpenFolderDialog()
    {
        BROWSEINFOW browseInfo = {};
        browseInfo.hwndOwner = GetActiveWindow();
        browseInfo.lpszTitle = L"Select Folder to Import";
        browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

        PIDLIST_ABSOLUTE pIDList = SHBrowseForFolderW(&browseInfo);
        if (!pIDList) return {};

        wchar_t buffer[MAX_PATH];
        SHGetPathFromIDListW(pIDList, buffer);
        CoTaskMemFree(pIDList);
        return std::filesystem::path(buffer);
    }

    std::filesystem::path GLFWWindow::SaveFileDialog(const std::string& defaultName)
    {
        constexpr int BUFFER = 32768;
        std::wstring buffer(BUFFER, L'\0');

        // Pre-fill the filename field with the suggested name
        if (!defaultName.empty())
        {
            std::wstring wide(defaultName.begin(), defaultName.end());
            wide.copy(buffer.data(), std::min(wide.size(), (size_t)(BUFFER - 1)));
        }

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = buffer.data();
        ofn.nMaxFile = BUFFER;
        ofn.lpstrTitle = L"Export Texture";
        ofn.lpstrFilter = L"PNG Image\0*.png\0JPEG Image\0*.jpg\0HDR Image\0*.hdr\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;

        if (!GetSaveFileNameW(&ofn))
            return {};

        return std::filesystem::path(buffer.data());
    }

	// ── GLFW static callback wrappers ─────────────────────────────
    void GLFWWindow::GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (!self || !self->m_keyCallback)
            return;

        Input::InputSystem::Get().SetKeyDown(TranslateKey(key), action != GLFW_RELEASE);
        self->m_keyCallback(TranslateKey(key), action != GLFW_RELEASE);
	}

    void GLFWWindow::GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self && self->m_mouseMoveCallback)
        {
            if (self->m_firstMouseMove) 
            {
                self->m_lastMouseX = xpos;
                self->m_lastMouseY = ypos;
                self->m_firstMouseMove = false;
            }
            double dx = xpos - self->m_lastMouseX;
            double dy = self->m_lastMouseY - ypos; // y inverted
            self->m_lastMouseX = xpos;
            self->m_lastMouseY = ypos;
            self->m_mouseMoveCallback(dx, dy);
        }
        Input::InputSystem::Get().SetMousePos({ (float)xpos, (float)ypos });
	}

    void GLFWWindow::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self && self->m_mouseButtonCallback)
            self->m_mouseButtonCallback(TranslateMouseButton(button), action == GLFW_PRESS);

        Input::InputSystem::Get().SetMouseButtonDown(TranslateMouseButton(button), action != GLFW_RELEASE);
    }

    void GLFWWindow::GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self && self->m_scrollCallback)
            self->m_scrollCallback(xoffset, yoffset);

        Input::InputSystem::Get().SetMouseScroll(yoffset);
    }

    void GLFWWindow::GLFWResizeCallback(GLFWwindow* window, int width, int height)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self) 
        {
            self->m_width = width;
            self->m_height = height;
            glViewport(0, 0, width, height);
            if (self->m_resizeCallback)
                self->m_resizeCallback(width, height);
        }
	}

    void GLFWWindow::GLFWDropCallback(GLFWwindow* window, int count, const char** paths)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (!self || !self->m_dropCallback) return;

        std::vector<std::filesystem::path> result;
        result.reserve(count);
        for (int i = 0; i < count; ++i)
            result.emplace_back(paths[i]);

        self->m_dropCallback(std::move(result));
    }

    void GLFWWindow::SetCloseCallback(CloseCallback callback)
    {
        m_closeCallback = std::move(callback);

        glfwSetWindowUserPointer(m_window, this);

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* win)
            {
                auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(win));

                if (self->m_closeCallback)
                    self->m_closeCallback();
            });
    }

    void GLFWWindow::SetShouldClose(bool close)
    {
        glfwSetWindowShouldClose(m_window, close);
    }

    // ── Translation helpers ───────────────────────────────────────
    Key GLFWWindow::TranslateKey(int glfwKey)
    {
        switch (glfwKey) 
        {
        case GLFW_KEY_A:      return Key::A;
        case GLFW_KEY_B:      return Key::B;
        case GLFW_KEY_C:      return Key::C;
        case GLFW_KEY_D:      return Key::D;
        case GLFW_KEY_E:      return Key::E;
        case GLFW_KEY_F:      return Key::F;
        case GLFW_KEY_G:      return Key::G;
        case GLFW_KEY_H:      return Key::H;
        case GLFW_KEY_I:      return Key::I;
        case GLFW_KEY_J:      return Key::J;
        case GLFW_KEY_K:      return Key::K;
        case GLFW_KEY_L:      return Key::L;
        case GLFW_KEY_M:      return Key::M;
        case GLFW_KEY_N:      return Key::N;
        case GLFW_KEY_O:      return Key::O;
        case GLFW_KEY_P:      return Key::P;
        case GLFW_KEY_Q:      return Key::Q;
        case GLFW_KEY_R:      return Key::R;
        case GLFW_KEY_S:      return Key::S;
        case GLFW_KEY_T:      return Key::T;
        case GLFW_KEY_U:      return Key::U;
        case GLFW_KEY_V:      return Key::V;
        case GLFW_KEY_W:      return Key::W;
        case GLFW_KEY_X:      return Key::X;
        case GLFW_KEY_Y:      return Key::Y;
        case GLFW_KEY_Z:      return Key::Z;

        case GLFW_KEY_0:      return Key::N0;
        case GLFW_KEY_1:      return Key::N1;
        case GLFW_KEY_2:      return Key::N2;
        case GLFW_KEY_3:      return Key::N3;
        case GLFW_KEY_4:      return Key::N4;
        case GLFW_KEY_5:      return Key::N5;
        case GLFW_KEY_6:      return Key::N6;
        case GLFW_KEY_7:      return Key::N7;
        case GLFW_KEY_8:      return Key::N8;
        case GLFW_KEY_9:      return Key::N9;

        case GLFW_KEY_UP:     return Key::Up;
        case GLFW_KEY_DOWN:   return Key::Down;
        case GLFW_KEY_LEFT:   return Key::Left;
        case GLFW_KEY_RIGHT:  return Key::Right;

        case GLFW_KEY_TAB:              return Key::Tab;
        case GLFW_KEY_LEFT_CONTROL:     return Key::Ctrl;
        case GLFW_KEY_RIGHT_CONTROL:    return Key::Ctrl;
        case GLFW_KEY_LEFT_SHIFT:       return Key::Shift;
        case GLFW_KEY_RIGHT_SHIFT:      return Key::Shift;
        case GLFW_KEY_SPACE:            return Key::Space;
        case GLFW_KEY_ENTER:            return Key::Enter;
        case GLFW_KEY_ESCAPE:           return Key::Escape;
        case GLFW_KEY_BACKSPACE:        return Key::BackSpace;


        default:              return Key::Unknown;
        }
	}

    MouseButton GLFWWindow::TranslateMouseButton(int glfwButton)
    {
        switch (glfwButton) 
        {
        case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
        default:                       return MouseButton::Left; // Default to left for unknown buttons
        }
	}
} // namespace Apex::Windowing