#ifndef HUD_EDITOR_H
#define HUD_EDITOR_H

#include "UI.h"
#include "Hud.h"
#include "ResourceManager.h"
#include "Window.h"
#include "IDrawList.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Apex::Editor
{
    class HudEditor
    {
    public:
        explicit HudEditor(Apex::UserInterface::IGUI& gui,
            Apex::Windowing::IWindow& window,
            Apex::Resources::ResourceManager& resourceManager);
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

        void DrawCanvas(Apex::UserInterface::IDrawList* drawList,
            LibMath::Vector2 canvasMin,
            LibMath::Vector2 canvasSize);
        void DrawProperties();
        void DrawWidgetTree();

        void HandleCanvasInput(LibMath::Vector2 canvasMin, LibMath::Vector2 canvasSize);

        void AddWidget(Apex::UserInterface::UIWidgetType type);
        void DeleteSelected();

        LibMath::Vector2 CanvasToScreen(LibMath::Vector2 normalized, LibMath::Vector2 canvasMin, LibMath::Vector2 canvasSize) const;
        LibMath::Vector2 ScreenToCanvas(LibMath::Vector2 screen, LibMath::Vector2 canvasMin, LibMath::Vector2 canvasSize) const;

        Apex::UserInterface::IGUI& m_gui;
        Apex::Windowing::IWindow& m_window;
        Apex::Resources::ResourceManager& m_resourceManager;

        Apex::UserInterface::UICanvas m_canvas;

        bool        m_open = false;
        bool        m_pendingFocus = false;
        int         m_instanceIndex = 0;
        fs::path    m_path;
        bool        m_dirty = false;

        int         m_selectedIndex = -1;
        bool        m_dragging = false;
        LibMath::Vector2 m_dragOffset = { 0, 0 };

        static constexpr float CANVAS_ASPECT = 16.f / 9.f;
        static constexpr float PROPS_WIDTH = 280.f;
        static constexpr float TREE_HEIGHT = 180.f;
    };
}

#endif