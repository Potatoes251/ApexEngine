#include "TextureEditor.h"

#ifndef STB_IMAGE_STATIC
extern "C"
{
    extern unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
    extern void           stbi_image_free(void* retval_from_stbi_load);
}
#endif

#include <glad/glad.h>

#include <iomanip>

namespace Apex::Editor
{
    TextureEditor::TextureEditor(IGUI& gui) : m_gui(gui) {}
    TextureEditor::~TextureEditor() { UnloadTexture(); }

    void TextureEditor::Open(const fs::path& path, int instanceIndex)
    {
        UnloadTexture();
        LoadTexture(path);
        m_open = true;
        m_pendingFocus = false;
        m_instanceIndex = instanceIndex;
        m_zoom = 1.f;
        m_panOffset = { 0.f, 0.f };
        m_showR = true;
        m_showG = true;
        m_showB = true;
        m_showA = true;
        m_background = TextureBackground::Black;
        m_tileRepeat = 1;
        m_mipLevel = 0;
        m_exposure = 1.f;
        m_hasPickedColor = false;
    }

    void TextureEditor::Close()
    {
        UnloadTexture();
        m_open = false;
    }

    void TextureEditor::Focus()
    {
        m_pendingFocus = true;
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

        m_gui.BeginPanel("Texture Editor - " + m_filename);

        DrawToolbar();
        m_gui.Separator();

        // Left: viewport | Right: details — side by side
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

        // Compress placeholder 
        if (m_gui.Button("Compress")) { }
        m_gui.SameLine(0.f, 8.f);
        if (m_gui.Button("Reimport"))
        {
            fs::path p = m_path;
            UnloadTexture();
            LoadTexture(p);
            m_mipLevel = 0;
        }

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        // Colored channel buttons: R=red, G=green, B=blue, A=grey
        DrawChannelButtons();

        m_gui.SameLine(0.f, 16.f);

        // Background toggle
        const char* backgroundLabel = "CHECKBOARD";
        if (m_background == TextureBackground::Black) backgroundLabel = "BLACK";
        else if (m_background == TextureBackground::White) backgroundLabel = "WHITE";
        if (m_gui.ButtonSized(backgroundLabel, LibMath::Vector2(150.f, 0.f)))
        {
            switch (m_background)
            {
            case TextureBackground::Checkerboard: m_background = TextureBackground::Black; break;
            case TextureBackground::Black:        m_background = TextureBackground::White; break;
            case TextureBackground::White:        m_background = TextureBackground::Checkerboard; break;
            }
        }

        m_gui.SameLine(0.f, 8.f);

        // Tile repeat
        std::string tileLabel = std::to_string(m_tileRepeat) + "x";
        if (m_gui.Button(tileLabel.c_str()))
            m_tileRepeat = (m_tileRepeat % 3) + 1;

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        // Mip level — dropdown
        {
            // Build null-separated items string
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

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        if (m_gui.ButtonSized("+", LibMath::Vector2(28.f, 0.f))) m_zoom = std::min(8.f, m_zoom + 0.25f);
        m_gui.SameLine(0.f, 8.f);
        if (m_gui.ButtonSized("-", LibMath::Vector2(28.f, 0.f))) m_zoom = std::max(0.05f, m_zoom - 0.25f);
        m_gui.SameLine(0.f, 8.f);
        m_gui.Text("Zoom:");
        m_gui.SameLine(0.f, 8.f);

        // Zoom slider
        m_gui.SetNextItemWidth(120.f);
        m_gui.SliderFloat("##zoom", &m_zoom, 0.05f, 8.f);
        m_gui.SameLine(0.f, 8.f);

        // Zoom percentage label
        std::string zoomPercentage = std::to_string(static_cast<int>(m_zoom * 100.f)) + "%";
        m_gui.Text(zoomPercentage);

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        // Exposure
        m_gui.Text("EV:");
        m_gui.SameLine(0.f, 8.f);
        m_gui.SetNextItemWidth(80.f);
        m_gui.SliderFloat("##ev", &m_exposure, 0.f, 4.f);

        m_gui.SameLine(0.f, 16.f);
        m_gui.VerticalSeparator();
        m_gui.SameLine(0.f, 16.f);

        if (m_gui.Button("Fit")) { m_zoom = 1.f; m_panOffset = { 0.f, 0.f }; }
        m_gui.SameLine(0.f, 8.f);
        if (m_gui.Button("Close")) Close();

        m_gui.PopStyleVariable();
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

            m_gui.PopColor();
            m_gui.PopColor();
            m_gui.PopColor();
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

        // Pan - only while button is active (LMB held inside viewport)
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
                float cx = panelPos[0] + panelSize[0] * 0.5f;
                float cy = panelPos[1] + panelSize[1] * 0.5f;
                float preMouseX = mouse[0] - cx - m_panOffset[0];
                float preMouseY = mouse[1] - cy - m_panOffset[1];

                float oldZoom = m_zoom;
                m_zoom *= (wheel > 0.f ? 1.1f : 0.9f);
                m_zoom = std::max(0.05f, std::min(8.f, m_zoom));

                float scale = m_zoom / oldZoom;
                m_panOffset[0] -= preMouseX * (scale - 1.f);
                m_panOffset[1] -= preMouseY * (scale - 1.f);
            }
        }

