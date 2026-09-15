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
		constexpr 	float	SMALL = 16.0f;
		constexpr 	float	DEFAULT = 22.0f;
		constexpr 	float	LARGE = 30.0f;
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

        void LoadFonts()  override;
        void BeginFrame() override;
        void EndFrame()   override;
        void StartDockSpace() override;

        bool WantCaptureMouse() const override;
        bool WantCaptureKeyboard() const override;
        bool IsKeyPressed(UIKey key) const override;
        bool IsPanelFocused() const override;

        void PushID(const std::string& id) override;
        void PushID(int id) override;
        void PopID() override;

        void PushColor(StyleColor target, Color color) override;
        void PopColor(int count) override;
        void PushStyleVariable(StyleVariable var, float x, float y = 0.f) override;
        void PopStyleVariable(int count) override;
        void PushFont(FontID id) override;
        void PopFont() override;
        float GetTextLineHeight() const override;

        void Separator() override;
        void VerticalSeparator() override;
        void SameLine(float offsetFromLeft = 0.f, float spacing = -1.f) override;
        void NewLine() override;
        void AlignTextToFramePadding()  override;
        void SetNextItemWidth(float width)  override;
        LibMath::Vector2 GetAvailableSize() const override;
        LibMath::Vector2 GetCursorPos()     const override;
        void SetCursorPos(LibMath::Vector2 pos)   override;
        LibMath::Vector2 GetPanelPos()      const override;
        LibMath::Vector2 GetPanelSize()     const override;

        void BeginPanel(const std::string& title, bool* open, bool dock, bool titleBar, bool scrollable) override;
        void EndPanel() override;

        void BeginChildPanel(const std::string& id,
                            float width, float height, bool border) override;
        void EndChildPanel() override;

        bool BeginTreeNode(const std::string& label,
                          TreeNodeFlags flags) override;
        void EndTreeNode() override;

        bool BeginContextMenu(const std::string& id = "##ctx") override;
        bool BeginGridContextMenu() override;
        void EndContextMenu() override;
        bool MenuItem(const std::string& label, bool enabled) override;

        void SetKeyboardFocusHere() override;
        void SetClipboardText(const std::string& text) override;
        void SetNextWindowFocus() override;

        void OpenPopup(const std::string& id) override;
        bool BeginPopup(const std::string& id) override;
        void EndPopup() override;
        bool BeginPopupModal(const std::string& title, LibMath::Vector2 size = {}) override;
        void CloseCurrentPopup() override;
        void SetNextWindowSize(LibMath::Vector2 size) override;
        void SetNextWindowPos(LibMath::Vector2 pos)  override;

        void BeginTooltip() override;
        void EndTooltip() override;

        void Text(const std::string& text) override;
        void TextDisabled(const std::string& text) override;
        void TextColored(Color color, const std::string& text) override;
        bool Button(const std::string& label) override;
        bool ButtonSized(const std::string& label, LibMath::Vector2 size) override;
        bool ImageButton(const std::string& id, uint32_t textureID,
                         LibMath::Vector2 size,
                         Color tint = { 1.f, 1.f, 1.f, 1.f },
                         float padding = 2.f) override;
        bool SmallButton(const std::string& label) override;
        bool Checkbox(const std::string& label, bool* value) override;
        bool CheckboxFormat(const std::string& label, bool* value) override;
        bool SliderFloat(const std::string& label, float* value, float min, float max) override;
        bool SliderInt(const std::string& label, int* value, int min, int max) override;
        bool SliderFloatFormat(const std::string& label, float* value, float min, float max, float size, 
                                float sliderPos = -1.f, float spacing = -1.f) override;
        bool SliderIntFormat(const std::string& label, int* value, int min, int max, float size, 
                                float sliderPos = -1.f, float spacing = -1.f) override;
        bool ColorEdit3(const std::string& label, float color[3]) override;
		bool ColorEdit4(const std::string& label, float color[4]) override;
        bool InputText(const std::string& label, char* buffer, size_t bufferSize,
                       const std::string& hint) override;
        bool InputFloat(const std::string& label, float* value) override;
		bool InputInt(const std::string& label, int* value) override;

        bool Combo(const std::string& label, int* currentItem, const char* items) override;
        bool ComboFormat(const std::string& label, int* currentItem, const char* items) override;
        bool CollapsingHeader(const std::string& label, bool defaultOpen = false) override;
        bool InvisibleButton(const std::string& id, LibMath::Vector2 size) override;

		Vector2 GetScreenSize() override;

        bool IsItemHovered() const override;
        bool IsItemActive() const override;
        bool IsAnyItemActive() const override;
        Vector2 GetItemRectSize() const override;
        bool IsItemClicked(MouseButton button) const override;
        bool IsItemDoubleClicked(MouseButton button) const override;
        bool IsMouseReleased(MouseButton button = MouseButton::Left) const override;
        bool IsMouseClicked(MouseButton button = MouseButton::Left) const override;
        bool IsWindowHovered() const override;
        bool IsWindowDocked() const override;
        bool IsAnyItemHovered() const override;
        bool IsDragDropActive() const override;
        const char* GetDragDropPayloadType() const override;
        bool IsMouseDown(MouseButton button = MouseButton::Left) const override;
        LibMath::Vector2 GetMouseDelta() const override;
        float GetMouseWheelDelta() const override;
        LibMath::Vector2 GetMousePos() const override;

		bool BeginDragSource() override;
        void SetDragPayload(const char* type, const void* data, size_t size) override;
		void EndDragSource() override;

		bool BeginDropTarget() override;
        const void* AcceptDragPayload(const char* type) override;
		void EndDropTarget() override;

		void DrawText2D(const std::string& text, float x, float y, float size, FontID id) override;
		void DrawImageBackground(unsigned int textureId, Vector2 topLeftCorner, Vector2 bottomRightCorner) override;
        void DrawImage(uint32_t textureID, LibMath::Vector2 size, bool flipY = true) override;
        void DrawImageTinted(uint32_t textureID, LibMath::Vector2 size, Color tint, bool flipY = true) override;
        IDrawList* GetDrawList();
        IDrawList* GetBackgroundDrawList();
        Vector2 GetItemRectMin() const override;
        Vector2 GetItemRectMax() const override;

        void SetManagesCursor(bool manages) override;

        void BeginModalBlock(const std::string& name) override;
        void EndModalBlock() override;

        void BeginDisabled(bool disabled = true) override;
        void EndDisabled() override;

        void Dummy(LibMath::Vector2 size) override;

	private:

        void Initialize() override;
        void Shutdown()   override;

        static int ToImGuiMouseButton(MouseButton button);
        static int ToImGuiTreeFlags(TreeNodeFlags flags);
        static int ToImGuiCol(StyleColor target);

		GLFWwindow*         m_window = nullptr;
		vector<ImFont*>     m_fonts;
        ImDrawListWrapper   m_drawListWrapper;

		static const std::vector<FontInitializer> m_fontDefinitions;

        std::string m_modalName;
        bool        m_hasModal = false;
	};
}

#endif // !IMGUI_MANAGER