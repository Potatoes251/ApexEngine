#ifndef GLFW_WINDOW
#define GLFW_WINDOW

// ================================================================================
// GLFWWindow.h — INTERNAL header. Do NOT include outside windowing implementation.
// ================================================================================

#include "Window.h"

// Forward declaration of GLFW types to avoid including GLFW headers in Window.h
struct GLFWwindow;

namespace Apex::Windowing
{
	class GLFWWindow final : public IWindow
	{
	public:

		GLFWWindow(const std::string& title, int width, int height);
		~GLFWWindow() override;
		// Disable Copying (Rule of Three)
		GLFWWindow(const GLFWWindow&) = delete;
		GLFWWindow& operator=(const GLFWWindow&) = delete;

		// Lifecycle
		bool ShouldClose() const override;
		void PollEvents() override;
		void SwapBuffers() override;
		void Close() override;

		// Properties
		int		GetWidth() const override;
		int		GetHeight() const override;
		float	GetAspectRatio() const override;
		float	GetDeltaTime() const override;

		// Input
		bool			IsKeyPressed(Key key) const override;
		MousePosition	GetMousePosition() const override;
		void			SetCursorCaptured(bool captured) override;

		// Callbacks
		void SetKeyCallback(KeyCallback callback) override;
		void SetMouseMoveCallback(MouseMoveCallback callback) override;
		void SetMouseButtonCallback(MouseButtonCallback callback) override;
		void SetScrollCallback(ScrollCallback callback) override;
		void SetResizeCallback(ResizeCallback callback) override;

		// Native handle
		void* NativeHandle() const override { return m_window; }

	private:
		GLFWwindow* m_window = nullptr;
		int			m_width = 0;
		int			m_height = 0;
		float		m_lastFrameTime = 0.0f;
		float		m_deltaTime = 0.0f;

		double		m_lastMouseX = 0.0;
		double		m_lastMouseY = 0.0;
		bool 		m_firstMouseMove = true;

		KeyCallback			m_keyCallback;
		MouseMoveCallback	m_mouseMoveCallback;
		MouseButtonCallback m_mouseButtonCallback;
		ScrollCallback		m_scrollCallback;
		ResizeCallback		m_resizeCallback;

		// GLFW static callback wrappers
		static void	GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void	GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos);
		static void	GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void	GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void	GLFWResizeCallback(GLFWwindow* window, int width, int height);

		static Key TranslateKey(int glfwKey);
		static MouseButton TranslateMouseButton(int glfwButton);

	};
} // namespace Apex::Windowing

#endif