        if (m_textureID == 0) return;

        // Color pick on right-click
        if (hovered && m_gui.IsMouseReleased(MouseButton::Right))
            HandleColorPick(ImageScreenOrigin(panelPos, panelSize));

        // Compute image display size
        float displayWidth = static_cast<float>(m_width) * m_zoom;
        float displayHeight = static_cast<float>(m_height) * m_zoom;

        // Apply exposure to tint
        Color tint = ChannelTint();
        tint.m_r *= m_exposure;
        tint.m_g *= m_exposure;
        tint.m_b *= m_exposure;

        // Center in panel, then apply pan offset
        float imageX = panelPos[0] + (panelSize[0] - displayWidth) * 0.5f + m_panOffset[0];
        float imageY = panelPos[1] + (panelSize[1] - displayHeight) * 0.5f + m_panOffset[1];

        // Draw tiled image
        for (int row = 0; row < m_tileRepeat; ++row)
        {
            for (int col = 0; col < m_tileRepeat; ++col)
            {
                LibMath::Vector2 origin = ImageScreenOrigin(panelPos, panelSize);
                float tileX = origin[0] + col * displayWidth;
                float tileY = origin[1] + row * displayHeight;
                float localX = tileX - panelPos[0] + cursor[0];
                float localY = tileY - panelPos[1] + cursor[1];
                m_gui.SetCursorPos(LibMath::Vector2(localX, localY));
                m_gui.DrawImageTinted(m_textureID, LibMath::Vector2(displayWidth, displayHeight), tint, false);
            }
        }

        // Border rect around image
        {
            IDrawList* drawList = m_gui.GetDrawList();
            LibMath::Vector2 origin = ImageScreenOrigin(panelPos, panelSize);
            float totalWidth = displayWidth * m_tileRepeat;
            float totalHeight = displayHeight * m_tileRepeat;
            drawList->DrawRect(
                LibMath::Vector2(origin[0] - 1.f, origin[1] - 1.f),
                LibMath::Vector2(origin[0] + totalWidth + 1.f,
                origin[1] + totalHeight + 1.f), 0xFF555555, 0.f, 1.f);
        }

        DrawZoomLabel(panelPos, panelSize);
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

        // Top info strip 
        m_gui.PushFont(FontID::Large);
        m_gui.Text("Details");
        m_gui.PopFont();
        m_gui.Separator();

        int displayWidth = m_width >> m_mipLevel;
        int displayHeight = m_height >> m_mipLevel;
        displayWidth = std::max(1, displayWidth);
        displayHeight = std::max(1, displayHeight);

        m_gui.TextDisabled("Imported:   " + std::to_string(m_width) + "x" + std::to_string(m_height));
        m_gui.TextDisabled("Displayed:  " + std::to_string(displayWidth) + "x" + std::to_string(displayHeight));
        m_gui.TextDisabled("Format:     " + FormatString());
        m_gui.TextDisabled("Mip Levels: " + MipCountString());
        m_gui.TextDisabled("Disk Size:  " + FileSizeString());

        m_gui.Separator();

        // File section 
        if (m_gui.CollapsingHeader("File", true))
        {
            m_gui.AlignTextToFramePadding();
            TextWithTooltip("Name: ", m_filename);

            m_gui.AlignTextToFramePadding();
            TextWithTooltip("Path: ", m_path.parent_path().string());
        }

