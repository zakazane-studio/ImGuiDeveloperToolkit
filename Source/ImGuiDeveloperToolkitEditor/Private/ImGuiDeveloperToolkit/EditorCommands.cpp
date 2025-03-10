#include "EditorCommands.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "EditorCommands"

namespace ImGuiDeveloperToolkit
{

namespace Private
{

class FEditorCommands final : public TCommands<FEditorCommands>
{
public:
	FEditorCommands()
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

void FEditorCommands::RegisterCommands()
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

TSharedRef<FUICommandList> RegisterEditorCommands()
{
	Private::FEditorCommands::Register();
	TSharedRef<FUICommandList> CommandList = MakeShared<FUICommandList>();

	const Private::FEditorCommands& Commands = Private::FEditorCommands::Get();
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
			[](FMenuBuilder& MenuBuilder)
			{ MenuBuilder.AddMenuEntry(Private::FEditorCommands::Get().ShowDeveloperToolkitCommand); }));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	return CommandList;
}

void UnregisterEditorCommands()
{
	Private::FEditorCommands::Unregister();
}

}  // namespace ImGuiDeveloperToolkit

#undef LOCTEXT_NAMESPACE
