#ifndef IMDRAWLISTWRAPPER_H
#define IMDRAWLISTWRAPPER_H

// ============================================================
// ImDrawListWrapper.h - INTERNAL.
// Concrete IDrawList backed by ImDrawList.
// ============================================================

#include "IDrawList.h"

struct ImDrawList;

namespace Apex::UserInterface
{
    class ImDrawListWrapper final : public IDrawList 
    {
    public:
        explicit ImDrawListWrapper(ImDrawList* drawList = nullptr) : m_drawList(drawList) {}

        void DrawRectFilled(LibMath::Vector2 min, LibMath::Vector2 max, uint32_t color, float rounding) override;
        void DrawRect(LibMath::Vector2 min, LibMath::Vector2 max,
                      uint32_t color, float rounding = 0.f, float thickness = 1.f) override;
        void DrawText(LibMath::Vector2 pos, uint32_t color, const std::string& text) override;
        void DrawTextEx(LibMath::Vector2 pos, uint32_t color, const std::string& text, float fontSize) override;
        LibMath::Vector2 CalculateTextSize(const std::string& text) override;
        LibMath::Vector2 CalculateTextSizeEx(const std::string& text, float fontSize) override;

        void AddLine(LibMath::Vector2 p1, LibMath::Vector2 p2, uint32_t color, float thickness) override;

        void DrawImage(uint32_t textureID,
            LibMath::Vector2 min, LibMath::Vector2 max,
            LibMath::Vector2 uv0 = { 0.f, 0.f },
            LibMath::Vector2 uv1 = { 1.f, 1.f },
            uint32_t tint = 0xFFFFFFFF) override;

        void PushClipRect(LibMath::Vector2 min, LibMath::Vector2 max) override;
        void PopClipRect() override;

        void Reset(ImDrawList* drawList) { m_drawList = drawList; }

    private:
        ImDrawList* m_drawList = nullptr;
    };

} // namespace Apex::UserInterface
#endif