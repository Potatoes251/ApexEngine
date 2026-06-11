#ifndef WINDOW
#define WINDOW

#include <filesystem>
#include <functional>
#include <string>

#include "InputSystem.h"

using CloseCallback = std::function<void()>;

namespace Apex::Windowing
{

	struct MousePosition
	{
		double m_x;
		double m_y;
	};

	// ── Callbacks ─────────────────────────────────────────────────
	using KeyCallback = std::function<void(Input::Key, bool)>;
	using MouseMoveCallback = std::function<void(double dx, double dy)>;
	using MouseButtonCallback = std::function<void(Input::MouseButton, bool)>;
	using ScrollCallback = std::function<void(double xoffset, double yoffset)>;
	using ResizeCallback = std::function<void(int width, int height)>;
	using ExternalDropCallback = std::function<void(std::vector<std::filesystem::path>)>;

	// ── Window interface ─────────────────────────────────────────
	class IWindow
	{
	public:
		virtual ~IWindow() = default;

		// Lifecycle
		virtual bool ShouldClose() const = 0;
		virtual void SwapBuffers() = 0;
		virtual void Clear() = 0;
		virtual void Close() = 0;
		virtual void Focus() = 0;
		virtual void Maximize() = 0;
		virtual void UpdateDeltaTime() = 0;
		
		// Properties
		virtual int		GetWidth() const = 0;
		virtual int		GetHeight() const = 0;
		virtual float	GetAspectRatio() const = 0;
		virtual float	GetDeltaTime() const = 0;

		virtual void	SetWindowTitle(const std::string& title) = 0;
		virtual void	SetWindowIcon(const std::string& path) = 0;

		// Input
		virtual void	SetCursorCaptured(bool captured) = 0;
		virtual void	SetCrosshairCursor() = 0;
		virtual void	ResetCursorShape() = 0;

		// Callbacks
		virtual void SetKeyCallback(KeyCallback callback) = 0;
		virtual void SetMouseMoveCallback(MouseMoveCallback callback) = 0;
		virtual void SetMouseButtonCallback(MouseButtonCallback callback) = 0;
		virtual void SetScrollCallback(ScrollCallback callback) = 0;
		virtual void SetResizeCallback(ResizeCallback callback) = 0;

		virtual void MakeContextCurrent() = 0;
		virtual void* NativeHandle() const = 0;

		// External drag-and-drop — fires when files are dropped onto the window
		// from the OS file explorer.
		virtual void SetExternalDropCallback(ExternalDropCallback callback) = 0;

		// Opens a native OS file picker dialog parented to this window.
		// Returns the selected paths, or an empty vector if cancelled.
		// allowMultiple: let the user select more than one file at once.
		virtual std::vector<std::filesystem::path> OpenFileDialog(bool allowMultiple = true) = 0;
		virtual std::filesystem::path OpenFolderDialog() = 0;
		virtual std::filesystem::path SaveFileDialog(const std::string& defaultName = "") = 0;

		virtual void SetCloseCallback(CloseCallback callback) = 0;
		virtual void SetShouldClose(bool close) = 0;
	};

	// ── Factory ───────────────────────────────────────────────────
	// shareContext: pass another IWindow* to share its context.
	// Required so the editor window can use the same GL objects as the game.
	IWindow* CreateWindow(const std::string& title, int width, int height, IWindow* shareContext = nullptr);

	// Processes events for ALL open windows — one call per frame is enough.
	// This is a global operation, not tied to any specific window.
	void PollAllEvents();

	// Convenience: make a window's context current on the calling thread.
	// Use this in main.cpp to switch between game and editor windows.
	void MakeContextCurrent(IWindow& window);

} // namespace Apex::Windowing
#endif