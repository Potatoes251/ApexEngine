#pragma once
// ============================================================
// IDrawList.h — Backend-agnostic immediate draw interface.
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

        virtual void DrawRectFilled(LibMath::Vector2 min, LibMath::Vector2 max,
            uint32_t color, float rounding = 0.f) = 0;
        virtual void DrawText(LibMath::Vector2 pos, uint32_t color,
            const std::string& text) = 0;
        virtual LibMath::Vector2 CalculateTextSize(const std::string& text) = 0;
    };

} // namespace Apex::UserInterface
