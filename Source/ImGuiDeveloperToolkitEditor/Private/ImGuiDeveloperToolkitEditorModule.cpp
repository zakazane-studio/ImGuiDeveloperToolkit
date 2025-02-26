#include "ImGuiDeveloperToolkitEditorModule.h"

#include "ImGuiDeveloperToolkit/EditorCommands.h"

void FImGuiDeveloperToolkitEditorModule::StartupModule()
{
	using namespace ImGuiDeveloperToolkit;

	RegisterEditorCommands();
}

void FImGuiDeveloperToolkitEditorModule::ShutdownModule()
{
	using namespace ImGuiDeveloperToolkit;

	UnregisterEditorCommands();
}

IMPLEMENT_MODULE(FImGuiDeveloperToolkitEditorModule, ImGuiDeveloperToolkitEditor)