        // Level of Detail section
        if (m_gui.CollapsingHeader("Level of Detail", true))
        {
            m_gui.AlignTextToFramePadding();
            m_gui.Text("Mip Preview:");
            m_gui.SameLine(LABEL_COLUMN_WIDTH);
            {
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
        }

        // View section
        if (m_gui.CollapsingHeader("View", true))
        {
            m_gui.AlignTextToFramePadding();
            m_gui.Text("Zoom:");
            m_gui.SameLine(LABEL_COLUMN_WIDTH);
            m_gui.SetNextItemWidth(-1.f);
            m_gui.SliderFloat("##det_zoom", &m_zoom, 0.05f, 8.f);

            m_gui.AlignTextToFramePadding();
            m_gui.Text("Exposure:");
            m_gui.SameLine(LABEL_COLUMN_WIDTH);
            m_gui.SetNextItemWidth(-1.f);
            m_gui.SliderFloat("##det_ev", &m_exposure, 0.f, 4.f);

            m_gui.AlignTextToFramePadding();
            m_gui.Text("Tiling:");
            m_gui.SameLine(LABEL_COLUMN_WIDTH);
            m_gui.SetNextItemWidth(-1.f);
            int tileIndex = m_tileRepeat - 1;  
            if (m_gui.Combo("##det_tile", &tileIndex, "1x1\0" "2x2\0" "3x3\0\0"))
                m_tileRepeat = tileIndex + 1; 
        }

        // Picked color section 
        if (m_hasPickedColor)
        {
            if (m_gui.CollapsingHeader("Picked Color", true))
            {
                // Color swatch
                IDrawList* drawList = m_gui.GetDrawList();
                LibMath::Vector2 cursorPos = m_gui.GetCursorPos();
                LibMath::Vector2 swatchMin(cursorPos[0], cursorPos[1]);
                LibMath::Vector2 swatchMax(swatchMin[0] + DETAILS_WIDTH - 20.f, swatchMin[1] + 28.f);

                uint32_t color32 =
                    (uint32_t(m_pickedColor.m_a * 255.f) << 24) |
                    (uint32_t(m_pickedColor.m_b * 255.f) << 16) |
                    (uint32_t(m_pickedColor.m_g * 255.f) << 8)  |
                     uint32_t(m_pickedColor.m_r * 255.f);

                drawList->DrawRectFilled(swatchMin, swatchMax, color32, 4.f);
                // 1px dark border so white swatches are visible
                drawList->DrawRect(swatchMin, swatchMax, 0xFF333333, 4.f, 1.f);
                m_gui.SetCursorPos(LibMath::Vector2(cursorPos[0], cursorPos[1] + 32.f));

                m_gui.Text("Position:  (" + std::to_string(m_pickedX) + ", " + std::to_string(m_pickedY) + ")");

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
                m_gui.TextDisabled("(right-click to pick)");
            }
        }

        m_gui.PopStyleVariable();
    }

    void TextureEditor::LoadTexture(const fs::path& path)
    {
        m_path = path;
        m_filename = path.filename().string();
        m_fileBytes = fs::exists(path) ? fs::file_size(path) : 0;

        unsigned char* data = stbi_load(path.string().c_str(), &m_width, &m_height, &m_channels, 4);
        if (!data) return;

        m_pixels.assign(data, data + (m_width * m_height * 4));

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
        m_textureID = static_cast<uint32_t>(texture);

        // Count mip levels
        int maxDimension = std::max(m_width, m_height);
        m_mipCount = 1;
        while (maxDimension > 1) { maxDimension /= 2; ++m_mipCount; }
    }

    void TextureEditor::UnloadTexture()
    {
        if (m_textureID != 0)
        {
            GLuint texture = m_textureID;
            glDeleteTextures(1, &texture);
            m_textureID = 0;
        }
        m_width = m_height = m_channels = m_mipCount = 0;
        m_fileBytes = 0;
        m_hasPickedColor = false;
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
        if (m_fileBytes >= 1024 * 1024)
            return std::to_string(m_fileBytes / (1024 * 1024)) + " MB";
        else if (m_fileBytes >= 1024)
            return std::to_string(m_fileBytes / 1024) + " KB";
        else
            return std::to_string(m_fileBytes) + " B";
    }

    std::string TextureEditor::TruncatePath(const std::string& str, int maxChars)
    {
        if (static_cast<int>(str.size()) <= maxChars)
            return str;

        // Keep the tail — the filename and nearest parent dirs are most useful
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

    void TextureEditor::HandleColorPick(LibMath::Vector2 imageOrigin)
    {
        if (m_pixels.empty()) return;
        LibMath::Vector2 mouse = m_gui.GetMousePos();
        float displayWidth = static_cast<float>(m_width) * m_zoom;
        float displayHeight = static_cast<float>(m_height) * m_zoom;

        // Total tiled image bounds in screen space
        float imgLeft = imageOrigin[0];
        float imgTop = imageOrigin[1];
        float imgRight = imgLeft + displayWidth * m_tileRepeat;
        float imgBottom = imgTop + displayHeight * m_tileRepeat;

        // Bail if the click is outside the image
        if (mouse[0] < imgLeft || mouse[0] >= imgRight ||
            mouse[1] < imgTop || mouse[1] >= imgBottom)
            return;

        int pickedX = static_cast<int>((mouse[0] - imageOrigin[0]) / displayWidth * m_width);
        int pickedY = static_cast<int>((mouse[1] - imageOrigin[1]) / displayHeight * m_height);
        pickedX = std::max(0, std::min(m_width - 1, pickedX));
        pickedY = std::max(0, std::min(m_height - 1, pickedY));

        int idx = (pickedY * m_width + pickedX) * 4;
        if (idx + 3 >= static_cast<int>(m_pixels.size())) return;

        uint8_t r = m_pixels[idx];
        uint8_t g = m_pixels[idx + 1];
        uint8_t b = m_pixels[idx + 2];
        uint8_t a = m_pixels[idx + 3];

        m_pickedColor = { r / 255.f, g / 255.f, b / 255.f, a / 255.f };
        m_pickedX = pickedX;
        m_pickedY = pickedY;
        m_hasPickedColor = true;

        std::ostringstream hex;
        hex << "#" << std::uppercase << std::hex << std::setfill('0')
            << std::setw(2) << (int)r << std::setw(2) << (int)g
            << std::setw(2) << (int)b << std::setw(2) << (int)a;
        m_gui.SetClipboardText(hex.str());
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
}