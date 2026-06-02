#ifndef MESH_VIEWER
#define MESH_VIEWER

// ============================================================
// MeshViewer.h - Floating mesh asset viewer / editor.
// Renders the mesh into an offscreen FBO and presents it
// inside an ImGui panel
//
// Features:
//   - Orbit / pan / zoom camera
//   - Display modes: Lit, Unlit, Wireframe, Normals
//   - Show flags: Grid, Bounding Box, Pivot, Normals overlay
//   - Details panel: vertex / triangle counts, bounds
//   - Reset view (F key)
// ============================================================

#include "UI.h"

#include "Window.h"
#include "RHI.h"

#include "ResourceHandle.h"
#include "ResourceManager.h"

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Matrix/Matrix4.h"

#include "FirstPersonCamera.h"

#include <filesystem>
#include <string>
#include <vector>

class Model;

namespace Fs = std::filesystem;
using namespace Apex::UserInterface;

namespace Apex::Editor
{
    enum class MeshDisplayMode { Lit, Unlit, Wireframe, Normals };

    class MeshViewer
    {
    public:
        explicit MeshViewer(IGUI* gui,
            Apex::Rendering::IRHI* rhi,
            Apex::Resources::ResourceManager* resourceManager);
        ~MeshViewer();
        MeshViewer(const MeshViewer&) = delete;
        MeshViewer& operator=(const MeshViewer&) = delete;

        void Open(const Fs::path& path, int instanceIndex = 0);
        void Close();
        void Focus();
        bool IsOpen() const { return m_open; }
        const Fs::path& GetPath() const { return m_path; }

        void Draw();

    private:
        // Layout
        void DrawToolbar();
        void DrawViewport();
        void DrawDetails();

        // Toolbar sub-sections
        void DrawToolbarDisplayMode();
        void DrawToolbarShowFlags();
        void DrawToolbarLighting();

        // Details sub-sections 
        void DrawDetailsMeshSection();
        void DrawDetailsBoundsSection();
        void DrawDetailsViewSection();

        // 3-D rendering into FBO 
        void RenderScene();
        void RenderGrid();
        void RenderBoundingBox();
        void RenderPivot();
        void RenderNormalsOverlay();

        // Viewport interaction 
        void HandleViewportInput(LibMath::Vector2 panelPos,
            LibMath::Vector2 panelSize,
            bool hovered, bool active);
        void ResetCamera();

        // Setup / teardown
        void LoadMesh(const Fs::path& path);
        void UnloadMesh();
        void FinalizeMeshLoad();
        void BuildShaders();
        void DestroyShaders();
        void RebuildFBO(float width, float height);
        void ComputeBounds();

        // Misc helpers
        void             UpdateOrbitCamera();
        LibMath::Matrix4 GetViewProj();
        LibMath::Vector3 GetCameraPosition() const;
        std::string      FileSizeString() const;

        // Core pointer
        IGUI* m_gui;
        Apex::Rendering::IRHI* m_rhi;
        Apex::Resources::ResourceManager* m_resourceManager;

        // Window state
        bool        m_open = false;
        bool        m_pendingFocus = false;
        int         m_instanceIndex = 0;
        Fs::path    m_path;
        std::string m_filename;
        uintmax_t   m_fileBytes = 0;

        // Mesh resource 
        Apex::Resources::ResourceHandle<Model> m_model;

        // Offscreen FBO 
        Apex::Rendering::RHIFrameBufferHandle m_fbo = 0;
        Apex::Rendering::RHITextureHandle     m_fboColor = 0;
        float m_fboWidth = 0.f;
        float m_fboHeight = 0.f;

        // Shaders (source embedded as strings, no .glsl files)
        Apex::Rendering::RHIShaderHandle m_shaderLit = 0;
        Apex::Rendering::RHIShaderHandle m_shaderUnlit = 0;
        Apex::Rendering::RHIShaderHandle m_shaderWireframe = 0;
        Apex::Rendering::RHIShaderHandle m_shaderNormals = 0;
        Apex::Rendering::RHIShaderHandle m_shaderFlat = 0;
        bool m_shadersReady = false;

        // Orbit camera
        Apex::Rendering::FirstPersonCamera m_camera;
        float            m_orbitYaw = 45.f;
        float            m_orbitPitch = 25.f;
        float            m_orbitDistance = 3.f;
        LibMath::Vector3 m_orbitTarget = { 0.f, 0.f, 0.f };

        // Display & show flags 
        MeshDisplayMode m_displayMode = MeshDisplayMode::Lit;
        bool  m_showGrid = true;
        bool  m_showBounds = false;
        bool  m_showPivot = true;
        bool  m_showNormals = false;
        float m_normalLength = 0.05f;
        float m_exposure = 1.f;

        // Directional light direction (world space, unit vector)
        LibMath::Vector3 m_lightDir = { 0.577f, 0.577f, 0.577f };

        // Mesh stats 
        uint32_t m_vertexCount = 0;
        uint32_t m_triangleCount = 0;
        bool     m_statsValid = false;

        // Bounding box 
        LibMath::Vector3 m_boundsMin = { 1e9f,  1e9f,  1e9f };
        LibMath::Vector3 m_boundsMax = { -1e9f, -1e9f, -1e9f };

        // Layout constants 
        static constexpr float DETAILS_WIDTH = 450.f;
        static constexpr float LABEL_COL_WIDTH = 180.f;
    };

} // namespace Apex::Editor

#endif