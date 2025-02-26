// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "imgui.h"

namespace ImGuiDeveloperToolkit
{

/**
 * Sets position and size of the next window to be opened with regard to the main viewport.
 * 
 * @param PosFactor Position of the top-left corner of the window as [0, 1] coordinates within the main viewport work
 *	area.
 * @param SizeFactor Size of the window as [0, 1] factor of the main viewport's work area size.
 * @param Cond Determines whether this call is effective. See ImGuiCond_ flags.
 */
IMGUIDEVELOPERTOOLKITSUBSYSTEM_API void SetNextWindowPosAndSizeWithinMainViewport(
	const ImVec2& PosFactor, const ImVec2& SizeFactor, ImGuiCond Cond = 0);

/**
 * Sets position and size of the current window with regard to the main viewport.
 * 
 * @param PosFactor Position of the top-left corner of the window as [0, 1] coordinates within the main viewport work
 *	area.
 * @param SizeFactor Size of the window as [0, 1] factor of the main viewport's work area size.
 * @param Cond Determines whether this call is effective. See ImGuiCond_ flags.
 */
IMGUIDEVELOPERTOOLKITSUBSYSTEM_API void SetWindowPosAndSizeWithinMainViewport(
	const ImVec2& PosFactor, const ImVec2& SizeFactor, ImGuiCond Cond = 0);

}  // namespace ImGuiDeveloperToolkit
