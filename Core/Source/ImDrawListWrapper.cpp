#include "ImDrawListWrapper.h"
#include "imgui.h"

namespace Apex::UserInterface
{
	void ImDrawListWrapper::DrawRectFilled(LibMath::Vector2 min, LibMath::Vector2 max,
										 uint32_t color, float rounding)
	{
		if (m_drawList)
		{
			m_drawList->AddRectFilled(
				ImVec2(min[0], min[1]),
				ImVec2(max[0], max[1]),
				color,
				rounding
			);
		}
	}
	void ImDrawListWrapper::DrawText(LibMath::Vector2 pos, uint32_t color,
									const std::string& text)
	{
		if (m_drawList)
		{
			m_drawList->AddText(
				ImVec2(pos[0], pos[1]),
				color,
				text.c_str()
			);
		}
	}
	LibMath::Vector2 ImDrawListWrapper::CalculateTextSize(const std::string& text)
	{
		if (m_drawList)
		{
			ImVec2 size = ImGui::CalcTextSize(text.c_str());
			return LibMath::Vector2(size.x, size.y);
		}
		return LibMath::Vector2(0.f, 0.f);
	}
} // namespace Apex::UserInterface