#include "GLFWWindow.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

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
                std::cout << "Failed to initialize GLFW" << std::endl;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, shareContext);
        if (!m_window) 
        {
            if (m_isPrimary) glfwTerminate();
            std::cout << "Failed to create GLFW window" << std::endl;
            return;
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);

        if (m_isPrimary)
        {
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
                std::cout << "Failed to initialize GLAD" << std::endl;
        }

        // Store pointer for static callbacks
        glfwSetWindowUserPointer(m_window, this);
        glfwSetKeyCallback(m_window, GLFWKeyCallback);
        glfwSetCursorPosCallback(m_window, GLFWCursorCallback);
        glfwSetMouseButtonCallback(m_window, GLFWMouseButtonCallback);
        glfwSetScrollCallback(m_window, GLFWScrollCallback);
        glfwSetFramebufferSizeCallback(m_window, GLFWResizeCallback);

        m_lastFrameTime = (float)glfwGetTime();
    }

    GLFWWindow::~GLFWWindow() 
    {
        if (m_window)     glfwDestroyWindow(m_window);
        if (m_isPrimary)  glfwTerminate();
    }

    // ── Lifecycle ────────────────────────────────────────────────
    void GLFWWindow::MakeContextCurrent() { glfwMakeContextCurrent(m_window); }
    bool GLFWWindow::ShouldClose()  const { return glfwWindowShouldClose(m_window); }
    void GLFWWindow::SwapBuffers() { glfwSwapBuffers(m_window); }
    void GLFWWindow::Clear()
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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

	// ── Input ────────────────────────────────────────────────────
    bool GLFWWindow::IsKeyPressed(Key key) const
    {
        int glfwKey = GLFW_KEY_UNKNOWN;
        switch (key) 
        {
        case Key::W:      
            glfwKey = GLFW_KEY_W;      
            break;
        case Key::A:      
            glfwKey = GLFW_KEY_A;      
            break;
        case Key::S:      
            glfwKey = GLFW_KEY_S;      
            break;
        case Key::D:      
            glfwKey = GLFW_KEY_D;      
            break;
        case Key::Q:      
            glfwKey = GLFW_KEY_Q;      
            break;
        case Key::E:      
            glfwKey = GLFW_KEY_E;      
            break;
        case Key::Escape: 
            glfwKey = GLFW_KEY_ESCAPE; 
            break;
        case Key::Space:  
            glfwKey = GLFW_KEY_SPACE;  
            break;
        default: 
            break;
        }
        return glfwGetKey(m_window, glfwKey) == GLFW_PRESS;
	}

    MousePosition GLFWWindow::GetMousePosition() const
    {
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);
        return { x, y };
	}

    void GLFWWindow::SetCursorCaptured(bool captured)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		m_firstMouseMove = true; // Reset mouse movement tracking when toggling capture
	}

    // ── Callback setters ─────────────────────────────────────────
	void GLFWWindow::SetKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
	void GLFWWindow::SetMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = callback; }
	void GLFWWindow::SetMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
	void GLFWWindow::SetScrollCallback(ScrollCallback callback) { m_scrollCallback = callback; }
	void GLFWWindow::SetResizeCallback(ResizeCallback callback) { m_resizeCallback = callback; }

	// ── GLFW static callback wrappers ─────────────────────────────
    void GLFWWindow::GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self && self->m_keyCallback)
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
	}

    void GLFWWindow::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self && self->m_mouseButtonCallback)
            self->m_mouseButtonCallback(TranslateMouseButton(button), action == GLFW_PRESS);
    }

    void GLFWWindow::GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        if (self && self->m_scrollCallback)
            self->m_scrollCallback(xoffset, yoffset);
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

    // ── Translation helpers ───────────────────────────────────────
    Key GLFWWindow::TranslateKey(int glfwKey)
    {
        switch (glfwKey) 
        {
        case GLFW_KEY_W:      return Key::W;
        case GLFW_KEY_A:      return Key::A;
        case GLFW_KEY_S:      return Key::S;
        case GLFW_KEY_D:      return Key::D;
        case GLFW_KEY_Q:      return Key::Q;
        case GLFW_KEY_E:      return Key::E;
        case GLFW_KEY_UP:     return Key::Up;
        case GLFW_KEY_DOWN:   return Key::Down;
        case GLFW_KEY_LEFT:   return Key::Left;
        case GLFW_KEY_RIGHT:  return Key::Right;
        case GLFW_KEY_ESCAPE: return Key::Escape;
        case GLFW_KEY_SPACE:  return Key::Space;
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