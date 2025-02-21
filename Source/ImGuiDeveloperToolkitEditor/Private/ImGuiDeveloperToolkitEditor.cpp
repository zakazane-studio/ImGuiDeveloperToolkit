#include "ImGuiDeveloperToolkitEditor.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FImGuiDeveloperToolkitEditorModule"

namespace ImGuiDeveloperToolkit
{

namespace Private
{

class FDeveloperToolkitEditorCommands final : public TCommands<FDeveloperToolkitEditorCommands>
{
public:
	FDeveloperToolkitEditorCommands()
		: TCommands{
			  TEXT("DeveloperToolkitEditorCommands"),
			  FText::FromString("DeveloperToolkitEditorCommands"),
			  NAME_None,
			  FAppStyle::GetAppStyleSetName()}
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> ShowDeveloperToolkitCommand = nullptr;
};

void FDeveloperToolkitEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		ShowDeveloperToolkitCommand,
		"Show Developer Toolkit",
		"Show the Developer Toolkit window",
		EUserInterfaceActionType::ToggleButton,
		FInputChord{EKeys::Backslash});
}

UImGuiDeveloperToolkitSubsystem* GetEditorImGuiSubsystem()
{
	if (!IsValid(GEngine))
	{
		return nullptr;
	}

	return GEngine->GetEngineSubsystem<UImGuiDeveloperToolkitSubsystem>();
}

}  // namespace Private

TSharedRef<FUICommandList> RegisterImGuiEditorCommands()
{
	Private::FDeveloperToolkitEditorCommands::Register();
	TSharedRef<FUICommandList> CommandList = MakeShared<FUICommandList>();

	const Private::FDeveloperToolkitEditorCommands& Commands = Private::FDeveloperToolkitEditorCommands::Get();
	CommandList->MapAction(
		Commands.ShowDeveloperToolkitCommand,
		FExecuteAction::CreateLambda(
			[]
			{
				UImGuiDeveloperToolkitSubsystem* const ImGuiSubsystem = Private::GetEditorImGuiSubsystem();
				if (!IsValid(ImGuiSubsystem))
				{
					return;
				}

				ImGuiSubsystem->ToggleShow();
			}),
		FCanExecuteAction{},
		FIsActionChecked::CreateLambda(
			[]
			{
				const UImGuiDeveloperToolkitSubsystem* const ImGuiSubsystem = Private::GetEditorImGuiSubsystem();
				if (!IsValid(ImGuiSubsystem))
				{
					return false;
				}

				return ImGuiSubsystem->IsShow();
			}));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedRef<FExtender> MenuExtender = MakeShared<FExtender>();
	MenuExtender->AddMenuExtension(
		"WindowLayout",
		EExtensionHook::After,
		CommandList,
		FMenuExtensionDelegate::CreateLambda(
			[](FMenuBuilder& MenuBuilder) {
				MenuBuilder.AddMenuEntry(Private::FDeveloperToolkitEditorCommands::Get().ShowDeveloperToolkitCommand);
			}));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	return CommandList;
}

void UnregisterImGuiEditorCommands()
{
	Private::FDeveloperToolkitEditorCommands::Unregister();
}

}  // namespace ImGuiDeveloperToolkit

void FImGuiDeveloperToolkitEditorModule::StartupModule()
{
	using namespace ImGuiDeveloperToolkit;

	RegisterImGuiEditorCommands();
}

void FImGuiDeveloperToolkitEditorModule::ShutdownModule()
{
	using namespace ImGuiDeveloperToolkit;

	UnregisterImGuiEditorCommands();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImGuiDeveloperToolkitEditorModule, ImGuiDeveloperToolkitEditor)
