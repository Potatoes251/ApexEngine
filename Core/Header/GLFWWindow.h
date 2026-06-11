#ifndef GLFW_WINDOW
#define GLFW_WINDOW

// ================================================================================
// GLFWWindow.h - INTERNAL header. Do NOT include outside windowing implementation.
// ================================================================================

#include "Window.h"

// Forward declaration of GLFW types to avoid including GLFW headers in Window.h
struct GLFWwindow;
struct GLFWcursor;

namespace Apex::Windowing
{
	class GLFWWindow final : public IWindow
	{
	public:
		// shareContext: another GLFWwindow* whose GL context to share.
		// Pass nullptr for the primary (game) window.
		GLFWWindow(const std::string& title, int width, int height, GLFWwindow* shareContext = nullptr);
		~GLFWWindow() override;
		// Disable Copying (Rule of Three)
		GLFWWindow(const GLFWWindow&) = delete;
		GLFWWindow& operator=(const GLFWWindow&) = delete;

		// Lifecycle
		bool ShouldClose() const override;
		void SwapBuffers() override;
		void Clear() override;
		void Close() override;
		void Focus() override;
		void Maximize() override;
		void UpdateDeltaTime() override;
		
		// Properties
		int		GetWidth() const override;
		int		GetHeight() const override;
		float	GetAspectRatio() const override;
		float	GetDeltaTime() const override;
		
		void	SetWindowTitle(const std::string& title) override;
		void	SetWindowIcon(const std::string& path) override;

		// Input
		void	SetCursorCaptured(bool captured) override;
		void	SetCrosshairCursor() override;
		void	ResetCursorShape() override;


		// Callbacks
		void SetKeyCallback(KeyCallback callback) override;
		void SetMouseMoveCallback(MouseMoveCallback callback) override;
		void SetMouseButtonCallback(MouseButtonCallback callback) override;
		void SetScrollCallback(ScrollCallback callback) override;
		void SetResizeCallback(ResizeCallback callback) override;

		void  MakeContextCurrent()   override;

		// Native handle
		void* NativeHandle() const override { return m_window; }

		void SetExternalDropCallback(ExternalDropCallback callback) override;

		std::vector<std::filesystem::path> OpenFileDialog(bool allowMultiple = true) override;
		std::filesystem::path OpenFolderDialog() override;
		std::filesystem::path SaveFileDialog(const std::string& defaultName = "") override;


	private:
		// GLFW static callback wrappers
		static void	GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void	GLFWCursorCallback(GLFWwindow* window, double xpos, double ypos);
		static void	GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void	GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void	GLFWResizeCallback(GLFWwindow* window, int width, int height);
		static void GLFWDropCallback(GLFWwindow* window, int count, const char** paths);

		void		SetCloseCallback(CloseCallback callback);
		void		SetShouldClose(bool close);

		static Input::Key TranslateKey(int glfwKey);
		static Input::MouseButton TranslateMouseButton(int glfwButton);

		GLFWwindow* m_window = nullptr;
		GLFWcursor* m_crosshairCursor = nullptr;
		bool        m_isPrimary = true;   // only primary calls glfwInit/Terminate
		int			m_width = 0;
		int			m_height = 0;
		float		m_lastFrameTime = 0.0f;
		float		m_deltaTime = 0.0f;

		double		m_lastMouseX = 0.0;
		double		m_lastMouseY = 0.0;
		bool 		m_firstMouseMove = true;

		KeyCallback			 m_keyCallback;
		MouseMoveCallback	 m_mouseMoveCallback;
		MouseButtonCallback  m_mouseButtonCallback;
		ScrollCallback		 m_scrollCallback;
		ResizeCallback		 m_resizeCallback;
		ExternalDropCallback m_dropCallback;
		CloseCallback		 m_closeCallback;
	};
} // namespace Apex::Windowing

#endif