#pragma once
// ============================================================
// ImDrawListWrapper.h — INTERNAL.
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

        void DrawRectFilled(LibMath::Vector2 min, LibMath::Vector2 max,
                            uint32_t color, float rounding) override;
        void DrawText(LibMath::Vector2 pos, uint32_t color,
                            const std::string& text)        override;
        LibMath::Vector2 CalculateTextSize(const std::string& text) override;

        void Reset(ImDrawList* drawList) { m_drawList = drawList; }

    private:
        ImDrawList* m_drawList = nullptr;
    };

} // namespace Apex::UserInterface