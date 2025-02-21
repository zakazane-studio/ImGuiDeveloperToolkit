// Copyright ZAKAZANE Studio. All Rights Reserved.

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitTool.h"

bool UImGuiDeveloperToolkitTool::ShouldCreateTool() const
{
	return true;
}

void UImGuiDeveloperToolkitTool::Show()
{
	bShow = true;
}

void UImGuiDeveloperToolkitTool::Tick(float DeltaTime)
{
	DoTick(DeltaTime, bShow);
}
