#ifndef UI
#define UI

#include <string>

struct GLFWwindow;

namespace Apex::UserInterface
{
    enum class FontID
    {
        Small = 0,      //0
        Default,        //1
        Large,          //2
        Title,          //3
        Count           //Has to be at the end
    };

	class IGUI
	{
	public:
        IGUI() = default;
		virtual	~IGUI() = default;

        // Lifecycle — call once after window/RHI are initialised.
        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;

        virtual void LoadFonts() = 0;

        // Per-frame — BeginFrame after PollEvents, EndFrame before SwapBuffers.
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        // Returns true when ImGui wants to consume input,
        // so the engine can suppress camera movement.
        virtual bool WantCaptureMouse()    const = 0;
        virtual bool WantCaptureKeyboard() const = 0;

        // Panel helpers — always call between BeginFrame / EndFrame.
        virtual void BeginPanel(const std::string& title) = 0;
        virtual void EndPanel() = 0;

        virtual void Text(const std::string& text) = 0;
        virtual bool Button(const std::string& label) = 0;
        virtual bool Checkbox(const std::string& label, bool* v) = 0;
        virtual bool SliderFloat(const char* label, float* v, float min, float max) = 0;
        virtual bool SliderInt(const char* label, int* v, int min, int max) = 0;
        virtual bool ColorEdit3(const char* label, float color[3]) = 0;
        virtual void Separator() = 0;
        virtual void SameLine() = 0;

        virtual void DrawText2D(const std::string& text, float x, float y, float size, FontID id) = 0;
	private:
	};

    IGUI* CreateGUI(void* nativeWindow);
}

#endif // !UI
