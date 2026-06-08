#ifndef TEXTURE_EDITOR
#define TEXTURE_EDITOR

// ============================================================
// TextureViewer.h - Floating popup that previews a texture
// asset with dimensions, channels and file size info.
// ============================================================
#include "UI.h"
#include "MetaFile.h"
#include "Window.h"

#include <filesystem>
#include <string>

namespace Fs = std::filesystem;
using namespace Apex::UserInterface;

namespace Apex::Editor
{
	enum class TextureBackground { Checkerboard, Black, White };

	// GPU block-compression formats supported by the editor.
	// None  - raw RGBA8, no compression
	// BC1   - RGB  (no alpha), ~6:1 compression
	// BC3   - RGBA (smooth alpha), ~4:1 compression
	// BC4   - single channel (R), grayscale / roughness maps
	// BC5   - two channels (RG), normal maps
	enum class CompressionFormat { None, BC1, BC3, BC4, BC5 };

	class TextureEditor
	{
	public:
		explicit TextureEditor(IGUI& gui, Apex::Windowing::IWindow& window);
		~TextureEditor() { UnloadTexture(); }
		TextureEditor(const TextureEditor&) = delete;
		TextureEditor& operator=(const TextureEditor&) = delete;

		// Open/close
		void Open(const Fs::path& path, int instanceIndex = 0);
		void Close();
		// bring this window to front
		void Focus() { m_pendingFocus = true; }
		bool IsOpen() const { return m_open; }
		const Fs::path& GetPath() const { return m_path; }

		// Call once per frame inside an active ImGui frame
		void Draw();
		
	private:
		// Sub-sections
		void DrawToolbar();
		void DrawToolbarActions();   
		void DrawToolbarViewOptions();   
		void DrawToolbarMip();   
		void DrawToolbarZoom();   
		void DrawToolbarExposure();
		void DrawToolbarSave();
		void DrawToolbarEyeDropper();
		void DrawChannelButtons();

		void DrawViewport();
		void HandleViewportInput(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize, bool hovered, bool active);
		void HandleHoverPixelInfo(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize);
		void DrawTiledImage(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize, LibMath::Vector2 cursorStart);
		void DrawImageBorder(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize);

		void DrawDetails();
		void DrawDetailsInfoStrip();
		void DrawDetailsLODSection();
		void DrawDetailsViewSection();
		void DrawDetailsFileSection();
		void DrawDetailsColorSection();
		void DrawDetailsCompressionSection();
		void DrawDetailsStatisticsSection();
		void DrawDetailsResizeSection();

		void DrawCheckerboard(LibMath::Vector2 min, LibMath::Vector2 max);
		void DrawBackground(LibMath::Vector2 min, LibMath::Vector2 max);

		// Helpers
		void LoadTexture();
		void UnloadTexture();
		void ApplyCompression();       // CPU-compress m_pixels and re-upload
		void LoadOrCreateMeta();       // read .meta sidecar, create if missing
		void SaveMeta();               // write .meta sidecar

		void SaveExport();
		void ResizeTexture(int newWidth, int newHeight);
		bool IsHDR() const { return !m_pixelsHDR.empty(); }

		void ComputeStatistics();

		Color ChannelTint() const;
		std::string FormatString() const;
		std::string MipCountString() const;
		std::string FileSizeString() const;
		std::string CompressionFormatString(CompressionFormat format) const;
		std::string FormatBytes(uintmax_t bytes) const;

		static std::string TruncatePath(const std::string& str, int maxChars);
		void TextWithTooltip(const std::string& label, const std::string& value, int maxChars = 22);

		LibMath::Vector2 ImageScreenOrigin(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize) const;
		bool HandleColorPick(LibMath::Vector2 imageOrigin);
		void DrawZoomLabel(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize);
		void DrawHoverInfo(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize);

		// Core
		IGUI& m_gui;
		Apex::Windowing::IWindow& m_window;
		bool  m_open = false;
		bool  m_pickerModeActive = false;
		bool  m_pendingFocus = false;
		int   m_instanceIndex = 0;

		// Texture GPU resource
		uint32_t    m_textureID = 0;
		int         m_width = 0;
		int         m_height = 0;
		int         m_channels = 0;
		int         m_mipCount = 0;
		uintmax_t   m_fileBytes = 0;
		std::string m_filename;
		Fs::path    m_path;

		// CPU-side pixels kept alive for color picker
		std::vector<unsigned char> m_pixels;  // RGBA, row-major
		std::vector<float>         m_pixelsHDR; // RGBA f32, row-major (HDR .hdr files)

		// Resize state (Details panel)
		int  m_resizeWidth = 0;   // target width  (0 = use current)
		int  m_resizeHeight = 0;   // target height (0 = use current)
		bool m_resizeLockAspectRatio = true; // lock aspect ratio

		// Hover pixel info - updated every frame when mouse is over image
		bool  m_hoverValid = false;
		int   m_hoverX = 0;
		int   m_hoverY = 0;
		Color m_hoverColor = { 0.f, 0.f, 0.f, 1.f };

		// Per-channel image statistics (computed once after load)
		struct ChannelStats { float m_min = 1.f, m_max = 0.f, m_avg = 0.f; };
		ChannelStats m_stats[4];  // R G B A
		bool         m_statsValid = false;

		// Compression & meta 
		CompressionFormat m_compression = CompressionFormat::None;
		bool              m_metaDirty = false;
		MetaFile          m_meta;

		// Viewport state
		float              m_zoom = 1.f;
		LibMath::Vector2   m_panOffset = { 0.f, 0.f };  // pixels panned from center
		bool			   m_showR = true;  // channel mask - each toggles independently
		bool               m_showG = true;
		bool               m_showB = true;
		bool               m_showA = true;
		TextureBackground  m_background = TextureBackground::Checkerboard;
		int                m_tileRepeat = 1;    // 1, 2, or 3
		int                m_mipLevel = 0;
		float              m_exposure = 1.f;

		// Color picker
		bool  m_hasPickedColor = false;
		Color m_pickedColor = { 0.f, 0.f, 0.f, 1.f };
		int   m_pickedX = 0;
		int   m_pickedY = 0;

		// Checkerboard tile size in screen pixels
		static constexpr float CHECKER_TILE_SIZE = 16.f;
		// Details panel width
		static constexpr float DETAILS_WIDTH = 470.f;
		// Label column width - all labels snap to this offset so widgets align
		static constexpr float LABEL_COLUMN_WIDTH = 150.f;
	};
}

#endif