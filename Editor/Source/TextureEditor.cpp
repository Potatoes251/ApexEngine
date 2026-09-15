#include "TextureEditor.h"

#include "EditorIcon.h"

#include "Log.h"

extern "C"
{
    unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    float*         stbi_loadf(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    void           stbi_image_free(void* retval_from_stbi_load);
}

#include <iostream>

#include <stb_dxt.h>
#include <stb_image_write.h>
#include <stb_image_resize2.h>

#include <glad/glad.h>

#include <iomanip>

namespace Apex::Editor
{
    TextureEditor::TextureEditor(IGUI& gui, Apex::Windowing::IWindow& window) : m_gui(gui), m_window(window) {}

    void TextureEditor::Open(const Fs::path& path, int instanceIndex)
    {
        UnloadTexture();
        m_path = path;
        LoadOrCreateMeta();
        LoadTexture();
        m_open = true;
        m_pendingFocus = false;
        m_instanceIndex = instanceIndex;
        m_zoom = 1.f;
        m_panOffset = { 0.f, 0.f };
        m_showR = true;
        m_showG = true;
        m_showB = true;
        m_showA = true;
        m_background = TextureBackground::Checkerboard;
        m_tileRepeat = 1;
        m_mipLevel = 0;
        m_exposure = 1.f;
        m_hasPickedColor = false;
        m_pickerModeActive = false;
        m_metaDirty = false;
    }

    void TextureEditor::Close()
    {
        if (m_pickerModeActive)
        {
            m_pickerModeActive = false;
            m_window.ResetCursorShape();
            m_gui.SetManagesCursor(true);
        }
        UnloadTexture();
        m_open = false;
    }

    void TextureEditor::Draw()
    {
        if (!m_open) return;

        m_gui.PushFont(FontID::Default);

        // Cascade each instance slightly so multiple windows are distinguishable.
        constexpr float CASCADE = 30.f;
        float offset = static_cast<float>(m_instanceIndex) * CASCADE;
        LibMath::Vector2 screen = m_gui.GetScreenSize();
        m_gui.SetNextWindowPos(LibMath::Vector2(offset, offset));
        m_gui.SetNextWindowSize(LibMath::Vector2(screen[0] - offset, screen[1] - offset));

        if (m_pendingFocus)
        {
            m_gui.SetNextWindowFocus();
            m_pendingFocus = false;
        }

        m_gui.BeginPanel("Texture Editor - " + m_filename, &m_open, false, true);

        if (!m_open)
        {
            Close();
            m_gui.EndPanel();
            m_gui.PopFont();
            return;
        }

        DrawToolbar();
        m_gui.Separator();

        // Left: viewport | Right: details - side by side
        float totalHeight = m_gui.GetAvailableSize()[1];

        m_gui.BeginChildPanel("##te_viewport", -DETAILS_WIDTH, totalHeight, false);
        DrawViewport();
        m_gui.EndChildPanel();

        m_gui.SameLine();

        m_gui.BeginChildPanel("##te_details", DETAILS_WIDTH, totalHeight, true);
        DrawDetails();
        m_gui.EndChildPanel();

        m_gui.EndPanel();

        m_gui.PopFont();
    }

    void TextureEditor::DrawToolbar()
    {
        m_gui.PushStyleVariable(StyleVariable::FramePadding, 6.f, 4.f);

        DrawToolbarActions();

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        DrawToolbarViewOptions();

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        DrawToolbarMip();

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        DrawToolbarZoom();

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        DrawToolbarExposure();

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        if (m_gui.Button("Fit")) { m_zoom = 1.f; m_panOffset = { 0.f, 0.f }; }

        m_gui.PopStyleVariable();
    }

    void TextureEditor::DrawToolbarActions()
    {
        DrawToolbarSave();
        m_gui.SameLine(0.f, 16.f);
        DrawToolbarEyeDropper();

        m_gui.SameLine(0.f, 16.f);
        if (IsHDR())
        {
            m_gui.PushColor(StyleColor::Button, Color{ 0.3f, 0.3f, 0.3f, 0.5f });
            m_gui.PushColor(StyleColor::ButtonHovered, Color{ 0.3f, 0.3f, 0.3f, 0.5f });
            m_gui.Button("Compress");  // non-functional, just visual indicator
            m_gui.PopColor(2);
            if (m_gui.IsItemHovered())
            {
                m_gui.BeginTooltip();
                m_gui.Text("BC compression not available for HDR textures");
                m_gui.EndTooltip();
            }
        }
        else
        {
            // Only highlight if the current setting is different from the saved meta state
            auto savedFormat = static_cast<CompressionFormat>(m_meta.GetInt("compression", 0));
            bool needsApply = (m_compression != savedFormat) || m_metaDirty;
            if (needsApply)
            {
                m_gui.PushColor(StyleColor::Button, Color{ 0.15f, 0.55f, 0.15f, 1.f });
                m_gui.PushColor(StyleColor::ButtonHovered, Color{ 0.20f, 0.70f, 0.20f, 1.f });
            }
            if (m_gui.Button("Compress")) ApplyCompression();
            if (needsApply) 
            { 
                m_gui.PopColor(2);
            }
        }
        m_gui.SameLine(0.f, 16.f);
        if (m_gui.Button("Reimport"))
        {
            UnloadTexture();
            LoadTexture();
            m_mipLevel = 0;
            LoadOrCreateMeta();
        }
    }

    void TextureEditor::DrawToolbarViewOptions()
    {
        DrawChannelButtons();

        m_gui.SameLine(0.f, 16.f);
        const char* backgroundLabel = "CHECKBOARD";
        if (m_background == TextureBackground::Black) backgroundLabel = "BLACK";
        else if (m_background == TextureBackground::White) backgroundLabel = "WHITE";
        if (m_gui.ButtonSized(backgroundLabel, LibMath::Vector2(150.f, 0.f)))
        {
            switch (m_background)
            {
            case TextureBackground::Checkerboard: 
                m_background = TextureBackground::Black; 
                break;
            case TextureBackground::Black:        
                m_background = TextureBackground::White; 
                break;
            case TextureBackground::White:        
                m_background = TextureBackground::Checkerboard; 
                break;
            }
        }

        m_gui.SameLine(0.f, 16.f);
        std::string tileLabel = std::to_string(m_tileRepeat) + "x";
        if (m_gui.Button(tileLabel.c_str()))
            m_tileRepeat = (m_tileRepeat % 3) + 1;
    }

    void TextureEditor::DrawToolbarMip()
    {
        std::string items;
        for (int i = 0; i < m_mipCount; ++i)
            items += "Mip " + std::to_string(i) + '\0';
        items += '\0';

        m_gui.SetNextItemWidth(110.f);
        if (m_gui.Combo("##mip", &m_mipLevel, items.c_str()))
        {
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, m_mipLevel);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_mipLevel);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void TextureEditor::DrawToolbarZoom()
    {
        if (m_gui.ButtonSized("+", LibMath::Vector2(28.f, 0.f))) m_zoom = std::min(8.f, m_zoom + 0.25f);
        m_gui.SameLine(0.f, 8.f);
        if (m_gui.ButtonSized("-", LibMath::Vector2(28.f, 0.f))) m_zoom = std::max(0.05f, m_zoom - 0.25f);
        m_gui.SameLine(0.f, 16.f);
        m_gui.SetNextItemWidth(120.f);
        m_gui.SliderFloat("##zoom", &m_zoom, 0.05f, 8.f);
        m_gui.SameLine(0.f, 16.f);
        m_gui.AlignTextToFramePadding();
        m_gui.Text(std::to_string(static_cast<int>(m_zoom * 100.f)) + "%");
    }

    void TextureEditor::DrawToolbarExposure()
    {
        m_gui.AlignTextToFramePadding();
        m_gui.Text("EV:");
        m_gui.SameLine(0.f, 8.f);
        m_gui.SetNextItemWidth(80.f);
        m_gui.SliderFloat("##ev", &m_exposure, 0.f, 4.f);
    }

    void TextureEditor::DrawToolbarSave()
    {
        uint32_t saveIcon = EditorIcon::Get("ApexAssets/Icons/save.png");
        if (saveIcon != 0)
        {
            if (m_gui.ImageButton("##save", saveIcon, LibMath::Vector2(22.f, 22.f)))
                SaveExport();
            if (m_gui.IsItemHovered()) 
            { 
                m_gui.BeginTooltip(); 
                m_gui.Text("Export texture..."); 
                m_gui.EndTooltip(); 
            }
        }
        else
        {
            if (m_gui.Button("Save")) SaveExport();
        }
    }

    void TextureEditor::DrawToolbarEyeDropper()
    {
        // Eyedropper / color picker mode toggle
        uint32_t eyeIcon = EditorIcon::Get("ApexAssets/Icons/color_picker.png");
        if (m_pickerModeActive)
            m_gui.PushColor(StyleColor::Button, Color{ 0.2f, 0.5f, 0.9f, 1.f });
        bool pickerClicked = false;
        if (eyeIcon != 0)
            pickerClicked = m_gui.ImageButton("##picker", eyeIcon,
                LibMath::Vector2(22.f, 22.f),
                Color{ 1.f, 1.f, 1.f, 1.f }, 0.f);
        else
            pickerClicked = m_gui.Button(m_pickerModeActive ? "[X]" : "[.]");

        if (m_pickerModeActive)
            m_gui.PopColor();   // always pop � state captured before the button

        if (pickerClicked)
        {
            m_pickerModeActive = !m_pickerModeActive;
            if (m_pickerModeActive)
            {
                m_gui.SetManagesCursor(false);
                m_window.SetCrosshairCursor();
            }
            else
            {
                m_window.ResetCursorShape();
                m_gui.SetManagesCursor(true);
            }
        }

        if (m_gui.IsItemHovered())
        {
            m_gui.BeginTooltip();
            m_gui.Text("Color Picker (Escape to cancel)");
            m_gui.EndTooltip();
        }
    }

    void TextureEditor::DrawChannelButtons()
    {
        // colored square buttons, active = fully opaque, inactive = dimmed
        struct ChannelButton
        {
            const char* m_label;
            bool*       m_active;
            Color       m_color;
        };

        ChannelButton channelButtons[] = {
            { "R", &m_showR, { 0.9f,  0.15f, 0.15f, 1.f } },
            { "G", &m_showG, { 0.15f, 0.75f, 0.15f, 1.f } },
            { "B", &m_showB, { 0.15f, 0.35f, 0.9f,  1.f } },
            { "A", &m_showA, { 0.65f, 0.65f, 0.65f, 1.f } },
        };

        for (const auto& button : channelButtons)
        {
            Color color = *button.m_active ? button.m_color :
                         Color{ button.m_color.m_r * 0.3f, button.m_color.m_g * 0.3f, button.m_color.m_b * 0.3f, 1.f };

            Color hoveredColor = { std::min(1.f, color.m_r * 1.3f),
                                   std::min(1.f, color.m_g * 1.3f),
                                   std::min(1.f, color.m_b * 1.3f), 1.f };

            m_gui.PushColor(StyleColor::Button, color);
            m_gui.PushColor(StyleColor::ButtonHovered, hoveredColor);
            m_gui.PushColor(StyleColor::ButtonActive, hoveredColor);

            if (m_gui.Button(button.m_label))
                *button.m_active = !(*button.m_active);  // toggle on click

            m_gui.PopColor(3);
            m_gui.SameLine(0.f, 4.f);
        }
    }

    void TextureEditor::DrawViewport()
    {
        LibMath::Vector2 panelPos = m_gui.GetPanelPos();
        LibMath::Vector2 panelSize = m_gui.GetAvailableSize();

        // Draw checkerboard across the entire viewport background
        DrawBackground(panelPos, LibMath::Vector2(panelPos[0] + panelSize[0], panelPos[1] + panelSize[1]));

        LibMath::Vector2 cursor = m_gui.GetCursorPos();
        m_gui.InvisibleButton("##viewport_interact", panelSize);
        bool hovered = m_gui.IsItemHovered();
        bool active = m_gui.IsItemActive();

        HandleViewportInput(panelPos, panelSize, hovered, active);

        if (m_textureID != 0)
        {
            DrawTiledImage(panelPos, panelSize, cursor);
            DrawImageBorder(panelPos, panelSize);
        }

        DrawZoomLabel(panelPos, panelSize);
        DrawHoverInfo(panelPos, panelSize);
    }

    void TextureEditor::HandleViewportInput(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize, bool hovered, bool active)
    {
        // Pan
        if (active && m_gui.IsMouseDown(MouseButton::Left))
        {
            LibMath::Vector2 delta = m_gui.GetMouseDelta();
            m_panOffset[0] += delta[0];
            m_panOffset[1] += delta[1];
        }
        // Zoom toward cursor
        if (hovered)
        {
            float wheel = m_gui.GetMouseWheelDelta();
            if (wheel != 0.f)
            {
                LibMath::Vector2 mouse = m_gui.GetMousePos();
                float cursorX = panelPos[0] + panelSize[0] * 0.5f;
                float cursorY = panelPos[1] + panelSize[1] * 0.5f;
                float preMouseX = mouse[0] - cursorX - m_panOffset[0];
                float preMouseY = mouse[1] - cursorY - m_panOffset[1];
                float oldZoom = m_zoom;
                m_zoom *= (wheel > 0.f ? 1.1f : 0.9f);
                m_zoom = std::max(0.05f, std::min(8.f, m_zoom));
                float scale = m_zoom / oldZoom;
                m_panOffset[0] -= preMouseX * (scale - 1.f);
                m_panOffset[1] -= preMouseY * (scale - 1.f);
            }

            HandleHoverPixelInfo(panelPos, panelSize);

            // Color pick: left-click when picker mode is active, or right-click fallback
            if (m_textureID != 0)
            {
                bool pickNow = false;
                if (m_pickerModeActive && m_gui.IsItemClicked(MouseButton::Left))
                    pickNow = true;
                if (pickNow)
                {
                    bool picked = HandleColorPick(ImageScreenOrigin(panelPos, panelSize));
                    if (picked)
                    {
                        m_pickerModeActive = false;
                        m_window.ResetCursorShape();
                        m_gui.SetManagesCursor(true);
                    }
                }
            }
            // Escape cancels picker mode
            if (m_pickerModeActive && m_gui.IsKeyPressed(UIKey::Escape))
            {
                m_pickerModeActive = false;
                m_window.ResetCursorShape();
                m_gui.SetManagesCursor(true);
            }
        }
    }

    void TextureEditor::HandleHoverPixelInfo(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize)
    {
        // Update hover pixel info every frame
        if (m_textureID != 0 && (!m_pixels.empty() || !m_pixelsHDR.empty()))
        {
            LibMath::Vector2 origin = ImageScreenOrigin(panelPos, panelSize);
            LibMath::Vector2 mouse = m_gui.GetMousePos();
            float displayWidth = static_cast<float>(m_width) * m_zoom;
            float displayHeight = static_cast<float>(m_height) * m_zoom;
            float imgR = origin[0] + displayWidth * m_tileRepeat;
            float imgB = origin[1] + displayHeight * m_tileRepeat;
            if (mouse[0] >= origin[0] && mouse[0] < imgR &&
                mouse[1] >= origin[1] && mouse[1] < imgB)
            {
                int px = static_cast<int>((mouse[0] - origin[0]) / displayWidth * m_width);
                int py = static_cast<int>((mouse[1] - origin[1]) / displayHeight * m_height);
                px = std::max(0, std::min(m_width - 1, px));
                py = std::max(0, std::min(m_height - 1, py));
                int idx = (py * m_width + px) * 4;
                m_hoverX = px;
                m_hoverY = py;
                if (!m_pixelsHDR.empty())
                    m_hoverColor = { m_pixelsHDR[idx],     m_pixelsHDR[idx + 1],
                                     m_pixelsHDR[idx + 2], m_pixelsHDR[idx + 3] };
                else
                    m_hoverColor = { m_pixels[idx] / 255.f,
                                     m_pixels[idx + 1] / 255.f,
                                     m_pixels[idx + 2] / 255.f,
                                     m_pixels[idx + 3] / 255.f };
                m_hoverValid = true;
            }
            else
            {
                m_hoverValid = false;
            }
        }
    }

    void TextureEditor::DrawTiledImage(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize, LibMath::Vector2 cursorStart)
    {
        float displayWidth = static_cast<float>(m_width) * m_zoom;
        float displayHeight = static_cast<float>(m_height) * m_zoom;

        Color tint = ChannelTint();
        tint.m_r *= m_exposure;
        tint.m_g *= m_exposure;
        tint.m_b *= m_exposure;

        for (int row = 0; row < m_tileRepeat; ++row)
        {
            for (int col = 0; col < m_tileRepeat; ++col)
            {
                LibMath::Vector2 origin = ImageScreenOrigin(panelPos, panelSize);
                float localX = origin[0] + col * displayWidth - panelPos[0] + cursorStart[0];
                float localY = origin[1] + row * displayHeight - panelPos[1] + cursorStart[1];
                m_gui.SetCursorPos(LibMath::Vector2(localX, localY));
                m_gui.DrawImageTinted(m_textureID, LibMath::Vector2(displayWidth, displayHeight), tint, true);
            }
        }
    }

    void TextureEditor::DrawImageBorder(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize)
    {
        IDrawList* drawList = m_gui.GetDrawList();
        LibMath::Vector2 origin = ImageScreenOrigin(panelPos, panelSize);
        float totalWidth = static_cast<float>(m_width) * m_zoom * m_tileRepeat;
        float totalHeight = static_cast<float>(m_height) * m_zoom * m_tileRepeat;
        drawList->DrawRect(
            LibMath::Vector2(origin[0] - 1.f, origin[1] - 1.f),
            LibMath::Vector2(origin[0] + totalWidth + 1.f, origin[1] + totalHeight + 1.f),
            0xFF555555, 0.f, 1.f);
    }

    void TextureEditor::DrawCheckerboard(LibMath::Vector2 min, LibMath::Vector2 max)
    {
        IDrawList* drawList = m_gui.GetDrawList();
        constexpr uint32_t colorA = 0xFF808080;  // mid grey
        constexpr uint32_t colorB = 0xFF606060;  // dark grey

        int cols = static_cast<int>((max[0] - min[0]) / CHECKER_TILE_SIZE) + 1;
        int rows = static_cast<int>((max[1] - min[1]) / CHECKER_TILE_SIZE) + 1;

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                float x0 = min[0] + col * CHECKER_TILE_SIZE;
                float y0 = min[1] + row * CHECKER_TILE_SIZE;
                float x1 = std::min(x0 + CHECKER_TILE_SIZE, max[0]);
                float y1 = std::min(y0 + CHECKER_TILE_SIZE, max[1]);

                uint32_t color = ((row + col) % 2 == 0) ? colorA : colorB;
                drawList->DrawRectFilled(LibMath::Vector2(x0, y0), LibMath::Vector2(x1, y1), color);
            }
        }
    }

    void TextureEditor::DrawBackground(LibMath::Vector2 min, LibMath::Vector2 max)
    {
        switch (m_background)
        {
        case TextureBackground::Checkerboard: 
            DrawCheckerboard(min, max); 
            break;
        case TextureBackground::Black: 
            m_gui.GetDrawList()->DrawRectFilled(min, max, 0xFF111111); 
            break;
        case TextureBackground::White: 
            m_gui.GetDrawList()->DrawRectFilled(min, max, 0xFFFFFFFF); 
            break;
        }
    }

    void TextureEditor::DrawDetails()
    {
        m_gui.PushStyleVariable(StyleVariable::ItemSpacing, 4.f, 6.f);

        DrawDetailsInfoStrip();
        m_gui.Separator();
        DrawDetailsFileSection();
        DrawDetailsCompressionSection();
        DrawDetailsLODSection();
        DrawDetailsViewSection();
        DrawDetailsResizeSection();
        DrawDetailsStatisticsSection();
        DrawDetailsColorSection();

        m_gui.PopStyleVariable();
    }

    void TextureEditor::DrawDetailsInfoStrip()
    {
        m_gui.PushFont(FontID::Large);
        m_gui.Text("Details");
        m_gui.PopFont();
        m_gui.Separator();

        int displayWidth = std::max(1, m_width >> m_mipLevel);
        int displayHeight = std::max(1, m_height >> m_mipLevel);

        m_gui.TextDisabled("Imported:   " + std::to_string(m_width) + "x" + std::to_string(m_height));
        m_gui.TextDisabled("Displayed:  " + std::to_string(displayWidth) + "x" + std::to_string(displayHeight));
        m_gui.TextDisabled("Format:     " + FormatString());
        m_gui.TextDisabled("Mip Levels: " + MipCountString());
        m_gui.TextDisabled("Disk Size:  " + FileSizeString());
    }

    void TextureEditor::DrawDetailsLODSection()
    {
        if (!m_gui.CollapsingHeader("Level of Detail", true)) return;

        m_gui.AlignTextToFramePadding();
        m_gui.Text("Mip:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);

        std::string items;
        for (int i = 0; i < m_mipCount; ++i)
            items += "Mip " + std::to_string(i) + '\0';
        items += '\0';

        m_gui.SetNextItemWidth(-1.f);
        if (m_gui.Combo("##det_mip", &m_mipLevel, items.c_str()))
        {
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, m_mipLevel);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_mipLevel);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void TextureEditor::DrawDetailsViewSection()
    {
        if (!m_gui.CollapsingHeader("View", true)) return;

        // Channels mask
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Channels:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        std::string mask;
        if (m_showR) mask += "R";
        if (m_showG) mask += "G";
        if (m_showB) mask += "B";
        if (m_showA) mask += "A";
        m_gui.TextDisabled(mask.empty() ? "None" : mask);

        // Zoom
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Zoom:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.SetNextItemWidth(-1.f);
        m_gui.SliderFloat("##det_zoom", &m_zoom, 0.05f, 8.f);

        // Exposure
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Exposure:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.SetNextItemWidth(-1.f);
        m_gui.SliderFloat("##det_ev", &m_exposure, 0.f, 4.f);

        // Tiling
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Tiling:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.SetNextItemWidth(-1.f);
        int tileIndex = m_tileRepeat - 1;
        if (m_gui.Combo("##det_tile", &tileIndex, "1x1\0" "2x2\0" "3x3\0\0"))
            m_tileRepeat = tileIndex + 1;
    }

    void TextureEditor::DrawDetailsFileSection()
    {
        if (!m_gui.CollapsingHeader("File", true)) return;

        TextWithTooltip("Name: ", m_filename);
        TextWithTooltip("Path: ", m_path.parent_path().string());
    }

    void TextureEditor::DrawDetailsColorSection()
    {
        if (!m_hasPickedColor) return;
        if (!m_gui.CollapsingHeader("Picked Color", true)) return;

        // Color swatch
        IDrawList* drawList = m_gui.GetDrawList();
        LibMath::Vector2 cursor = m_gui.GetCursorPos();
        LibMath::Vector2 swatchMin(cursor[0], cursor[1]);
        LibMath::Vector2 swatchMax(swatchMin[0] + DETAILS_WIDTH - 20.f, swatchMin[1] + 28.f);

        // Clamp to [0,1] for display � HDR values above 1 are shown as white
        auto clamp01 = [](float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
        uint32_t color32 =
            (uint32_t(clamp01(m_pickedColor.m_a) * 255.f) << 24) |
            (uint32_t(clamp01(m_pickedColor.m_b) * 255.f) << 16) |
            (uint32_t(clamp01(m_pickedColor.m_g) * 255.f) << 8) |
            uint32_t(clamp01(m_pickedColor.m_r) * 255.f);

        drawList->DrawRectFilled(swatchMin, swatchMax, color32, 4.f);
        drawList->DrawRect(swatchMin, swatchMax, 0xFF333333, 4.f, 1.f);
        m_gui.SetCursorPos(LibMath::Vector2(cursor[0], cursor[1] + 32.f));

        m_gui.Text("Position:  (" + std::to_string(m_pickedX) + ", " + std::to_string(m_pickedY) + ")");

        auto fmtFloat = [](float v) { char buf[16]; snprintf(buf, sizeof(buf), "%.4f", v); return std::string(buf); };

        if (IsHDR())
        {
            // HDR � show raw float values, may exceed 1.0
            m_gui.Text("R: " + fmtFloat(m_pickedColor.m_r) +
                "  G: " + fmtFloat(m_pickedColor.m_g) +
                "  B: " + fmtFloat(m_pickedColor.m_b) +
                "  A: " + fmtFloat(m_pickedColor.m_a));
            m_gui.TextDisabled("(HDR values may exceed 1.0)");
        }
        else
        {
            std::ostringstream hex;
            hex << "#" << std::uppercase << std::hex << std::setfill('0')
                << std::setw(2) << (int)(m_pickedColor.m_r * 255.f)
                << std::setw(2) << (int)(m_pickedColor.m_g * 255.f)
                << std::setw(2) << (int)(m_pickedColor.m_b * 255.f)
                << std::setw(2) << (int)(m_pickedColor.m_a * 255.f);
            m_gui.Text("Hex:  " + hex.str());

            auto format = [](float value) { return std::to_string(value).substr(0, 4); };
            m_gui.Text("RGBA: " + format(m_pickedColor.m_r) +
                "  " + format(m_pickedColor.m_g) +
                "  " + format(m_pickedColor.m_b) +
                "  " + format(m_pickedColor.m_a));
        }
    }

    void TextureEditor::DrawDetailsCompressionSection()
    {
        if (!m_gui.CollapsingHeader("Compression", true)) return;

        if (IsHDR())
        {
            m_gui.TextDisabled("BC compression is not available for HDR textures.");
            m_gui.TextDisabled("HDR data is stored as GL_RGBA16F (floating point).");
            return;
        }

        auto drawRow = [this](const char* label, auto contentFunc) 
            {
                m_gui.AlignTextToFramePadding();
                m_gui.Text(label);
                m_gui.SameLine(LABEL_COLUMN_WIDTH);
                contentFunc();
            };

        // 1. Format Picker
        drawRow("Format:", [this] 
            {
                m_gui.SetNextItemWidth(-1.f);
                int fmt = static_cast<int>(m_compression);
                if (m_gui.Combo("##comp_fmt", &fmt, "None\0BC1\0BC3\0BC4\0BC5\0\0")) 
                {
                    m_compression = static_cast<CompressionFormat>(fmt);
                    m_metaDirty = true;
                }
            });

        // 2. Info & Size Estimates
        static const char* hints[] = { "Uncompressed RGBA8", "~6:1 � diffuse", "~4:1 � alpha", "~2:1 � linear", "~2:1 � normal" };
        drawRow("Info:", [&] { m_gui.TextDisabled(hints[static_cast<int>(m_compression)]); });

        if (m_compression != CompressionFormat::None && m_width > 0) 
        {
            drawRow("Est. size:", [&] 
                {
                int bpb = (m_compression == CompressionFormat::BC1 || m_compression == CompressionFormat::BC4) ? 8 : 16;
                uintmax_t size = static_cast<uintmax_t>((m_width + 3) / 4) * ((m_height + 3) / 4) * bpb;
                m_gui.TextDisabled(FormatBytes(size).c_str());
                });
        }

        // 3. Disk State & Action
        CompressionFormat saved = static_cast<CompressionFormat>(m_meta.GetInt("compression", 0));
        drawRow("On disk:", [&] { m_gui.TextDisabled(CompressionFormatString(saved)); });

        bool needsSave = m_metaDirty || m_compression != saved;
        if (needsSave) m_gui.PushColor(StyleColor::Button, { 0.15f, 0.55f, 0.15f, 1.f });

        m_gui.Separator();
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        if (m_gui.Button("Apply & Save")) ApplyCompression();

        if (needsSave) m_gui.PopColor();
    }

    void TextureEditor::DrawDetailsStatisticsSection()
    {
        if (!m_statsValid) return;
        if (!m_gui.CollapsingHeader("Statistics")) return;

        const char* labels[4] = { "R:", "G:", "B:", "A:" };
        const Color  tints[4] = 
        {
            { 0.9f, 0.3f, 0.3f, 1.f },
            { 0.3f, 0.9f, 0.3f, 1.f },
            { 0.3f, 0.5f, 0.9f, 1.f },
            { 0.7f, 0.7f, 0.7f, 1.f }
        };

        auto format = [](float v) -> std::string 
            {
                char buf[8]; 
                snprintf(buf, sizeof(buf), "%.3f", v); 
                return buf;
            };

        for (int c = 0; c < 4; ++c)
        {
            const auto& stat = m_stats[c];
            m_gui.AlignTextToFramePadding();
            m_gui.TextColored(tints[c], labels[c]);
            m_gui.SameLine(100);
            m_gui.TextDisabled(
                "min " + format(stat.m_min) +
                "  max " + format(stat.m_max) +
                "  avg " + format(stat.m_avg));
        }
    }

    void TextureEditor::DrawDetailsResizeSection()
    {
        if (IsHDR()) return;
        if (m_pixels.empty()) return;
        if (!m_gui.CollapsingHeader("Resize")) return;
        // Width
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Width:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.SetNextItemWidth(-1.f);
        if (m_gui.SliderInt("##rw", &m_resizeWidth, 1, 8192))
        {
            if (m_resizeLockAspectRatio && m_width > 0)
                m_resizeHeight = std::max(1, static_cast<int>(m_resizeWidth * static_cast<float>(m_height) / m_width));
        }
        // Height
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Height:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.SetNextItemWidth(-1.f);
        if (m_gui.SliderInt("##rh", &m_resizeHeight, 1, 8192))
        {
            if (m_resizeLockAspectRatio && m_height > 0)
                m_resizeWidth = std::max(1, static_cast<int>(m_resizeHeight * static_cast<float>(m_width) / m_height));
        }
        // Lock aspect ratio toggle
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Lock AR:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.Checkbox("##lockAR", &m_resizeLockAspectRatio);
        // Show what the resize will do
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Result:");
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.TextDisabled(std::to_string(m_resizeWidth) + " x " + std::to_string(m_resizeHeight));

        m_gui.Separator();

        // Power-of-two shortcuts
        m_gui.AlignTextToFramePadding();
        m_gui.Text("Presets:");
        m_gui.SameLine(85);
        for (int preset : { 64, 128, 256, 512, 1024, 2048 })
        {
            if (m_gui.ButtonSized(std::to_string(preset).c_str(), LibMath::Vector2(58.f, 0.f)))
            {
                m_resizeWidth = preset;
                m_resizeHeight = m_resizeLockAspectRatio && m_width > 0
                    ? std::max(1, static_cast<int>(preset * static_cast<float>(m_height) / m_width))
                    : preset;
            }
            m_gui.SameLine(0.f, 2.f);
        }
        m_gui.Separator();
        bool changed = (m_resizeWidth != m_width || m_resizeHeight != m_height);
        if (changed)
        {
            m_gui.PushColor(StyleColor::Button,
                Color{ 0.15f, 0.55f, 0.15f, 1.f });
            m_gui.PushColor(StyleColor::ButtonHovered,
                Color{ 0.20f, 0.70f, 0.20f, 1.f });
        }
        
        if (m_gui.Button("Apply Resize"))
            ResizeTexture(m_resizeWidth, m_resizeHeight);
        if (changed)
        {
            m_gui.PopColor(2);
        }
        m_gui.SameLine(0.f, 8.f);
        if (m_gui.Button("Reset"))
        {
            m_resizeWidth = m_width;
            m_resizeHeight = m_height;
        }
    }

    void TextureEditor::LoadTexture()
    {
        m_filename = m_path.filename().string();
        m_fileBytes = Fs::exists(m_path) ? Fs::file_size(m_path) : 0;
        m_pixels.clear();
        m_pixelsHDR.clear();

        std::string ext = m_path.extension().string();
        for (auto& c : ext) c = static_cast<char>(::tolower(c));

        if (ext == ".hdr")
        {
            float* data = stbi_loadf(m_path.string().c_str(), &m_width, &m_height, &m_channels, 4);
            if (!data) return;
            m_pixelsHDR.assign(data, data + (m_width * m_height * 4));
            stbi_image_free(data);

            if (m_textureID == 0)
            {
                GLuint tex; glGenTextures(1, &tex);
                m_textureID = static_cast<uint32_t>(tex);
            }
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, m_pixelsHDR.data());
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            unsigned char* data = stbi_load(m_path.string().c_str(), &m_width, &m_height, &m_channels, 4);
            if (!data) return;
            m_pixels.assign(data, data + (m_width * m_height * 4));
            stbi_image_free(data);

            if (m_textureID == 0)
            {
                GLuint tex; glGenTextures(1, &tex);
                m_textureID = static_cast<uint32_t>(tex);
            }
            ApplyCompression();
        }

        // Reset resize targets to current dimensions
        m_resizeWidth = m_width;
        m_resizeHeight = m_height;

        ComputeStatistics();

        int maxDim = std::max(m_width, m_height);
        m_mipCount = 1;
        while (maxDim > 1) { maxDim /= 2; ++m_mipCount; }
    }

    void TextureEditor::UnloadTexture()
    {
        if (m_textureID != 0)
        {
            GLuint texture = m_textureID;
            glDeleteTextures(1, &texture);
            m_textureID = 0;
        }
        m_pixels.clear();
        m_pixelsHDR.clear();
        m_width = m_height = m_channels = m_mipCount = 0;
        m_fileBytes = 0;
        m_hasPickedColor = false;
        m_hoverValid = false;
        m_statsValid = false;
        m_resizeWidth = m_resizeHeight = 0;
    }

    void TextureEditor::ApplyCompression()
    {
        if (m_pixels.empty() || m_textureID == 0) return;

        struct FormatDesc { GLenum m_internalFormat; int m_bytesPerBlock; };
        static const FormatDesc formats[] = {
            { GL_RGBA8, 0 },                          // None
            { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8 },   // BC1
            { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 16 },  // BC3
            { GL_COMPRESSED_RED_RGTC1, 8 },           // BC4
            { GL_COMPRESSED_RG_RGTC2, 16 }            // BC5
        };
        const auto& target = formats[static_cast<int>(m_compression)];

        GLint currentGpuFormat;
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &currentGpuFormat);
        if (static_cast<GLenum>(currentGpuFormat) == target.m_internalFormat) 
        {
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }

        if (m_compression == CompressionFormat::None) 
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.data());
        }
        else 
        {
            int blocksWide = (m_width + 3) / 4;
            int blocksHigh = (m_height + 3) / 4;
            std::vector<unsigned char> outData(blocksWide * blocksHigh * target.m_bytesPerBlock);
            unsigned char* writePtr = outData.data();

            for (int by = 0; by < blocksHigh; ++by) 
            {
                for (int bx = 0; bx < blocksWide; ++bx) 
                {
                    unsigned char rgbaBlock[64] = {};
                    for (int row = 0; row < 4; ++row) 
                    {
                        int pixelY = std::min(by * 4 + row, m_height - 1);
                        int pixelX = std::min(bx * 4, m_width - 1);
                        memcpy(&rgbaBlock[row * 16], &m_pixels[(pixelY * m_width + pixelX) * 4], 16);
                    }

                    if (m_compression == CompressionFormat::BC1) stb_compress_dxt_block(writePtr, rgbaBlock, 0, 0);
                    else if (m_compression == CompressionFormat::BC3) stb_compress_dxt_block(writePtr, rgbaBlock, 1, 0);
                    else if (m_compression == CompressionFormat::BC4) 
                    {
                        unsigned char redOnly[16]; for (int i = 0; i < 16; ++i) redOnly[i] = rgbaBlock[i * 4];
                        stb_compress_bc4_block(writePtr, redOnly);
                    }
                    else 
                    {
                        unsigned char rgOnly[32]; 
                        for (int i = 0; i < 16; ++i) 
                        {
                            rgOnly[i * 2] = rgbaBlock[i * 4]; rgOnly[i * 2 + 1] = rgbaBlock[i * 4 + 1];
                        }
                        stb_compress_bc5_block(writePtr, rgOnly);
                    }
                    writePtr += target.m_bytesPerBlock;
                }
            }
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, target.m_internalFormat, m_width, m_height, 0, 
                                   (GLsizei)outData.size(), outData.data());
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        SaveMeta();
    }

    void TextureEditor::LoadOrCreateMeta()
    {
        m_meta.Load(m_path);  // no-op if file doesn't exist

        // Read saved compression setting, defaulting to None
        int saved = m_meta.GetInt("compression", static_cast<int>(CompressionFormat::None));
        m_compression = static_cast<CompressionFormat>(saved);
        m_metaDirty = false;
    }

    void TextureEditor::SaveMeta()
    {
        m_meta.Set("compression", static_cast<int>(m_compression));
        m_meta.Set("width", m_width);
        m_meta.Set("height", m_height);
        m_meta.Set("channels", m_channels);
        m_meta.Save(m_path);
        m_metaDirty = false;
    }

    void TextureEditor::SaveExport()
    {
        bool hasPixels = !m_pixels.empty() || !m_pixelsHDR.empty();
        if (!hasPixels) return;

        // Suggest the current filename as a default
        std::string suggested = m_path.stem().string()
            + (IsHDR() ? ".hdr" : ".png");
        Fs::path dest = m_window.SaveFileDialog(suggested);
        if (dest.empty()) return;

        if (!m_pixelsHDR.empty())
        {
            // HDR export � write as .hdr (RGBE)
            if (dest.extension().empty()) dest += ".hdr";
            stbi_write_hdr(dest.string().c_str(),
                m_width, m_height, 4, m_pixelsHDR.data());
        }
        else
        {
            // LDR export � prefer PNG, fallback to JPG
            if (dest.extension().empty()) dest += ".png";
            std::string destStr = dest.string();
            int success = stbi_write_png(destStr.c_str(),
                m_width, m_height, 4,
                m_pixels.data(), m_width * 4);
            if (!success)
            {
                dest.replace_extension(".jpg");
                success = stbi_write_jpg(dest.string().c_str(),
                    m_width, m_height, 4,
                    m_pixels.data(), 90);

                if (!success)
                {
                    LOG_WARNING_CAT("Resource", "Failed to save \"{}\"", dest.string());
                }
            }
        }
    }

    void TextureEditor::ResizeTexture(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0 || m_pixels.empty()) return;
        if (newWidth == m_width && newHeight == m_height) return;

        std::vector<unsigned char> out(newWidth * newHeight * 4);

        stbir_resize_uint8_linear(
            m_pixels.data(), m_width, m_height, 0,
            out.data(), newWidth, newHeight, 0,
            STBIR_RGBA);

        m_pixels = std::move(out);
        m_width = newWidth;
        m_height = newHeight;

        // Re-upload resized pixels
        if (m_textureID != 0)
        {
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.data());
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Recompute stats and mip count for the new dimensions
        ComputeStatistics();
        m_resizeWidth = m_width;
        m_resizeHeight = m_height;

        int maxDim = std::max(m_width, m_height);
        m_mipCount = 1;
        while (maxDim > 1) { maxDim /= 2; ++m_mipCount; }

        // Save new dimensions to meta
        m_meta.Set("width", m_width);
        m_meta.Set("height", m_height);
        m_meta.Save(m_path);
    }

    void TextureEditor::ComputeStatistics()
    {
        m_statsValid = false;
        if (m_pixels.empty()) return;

        for (int c = 0; c < 4; ++c)
        {
            m_stats[c].m_min = 1.f;
            m_stats[c].m_max = 0.f;
            m_stats[c].m_avg = 0.f;
        }

        int pixelCount = m_width * m_height;
        for (int i = 0; i < pixelCount; ++i)
        {
            for (int c = 0; c < 4; ++c)
            {
                float value = m_pixels[i * 4 + c] / 255.f;
                m_stats[c].m_min = std::min(m_stats[c].m_min, value);
                m_stats[c].m_max = std::max(m_stats[c].m_max, value);
                m_stats[c].m_avg += value;
            }
        }

        for (int c = 0; c < 4; ++c)
            m_stats[c].m_avg /= static_cast<float>(pixelCount);

        m_statsValid = true;
    }

    Color TextureEditor::ChannelTint() const
    {
        return 
        {
            m_showR ? 1.f : 0.f,
            m_showG ? 1.f : 0.f,
            m_showB ? 1.f : 0.f,
            m_showA ? 1.f : 0.f
        };
    }

    std::string TextureEditor::FormatString() const
    {
        switch (m_channels)
        {
        case 1:  return "R8";
        case 2:  return "RG8";
        case 3:  return "RGB8";
        case 4:  return "RGBA8";
        default: return "Unknown";
        }
    }

    std::string TextureEditor::MipCountString() const
    {
        return std::to_string(m_mipCount) + " level" + (m_mipCount != 1 ? "s" : "");
    }

    std::string TextureEditor::FileSizeString() const
    {
        return FormatBytes(m_fileBytes);
    }

    std::string TextureEditor::CompressionFormatString(CompressionFormat format) const
    {
        switch (format)
        {
        case CompressionFormat::None: return "None (RGBA8)";
        case CompressionFormat::BC1:  return "BC1 (DXT1)";
        case CompressionFormat::BC3:  return "BC3 (DXT5)";
        case CompressionFormat::BC4:  return "BC4 (RGTC1)";
        case CompressionFormat::BC5:  return "BC5 (RGTC2)";
        }
        return "Unknown";
    }

    std::string TextureEditor::FormatBytes(uintmax_t bytes) const
    {
        if (bytes >= 1048576) return std::to_string(bytes / 1048576) + " MB";
        if (bytes >= 1024)    return std::to_string(bytes / 1024) + " KB";
        return std::to_string(bytes) + " B";
    }

    std::string TextureEditor::TruncatePath(const std::string& str, int maxChars)
    {
        if (static_cast<int>(str.size()) <= maxChars)
            return str;

        // Keep the tail - the filename and nearest parent dirs are most useful
        std::string tail = str.substr(str.size() - (maxChars - 3));

        // Snap forward to the next path separator so we don't cut mid-component
        auto separator = tail.find_first_of("/\\");
        if (separator != std::string::npos && separator < tail.size() - 1)
            tail = tail.substr(separator);

        return "..." + tail;
    }

    void TextureEditor::TextWithTooltip(const std::string& label, const std::string& value, int maxChars)
    {
        std::string display = TruncatePath(value, maxChars);
        m_gui.Text(label);
        m_gui.SameLine(LABEL_COLUMN_WIDTH);
        m_gui.Text(display);

        // Only show tooltip when the text was actually truncated
        if (display != value && m_gui.IsItemHovered())
        {
            m_gui.BeginTooltip();
            m_gui.PushFont(FontID::Default);
            m_gui.Text(value);
            m_gui.PopFont();
            m_gui.EndTooltip();
        }
    }

    LibMath::Vector2 TextureEditor::ImageScreenOrigin(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize) const
    {
        float totalWidth = static_cast<float>(m_width) * m_zoom * m_tileRepeat;
        float totalHeight = static_cast<float>(m_height) * m_zoom * m_tileRepeat;
        float x = panelPos[0] + (panelSize[0] - totalWidth) * 0.5f + m_panOffset[0];
        float y = panelPos[1] + (panelSize[1] - totalHeight) * 0.5f + m_panOffset[1];
        return LibMath::Vector2(x, y);
    }

    bool TextureEditor::HandleColorPick(LibMath::Vector2 imageOrigin)
    {
        bool hasPixels = !m_pixels.empty() || !m_pixelsHDR.empty();
        if (!hasPixels) return false;

        LibMath::Vector2 mouse = m_gui.GetMousePos();
        float displayWidth = static_cast<float>(m_width) * m_zoom;
        float displayHeight = static_cast<float>(m_height) * m_zoom;

        float imgLeft = imageOrigin[0];
        float imgTop = imageOrigin[1];
        float imgRight = imgLeft + displayWidth * m_tileRepeat;
        float imgBottom = imgTop + displayHeight * m_tileRepeat;

        // Bail if the click is outside the image
        if (mouse[0] < imgLeft || mouse[0] >= imgRight ||
            mouse[1] < imgTop || mouse[1] >= imgBottom)
            return false;

        int pickedX = static_cast<int>((mouse[0] - imageOrigin[0]) / displayWidth * m_width);
        int pickedY = static_cast<int>((mouse[1] - imageOrigin[1]) / displayHeight * m_height);
        pickedY = (m_height - 1) - pickedY;
        pickedX = std::max(0, std::min(m_width - 1, pickedX));
        pickedY = std::max(0, std::min(m_height - 1, pickedY));

        int idx = (pickedY * m_width + pickedX) * 4;
        if (!m_pixelsHDR.empty())
        {
            if (static_cast<size_t>(idx + 3) >= m_pixelsHDR.size()) return false;
            m_pickedColor = { m_pixelsHDR[idx],     m_pixelsHDR[idx + 1],
                              m_pixelsHDR[idx + 2], m_pixelsHDR[idx + 3] };
        }
        else
        {
            if (static_cast<size_t>(idx + 3) >= m_pixels.size()) return false;
            m_pickedColor = { m_pixels[idx] / 255.f, m_pixels[idx + 1] / 255.f,
                              m_pixels[idx + 2] / 255.f, m_pixels[idx + 3] / 255.f };
        }
        m_pickedX = pickedX;
        m_pickedY = pickedY;
        m_hasPickedColor = true;
        // Clipboard: HDR values can exceed 1 so show as floats, LDR as hex
        if (!m_pixelsHDR.empty())
        {
            auto format = [](float v) 
                { 
                    char buffer[16]; 
                    snprintf(buffer, sizeof(buffer), "%.4f", v); 
                    return std::string(buffer); 
                };
            m_gui.SetClipboardText(
                format(m_pickedColor.m_r) + " " + format(m_pickedColor.m_g) + " " +
                format(m_pickedColor.m_b) + " " + format(m_pickedColor.m_a));
        }
        else
        {
            std::ostringstream hex;
            hex << "#" << std::uppercase << std::hex << std::setfill('0')
                << std::setw(2) << (int)(m_pickedColor.m_r * 255.f)
                << std::setw(2) << (int)(m_pickedColor.m_g * 255.f)
                << std::setw(2) << (int)(m_pickedColor.m_b * 255.f)
                << std::setw(2) << (int)(m_pickedColor.m_a * 255.f);
            m_gui.SetClipboardText(hex.str());
        }
        return true;
    }
    
    void TextureEditor::DrawZoomLabel(LibMath::Vector2 panelPos,
        LibMath::Vector2 panelSize)
    {
        IDrawList* drawList = m_gui.GetDrawList();
        std::string label = std::to_string(static_cast<int>(m_zoom * 100.f)) + "%";
        constexpr float FONT_SIZE = 22.f;
        constexpr float PAD = 6.f;
        LibMath::Vector2 size = drawList->CalculateTextSizeEx(label, FONT_SIZE);
        LibMath::Vector2 pos(panelPos[0] + 10.f, panelPos[1] + panelSize[1] - size[1] - 10.f);

        drawList->DrawRectFilled(
            LibMath::Vector2(pos[0] - PAD, pos[1] - PAD),
            LibMath::Vector2(pos[0] + size[0] + PAD, pos[1] + size[1] + PAD),
            0xBB000000, 4.f);

        drawList->DrawTextEx(pos, 0xFFFFFFFF, label, FONT_SIZE);
    }

    void TextureEditor::DrawHoverInfo(LibMath::Vector2 panelPos, LibMath::Vector2 panelSize)
    {
        if (!m_hoverValid) return;

        IDrawList* drawList = m_gui.GetDrawList();

        // Build the info string
        auto format = [](float v) -> std::string 
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "%.3f", v);
                return buf;
            };
        std::string info =
            "(" + std::to_string(m_hoverX) + ", " + std::to_string(m_hoverY) + ")  "
            + "R:" + format(m_hoverColor.m_r)
            + "  G:" + format(m_hoverColor.m_g)
            + "  B:" + format(m_hoverColor.m_b)
            + "  A:" + format(m_hoverColor.m_a);

        constexpr float FONT_SIZE = 16.f;
        constexpr float PAD = 5.f;
        LibMath::Vector2 size = drawList->CalculateTextSizeEx(info, FONT_SIZE);

        // Position: bottom-right of viewport
        float x = panelPos[0] + panelSize[0] - size[0] - PAD * 2.f - 8.f;
        float y = panelPos[1] + panelSize[1] - size[1] - PAD * 2.f - 8.f;

        // Small color swatch to the left of the text
        constexpr float SWATCH_SIZE =  14.f;
        float swatchX = x - SWATCH_SIZE - 6.f;
        float swatchY = y + (size[1] - SWATCH_SIZE) * 0.5f + PAD;

        // Background pill
        drawList->DrawRectFilled(
            LibMath::Vector2(swatchX - PAD, y),
            LibMath::Vector2(x + size[0] + PAD, y + size[1] + PAD * 2.f),
            0xCC000000, 4.f);

        // Color swatch
        uint32_t swCol =
            (uint32_t(m_hoverColor.m_a * 255.f) << 24) |
            (uint32_t(m_hoverColor.m_b * 255.f) << 16) |
            (uint32_t(m_hoverColor.m_g * 255.f) << 8) |
            uint32_t(m_hoverColor.m_r * 255.f);
        drawList->DrawRectFilled(
            LibMath::Vector2(swatchX, swatchY),
            LibMath::Vector2(swatchX + SWATCH_SIZE, swatchY + SWATCH_SIZE),
            swCol, 2.f);
        drawList->DrawRect(
            LibMath::Vector2(swatchX, swatchY),
            LibMath::Vector2(swatchX + SWATCH_SIZE, swatchY + SWATCH_SIZE),
            0xFF888888, 2.f, 1.f);

        // Text
        drawList->DrawTextEx(LibMath::Vector2(x, y + PAD), 0xFFFFFFFF, info, FONT_SIZE);
    }
}