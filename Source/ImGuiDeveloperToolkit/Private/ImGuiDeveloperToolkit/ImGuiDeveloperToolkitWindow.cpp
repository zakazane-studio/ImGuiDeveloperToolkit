#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitWindow.h"

namespace ImGuiDeveloperToolkit
{

void SetNextWindowPosAndSizeWithinMainViewport(const ImVec2& PosFactor, const ImVec2& SizeFactor, const ImGuiCond Cond)
{
	if (const ImGuiViewport* const MainViewport = ImGui::GetMainViewport())
	{
		ImGui::SetNextWindowPos(
			ImVec2{
				MainViewport->WorkPos.x + MainViewport->WorkSize.x * PosFactor.x,
				MainViewport->WorkPos.y + MainViewport->WorkSize.y * PosFactor.y},
			Cond);
		ImGui::SetNextWindowSize(
			ImVec2{MainViewport->WorkSize.x * SizeFactor.x, MainViewport->WorkSize.y * SizeFactor.y}, Cond);
	}
}

void SetWindowPosAndSizeWithinMainViewport(const ImVec2& PosFactor, const ImVec2& SizeFactor, const ImGuiCond Cond)
{
	if (const ImGuiViewport* const MainViewport = ImGui::GetMainViewport())
	{
		ImGui::SetWindowPos(
			ImVec2{
				MainViewport->WorkPos.x + MainViewport->WorkSize.x * PosFactor.x,
				MainViewport->WorkPos.y + MainViewport->WorkSize.y * PosFactor.y},
			Cond);
		ImGui::SetWindowPos(
			ImVec2{MainViewport->WorkSize.x * SizeFactor.x, MainViewport->WorkSize.y * SizeFactor.y}, Cond);
	}
}

}  // namespace ImGuiDeveloperToolkit
