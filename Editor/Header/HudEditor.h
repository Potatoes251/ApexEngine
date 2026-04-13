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

namespace fs = std::filesystem;

namespace Apex::Editor
{
    struct WidgetSelection
    {
        Apex::UserInterface::UIPanel* m_panel = nullptr;
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
        Apex::UserInterface::UIPanel* srcParent = nullptr;
        int srcIndex = -1;

        // target
        Apex::UserInterface::UIPanel* dstParent = nullptr;
        int dstIndex = -1;
    };

    class HudEditor
    {
    public:
        explicit HudEditor(Apex::UserInterface::IGUI& gui,
            Apex::Windowing::IWindow& window,
            Apex::Resources::ResourceManager& resourceManager,
            Apex::Rendering::IRHI& rhi);
        ~HudEditor() = default;
        HudEditor(const HudEditor&) = delete;
        HudEditor& operator=(const HudEditor&) = delete;

        void Open(const fs::path& path, int instanceIndex = 0);
        void Close();
        void Focus();

        bool IsOpen()             const { return m_open; }
        const fs::path& GetPath() const { return m_path; }

        void Draw();

    private:

        void DrawToolbar();
        void DrawToolbarSave();
        void DrawToolbarLoad();
        void DrawCanvas();
        void DrawProperties();
        void DrawWidgetTree();
        void DrawTreeElement(Apex::UserInterface::UIElement* element, size_t index, Apex::UserInterface::UIPanel* parent);
        void DrawInsertLine(float y);
        void DrawTexturePicker();

        void HandleCanvasInput();

        void AddWidget(Apex::UserInterface::UIWidgetType type);
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

        Apex::UserInterface::IGUI& m_gui;
        Apex::Windowing::IWindow& m_window;
        Apex::Resources::ResourceManager& m_resourceManager;
        Apex::Rendering::IRHI& m_rhi;

        Apex::UserInterface::UICanvas m_canvas;

        bool        m_open = false;
        bool        m_pendingFocus = false;
        int         m_instanceIndex = 0;
        fs::path    m_path;
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
        std::vector<fs::path> m_textureFiles;

        static constexpr float CANVAS_ASPECT = 16.f / 9.f;
        static constexpr float PROPS_WIDTH = 500.f;
        static constexpr float TREE_WIDTH = 300.f;
        static constexpr float LABEL_WIDTH = 150.f;
    };
}

#endif