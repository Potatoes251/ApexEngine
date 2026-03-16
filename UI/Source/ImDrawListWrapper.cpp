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

	void ImDrawListWrapper::DrawRect(LibMath::Vector2 min, LibMath::Vector2 max,
									 uint32_t color, float rounding, float thickness)
	{
		if (m_drawList)
			m_drawList->AddRect(ImVec2(min[0], min[1]),
				ImVec2(max[0], max[1]), color, rounding,
				0, thickness);
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

	void ImDrawListWrapper::DrawTextEx(LibMath::Vector2 pos, uint32_t color,
		const std::string& text, float fontSize)
	{
		if (m_drawList)
			m_drawList->AddText(nullptr, fontSize, ImVec2(pos[0], pos[1]), color, text.c_str());
	}

	LibMath::Vector2 ImDrawListWrapper::CalculateTextSize(const std::string& text)
	{
		ImVec2 size = ImGui::CalcTextSize(text.c_str());
		return LibMath::Vector2(size.x, size.y);
	}

	LibMath::Vector2 ImDrawListWrapper::CalculateTextSizeEx(const std::string& text, float fontSize)
	{
		// Scale the default text size by the ratio of requested vs default font size
		ImFont* font = ImGui::GetFont();
		float   defaultSize = font ? font->FontSize : 13.f;
		float   scale = (defaultSize > 0.f) ? (fontSize / defaultSize) : 1.f;
		ImVec2  size = ImGui::CalcTextSize(text.c_str());
		return LibMath::Vector2(size.x * scale, size.y * scale);
	}

	void ImDrawListWrapper::AddLine(LibMath::Vector2 p1, LibMath::Vector2 p2, uint32_t color, float thickness)
	{
		m_drawList->AddLine(
			ImVec2(p1[0], p1[1]),
			ImVec2(p2[0], p2[1]),
			color,
			thickness
		);
	}
} // namespace Apex::UserInterface