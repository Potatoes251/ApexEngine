#ifndef UI
#define UI

#include "IDrawList.h"
#include <string>
#include "LibMath/Vector/Vector2.h"

using LibMath::Vector2;

struct GLFWwindow;


namespace Apex::UserInterface
{
    struct Color
    {
        float m_r, m_g, m_b, m_a = 1.f;
	};

    // Pack a Color into a 32-bit RGBA integer (same bit layout as IM_COL32)
    inline uint32_t PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(g) << 8) | r;
    }

    enum class FontID
    {
        Small = 0,      //0
        Default,        //1
        Large,          //2
        Title,          //3
        Count           //Has to be at the end
    };

    enum class MouseButton 
    { 
        Left, 
        Right, 
        Middle 
    };

    // Tree node flags
    enum class TreeNodeFlags : uint32_t 
    {
        None = 0,
        OpenOnArrow = 1 << 0,
        Selected = 1 << 1,
        Leaf = 1 << 2,
        SpanAvailWidth = 1 << 3,
    };

    inline TreeNodeFlags operator|(TreeNodeFlags a, TreeNodeFlags b) 
    {
        return static_cast<TreeNodeFlags>(uint32_t(a) | uint32_t(b));
    }

    inline bool operator&(TreeNodeFlags a, TreeNodeFlags b) 
    {
        return (uint32_t(a) & uint32_t(b)) != 0;
    }

    // Style color targets
    enum class StyleColor 
    {
        Text, Button, ButtonHovered, ButtonActive,
        ChildBackground, Header, HeaderHovered
    };

    // Style variables that can be temporarily overridden
    enum class StyleVariable 
    {
        FramePadding, ItemSpacing, WindowPadding,
        IndentSpacing, FrameRounding, GrabRounding
    };

    // Keys usable in UI panels (ImGui-layer input, not gameplay input).
    // Add entries here as new shortcuts are needed.
    enum class UIKey
    {
        Delete,
        Backspace,
        Enter,
        Escape,
        Tab,
        Space,
        A, C, V, X, Z, Y,    // common editing shortcuts
        F2,                  // rename
        F5,                  // refresh
    };

	class IGUI
	{
	public:
                 IGUI() = default;
		virtual	~IGUI() = default;

        // Lifecycle
        virtual void LoadFonts() = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void StartDockSpace() = 0;

        // Input Passthrough
        virtual bool WantCaptureMouse()    const = 0;
        virtual bool WantCaptureKeyboard() const = 0;
        virtual bool IsKeyPressed(UIKey key) const = 0;
        virtual bool IsPanelFocused() const = 0;

        // ID stack (for list items, tiles…)
        virtual void PushID(const std::string& id) = 0;
        virtual void PushID(int id) = 0;
        virtual void PopID() = 0;

        // Style 
        virtual void PushColor(StyleColor target, Color color) = 0;
        virtual void PopColor(int count = 1) = 0;
        virtual void PushStyleVariable(StyleVariable var, float x, float y = 0.f) = 0;
        virtual void PopStyleVariable(int count = 1) = 0;
        virtual void PushFont(FontID id) = 0;
        virtual void PopFont() = 0;
        virtual float GetTextLineHeight() const = 0;

        // Layout 
        virtual void Separator() = 0;
        virtual void VerticalSeparator() = 0;
        virtual void SameLine(float offsetFromLeft = 0.f, float spacing = -1.f) = 0;
        virtual void NewLine() = 0;
        virtual void AlignTextToFramePadding() = 0;  
        virtual void SetNextItemWidth(float width) = 0;
        virtual LibMath::Vector2 GetAvailableSize() const = 0;
        virtual LibMath::Vector2 GetCursorPos() const = 0;
        virtual void SetCursorPos(LibMath::Vector2 pos) = 0;
        virtual LibMath::Vector2 GetPanelPos()  const = 0;  // top-left of current panel in screen coords
        virtual LibMath::Vector2 GetPanelSize() const = 0;  // size of current panel

        //  Panels & children 
        // BeginPanel / EndPanel - top-level named window
        virtual void BeginPanel(const std::string& title, bool* open = nullptr, bool dock = true, bool titleBar = false, bool scrollable = true) = 0;
        virtual void EndPanel() = 0;

        // BeginChildPanel / EndChildPanel - scrollable sub-region
        virtual void BeginChildPanel(const std::string& id,
                                    float width = 0.f,
                                    float height = 0.f,
                                    bool  border = false) = 0;
        virtual void EndChildPanel() = 0;

        // Tree nodes 
        // Returns true if the node is open (caller must call EndTreeNode if so).
        virtual bool BeginTreeNode(const std::string& label,
                                   TreeNodeFlags flags = TreeNodeFlags::None) = 0;
        virtual void EndTreeNode() = 0;

        // Context menu 
        // Must be called right after the widget it relates to.
        // Returns true if the menu is open (caller must call EndContextMenu if so).
        virtual bool BeginContextMenu(const std::string& id = "##ctx") = 0;
        virtual bool BeginGridContextMenu() = 0;  // right-click on empty window background
        virtual void EndContextMenu() = 0;
        virtual bool MenuItem(const std::string& label,
                              bool enabled = true) = 0;

        // Focus
        // Call before an InputText to auto-focus it on the next frame.
        virtual void SetKeyboardFocusHere() = 0;
        virtual void SetClipboardText(const std::string& text) = 0;
        virtual void SetNextWindowFocus() = 0;

        // Popup
        virtual void OpenPopup(const std::string& id) = 0;
        virtual bool BeginPopup(const std::string& id) = 0;
        virtual void EndPopup() = 0;
        // Modal stays open until CloseCurrentPopup() is called.
        // Pass size {0,0} to let ImGui auto-size.
        virtual bool BeginPopupModal(const std::string& title, LibMath::Vector2 size = {}) = 0;
        virtual void CloseCurrentPopup() = 0;
        virtual void SetNextWindowSize(LibMath::Vector2 size) = 0;
        virtual void SetNextWindowPos(LibMath::Vector2 pos) = 0;

        // Tooltip 
        virtual void BeginTooltip() = 0;
        virtual void EndTooltip() = 0;

        // Widgets 
        virtual void Text(const std::string& text) = 0;
        virtual void TextDisabled(const std::string& text) = 0;
        virtual void TextColored(Color color, const std::string& text) = 0;
        virtual bool Button(const std::string& label) = 0;
        virtual bool ButtonSized(const std::string& label, LibMath::Vector2 size) = 0;
        virtual bool ImageButton(const std::string& id, uint32_t textureID,
                                 LibMath::Vector2 size,
                                 Color tint = { 1.f, 1.f, 1.f, 1.f },
                                 float padding = 2.f) = 0;
        virtual bool SmallButton(const std::string& label) = 0;
        virtual bool Checkbox(const std::string& label, bool* value) = 0;
        virtual bool CheckboxFormat(const std::string& label, bool* value) = 0;
        virtual bool SliderFloat(const std::string& label, float* value, float min, float max) = 0;
        virtual bool SliderInt(const std::string& label, int* value, int min, int max) = 0;
        virtual bool SliderFloatFormat(const std::string& label, float* value, float min, float max, 
                                        float size, float sliderPos = -1.f, float spacing = -1.f) = 0;
        virtual bool SliderIntFormat(const std::string& label, int* value, int min, int max, 
                                        float size, float sliderPos = -1.f, float spacing = -1.f) = 0;
        virtual bool ColorEdit3(const std::string& label, float color[3]) = 0;
		virtual bool ColorEdit4(const std::string& label, float color[4]) = 0;
        virtual bool InputText(const std::string& label,
                               char* buffer, size_t bufferSize,
                               const std::string& hint = "") = 0;
        virtual bool InputFloat(const std::string& label, float* value) = 0;
		virtual bool InputInt(const std::string& label, int* value) = 0;

        virtual bool Combo(const std::string& label, int* currentItem, const char* items) = 0;
        virtual bool ComboFormat(const std::string& label, int* currentItem, const char* items) = 0;

        virtual bool CollapsingHeader(const std::string& label, bool defaultOpen = false) = 0;

        // Clickable invisible rectangle — used for custom tile/icon hitboxes.
        virtual bool InvisibleButton(const std::string& id, LibMath::Vector2 size) = 0;
        
        virtual Vector2 GetScreenSize() = 0;

        // Query 
        virtual bool IsItemHovered() const = 0;
        virtual bool IsItemActive()     const = 0;
        virtual bool IsAnyItemActive()     const = 0;
        virtual LibMath::Vector2 GetItemRectSize()  const = 0;
        virtual bool IsItemClicked(MouseButton button = MouseButton::Left) const = 0;
        virtual bool IsItemDoubleClicked(MouseButton button = MouseButton::Left) const = 0;
        virtual bool IsMouseReleased(MouseButton button = MouseButton::Left) const = 0;
        virtual bool IsMouseClicked(MouseButton button = MouseButton::Left) const = 0;
        virtual bool IsWindowHovered() const = 0;
        virtual bool IsWindowDocked() const = 0;
        virtual bool IsAnyItemHovered()   const = 0;
        virtual bool IsDragDropActive() const = 0;
        virtual const char* GetDragDropPayloadType() const = 0;
        virtual bool IsMouseDown(MouseButton button = MouseButton::Left) const = 0;
        virtual LibMath::Vector2 GetMouseDelta() const = 0;  // pixels moved since last frame
        virtual float GetMouseWheelDelta() const = 0;  // scroll ticks this frame
        virtual LibMath::Vector2 GetMousePos() const = 0;  // screen-space cursor position

        // Drag & Drop
        // Call BeginDragSource after InvisibleButton/item — returns true if dragging.
        virtual bool BeginDragSource() = 0;
        virtual void SetDragPayload(const char* type, const void* data, size_t size) = 0;
        virtual void EndDragSource() = 0;
        // Call BeginDropTarget after any item — returns true if a drag is hovering.
        virtual bool BeginDropTarget() = 0;
        virtual const void* AcceptDragPayload(const char* type) = 0;
        virtual void EndDropTarget() = 0;

        virtual void DrawText2D(const std::string& text, float x, float y, float size, FontID id) = 0;
        virtual void DrawImageBackground(unsigned int textureId, Vector2 topLeftCorner, Vector2 bottomRightCorner) = 0;
        virtual void DrawImage(uint32_t textureID, LibMath::Vector2 size, bool flipY = true) = 0;
        virtual void DrawImageTinted(uint32_t textureID, LibMath::Vector2 size, Color tint, bool flipY = true) = 0;

        // Background draw list — for custom tile rendering inside panels.
        // Lifetime: valid for the current frame only.
        virtual IDrawList* GetDrawList() = 0;
        virtual IDrawList* GetBackgroundDrawList() = 0;

        virtual Vector2 GetItemRectMin() const = 0;
        virtual Vector2 GetItemRectMax() const = 0;

        virtual void SetManagesCursor(bool manages) = 0;

        virtual void BeginModalBlock(const std::string& name) = 0;
        virtual void EndModalBlock() = 0;

        virtual void BeginDisabled(bool disabled = true) = 0;
        virtual void EndDisabled() = 0;

        virtual void Dummy(LibMath::Vector2 size) = 0;
	private:

        // Lifecycle
        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
	};

    IGUI* CreateGUI(void* nativeWindow);
}

#endif // !UI
