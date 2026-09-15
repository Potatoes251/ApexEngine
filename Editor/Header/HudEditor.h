#ifndef HUD_EDITOR_H
#define HUD_EDITOR_H

#include "UI.h"
#include "Hud.h"
#include "RHI.h"
#include "ResourceManager.h"
#include "Window.h"
#include "IDrawList.h"

#include <filesystem>
#include <string>

namespace Fs = std::filesystem;

using namespace Apex::UserInterface;

namespace Apex::Editor
{
    struct WidgetSelection
    {
        UIPanel* m_panel = nullptr;
        int  m_index = -1;

        bool IsValid()  const { return m_index >= 0; }
        bool IsTopLevel() const { return m_panel == nullptr; }

        bool operator==(const WidgetSelection& o) const
        {
            return m_panel == o.m_panel && m_index == o.m_index;
        }
        bool operator!=(const WidgetSelection& o) const { return !(*this == o); }
    };

    struct PendingTreeOp
    {
        enum class Kind { None, ReorderBefore, ReorderAfter, Reparent } kind = Kind::None;

        // source
        UIPanel* srcParent = nullptr;
        int srcIndex = -1;

        // target
        UIPanel* dstParent = nullptr;
        int dstIndex = -1;
    };

    class HudEditor
    {
    public:
        explicit HudEditor(Apex::UserInterface::IGUI* gui,
            Apex::Resources::ResourceManager* resourceManager,
            Apex::Rendering::IRHI* rhi);
        ~HudEditor() = default;
        HudEditor(const HudEditor&) = delete;
        HudEditor& operator=(const HudEditor&) = delete;

        void Open(const Fs::path& path, int instanceIndex = 0);
        void Close();
        void Focus();

        bool IsOpen()             const { return m_open; }
        const Fs::path& GetPath() const { return m_path; }

        void Draw();

    private:

        void DrawToolbar();
        void DrawToolbarSave();
        void DrawToolbarLoad();
        void DrawCanvas();
        void DrawProperties();
        void DrawRow(const char* label);
        void DrawColorRow(const char* label, const char* id, UIColor& color);
        void DrawBaseProperties(UIElement* element);
        void DrawTypeSpecificProperties(UIElement* element);
        void DrawTextProperties(UIText* text);
		void DrawImageProperties(UIImage* image);
		void DrawPanelProperties(UIPanel* panel);
		void DrawButtonProperties(UIButton* button);
		void DrawProgressBarProperties(UIProgressBar* progressBar);
        void DrawWidgetTree();
        void DrawTreeElement(Apex::UserInterface::UIElement* element, size_t index, Apex::UserInterface::UIPanel* parent);
        TreeNodeFlags GetTreeNodeFlags(UIElement* element, const WidgetSelection& selection);
        void HandleTreeInteraction(UIElement* element, size_t index, UIPanel* parent, UIPanel* asPanel, const WidgetSelection& selection);
        void HandleDropOnElement(size_t index, UIPanel* parent, UIPanel* asPanel);
        void DrawInsertLine(float y);
        void DrawTexturePicker();
        std::string DrawTextureGrid(int cols);
        void CommitTextureSelection(const std::string& key);

        void HandleCanvasInput();

        void AddWidget(Apex::UserInterface::UIWidgetType type);
        void HandleAddingWidget(std::unique_ptr<Apex::UserInterface::UIElement>& element);
        void DeleteSelected();
        
        Apex::UserInterface::UIElement* ResolveSelected() const;
        Apex::UserInterface::UIElement* ResolveElement(const WidgetSelection& selection) const;

        void ApplyPendingTreeOp();

        // Converts normalised anchor-relative pos to screen pos inside canvas
        LibMath::Vector2 WidgetToCanvas(const Apex::UserInterface::UIElement& element) const;
        // Converts absolute canvas screen pos to anchor-relative normalised pos
        LibMath::Vector2 CanvasToWidget(LibMath::Vector2 screenPos, Apex::UserInterface::UIAnchor anchor) const;
        LibMath::Vector2 AnchorOrigin(Apex::UserInterface::UIAnchor anchor) const;

        void ScanTextures();
        std::string ShortenName(const std::string& name, size_t maxLen = 14);

        IGUI* m_gui;
        Apex::Resources::ResourceManager* m_resourceManager;
        Apex::Rendering::IRHI* m_rhi;

        UICanvas m_canvas;

        bool        m_open = false;
        bool        m_pendingFocus = false;
        int         m_instanceIndex = 0;
        Fs::path    m_path;
        bool        m_dirty = false;

        // Canvas viewport state
        LibMath::Vector2 m_canvasMin = { 0, 0 };
        LibMath::Vector2 m_canvasSize = { 1, 1 };

        // Selection + drag
        WidgetSelection  m_selection;
        bool             m_dragging = false;
        LibMath::Vector2 m_dragStartMouse;
        LibMath::Vector2 m_dragStartPos;

        // Tree drag-reorder state
        PendingTreeOp    m_pendingOp;
        bool             m_showInsertLine = false;
        float            m_insertLineY = 0.f;

        // Texture picker
        bool m_showTexturePicker = false;
        bool m_texPickerOpened = false;
        std::vector<Fs::path> m_textureFiles;

        static constexpr float CANVAS_ASPECT = 16.f / 9.f;
        static constexpr float PROPS_WIDTH = 500.f;
        static constexpr float TREE_WIDTH = 300.f;
        static constexpr float LABEL_WIDTH = 150.f;
    };
}

#endif