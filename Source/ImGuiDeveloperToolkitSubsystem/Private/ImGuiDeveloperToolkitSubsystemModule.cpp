// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImGuiDeveloperToolkitSubsystemModule.h"

#define LOCTEXT_NAMESPACE "FImGuiDeveloperToolkitSubsystemModule"

namespace ImGuiDeveloperToolkit
{

void FImGuiDeveloperToolkitSubsystemModule::StartupModule()
{
}

void FImGuiDeveloperToolkitSubsystemModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImGuiDeveloperToolkitSubsystemModule, ImGuiDeveloperToolkitSubsystem)

}  // namespace ImGuiDeveloperToolkit
