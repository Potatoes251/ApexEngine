#ifndef IDRAWLIST_H
#define IDRAWLIST_H

// ============================================================
// IDrawList.h - Backend-agnostic immediate draw interface.
// Used by editor panels to draw custom geometry inside ImGui
// windows without depending on ImGui directly.
// ============================================================

#include <string>
#include "LibMath/Vector/Vector2.h"

namespace Apex::UserInterface
{
    class IDrawList
    {
    public:
        virtual ~IDrawList() = default;

        virtual void DrawRectFilled(LibMath::Vector2 min, LibMath::Vector2 max, uint32_t color, float rounding = 0.f) = 0;
        virtual void DrawRect(LibMath::Vector2 min, LibMath::Vector2 max,
                              uint32_t color, float rounding = 0.f, float thickness = 1.f) = 0;
        virtual void DrawText(LibMath::Vector2 pos, uint32_t color, const std::string& text) = 0;
        // Same as DrawText but renders at an explicit pixel size
        virtual void DrawTextEx(LibMath::Vector2 pos, uint32_t color, const std::string& text, float fontSize) = 0;
        virtual LibMath::Vector2 CalculateTextSize(const std::string& text) = 0;
        virtual LibMath::Vector2 CalculateTextSizeEx(const std::string& text, float fontSize) = 0;
        virtual void AddLine(LibMath::Vector2 p1, LibMath::Vector2 p2, uint32_t color, float thickness = 1.f) = 0;
        virtual void DrawImage(uint32_t textureID,
            LibMath::Vector2 min, LibMath::Vector2 max,
            LibMath::Vector2 uv0 = { 0.f, 0.f },
            LibMath::Vector2 uv1 = { 1.f, 1.f },
            uint32_t tint = 0xFFFFFFFF) = 0;

        virtual void PushClipRect(LibMath::Vector2 min, LibMath::Vector2 max) = 0;
        virtual void PopClipRect() = 0;
    };

} // namespace Apex::UserInterface
#endif
