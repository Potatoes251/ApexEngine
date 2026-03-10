#ifndef IMGUI_MANAGER
#define IMGUI_MANAGER

#include "UI.h"
#include "ImDrawListWrapper.h"

#include <vector>

using std::vector;

// Forward declaration of IMGUI types to avoid including IMGUI headers in UI.h
struct ImFont;

namespace Apex::UserInterface
{
	namespace  FontSize
	{
		constexpr 	float	SMALL = 14.0f;
		constexpr 	float	DEFAULT = 18.0f;
		constexpr 	float	LARGE = 28.0f;
		constexpr 	float	TITLE = 36.0f;
	};

	struct FontInitializer
	{
		FontInitializer(FontID id, const std::string& file, float fontSize);

		FontID      	m_id;
		std::string		m_file;
		float			m_fontSize;
	};

	class ImGUIManager final : public IGUI
	{
	public:
		explicit ImGUIManager(GLFWwindow* nativeWindow);
		~ImGUIManager() override;

        void Initialize() override;
        void Shutdown()   override;
        void LoadFonts()  override;
        void BeginFrame() override;
        void EndFrame()   override;

        bool WantCaptureMouse() const override;
        bool WantCaptureKeyboard() const override;

        void PushID(const std::string& id) override;
        void PushID(int id) override;
        void PopID() override;

        void PushColor(StyleColor target, Color color) override;
        void PopColor() override;

        void Separator() override;
        void SameLine(float offsetFromLeft, float spacing) override;
        void SetNextItemWidth(float width)  override;
        LibMath::Vector2 GetAvailableSize() const override;
        LibMath::Vector2 GetCursorPos()     const override;
        void SetCursorPos(LibMath::Vector2 pos)   override;

        void BeginPanel(const std::string& title, bool* open) override;
        void EndPanel() override;

        void BeginChildPanel(const std::string& id,
                            float width, float height, bool border) override;
        void EndChildPanel() override;

        bool BeginTreeNode(const std::string& label,
                          TreeNodeFlags flags) override;
        void EndTreeNode() override;

        bool BeginContextMenu() override;
        void EndContextMenu() override;
        bool MenuItem(const std::string& label,
            bool enabled)                override;

        void BeginTooltip() override;
        void EndTooltip() override;

        void Text(const std::string& text) override;
        void TextDisabled(const std::string& text) override;
        void TextColored(Color color, const std::string& text) override;
        bool Button(const std::string& label) override;
        bool SmallButton(const std::string& label) override;
        bool Checkbox(const std::string& label, bool* value) override;
        bool SliderFloat(const std::string& label, float* value, float min, float max) override;
        bool SliderInt(const std::string& label, int* value, int min, int max) override;
        bool ColorEdit3(const std::string& label, float color[3]) override;
        bool InputText(const std::string& label, char* buffer, size_t bufferSize,
                       const std::string& hint) override;
        bool InvisibleButton(const std::string& id, LibMath::Vector2 size) override;

        bool IsItemHovered() const override;
        bool IsItemClicked(MouseButton button) const override;
        bool IsItemDoubleClicked(MouseButton button) const override;

		bool BeginDragSource() override;
        void SetDragPayload(const char* type, const void* data, size_t size) override;
		void EndDragSource() override;

		bool BeginDropTarget() override;
        const void* AcceptDragPayload(const char* type) override;
		void EndDropTarget() override;

        void DrawText2D(const std::string& text, float x, float y,
                        float size, FontID id) override;
        IDrawList* GetDrawList();

	private:
        static int ToImGuiMouseButton(MouseButton button);
        static int ToImGuiTreeFlags(TreeNodeFlags flags);
        static int ToImGuiCol(StyleColor target);

		GLFWwindow* m_window = nullptr;
		vector<ImFont*> m_fonts;
        ImDrawListWrapper m_drawListWrapper;

		static const std::vector<FontInitializer> m_fontDefinitions;
	};
}

#endif // !IMGUI_MANAGER