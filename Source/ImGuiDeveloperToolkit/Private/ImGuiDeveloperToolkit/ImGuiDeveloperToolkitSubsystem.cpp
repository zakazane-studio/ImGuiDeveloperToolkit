// Copyright ZAKAZANE Studio. All Rights Reserved.

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitTool.h"

#include <imgui.h>

namespace ImGuiDeveloperToolkitSubsystemPrivate
{

bool IsWithinCurrentContext(const UImGuiDeveloperToolkitTool& Tool)
{
	const bool bInEditor =
#if WITH_EDITOR
		GIsEditor && IsValid(GEditor) && (GEditor->PlayWorld == nullptr);
#else
		false;
#endif

	const EImGuiDeveloperToolkitToolContext ToolContext = Tool.GetContext();

	return ToolContext == EImGuiDeveloperToolkitToolContext::EditorAndGame
		   || (ToolContext == EImGuiDeveloperToolkitToolContext::EditorOnly && bInEditor)
		   || (ToolContext == EImGuiDeveloperToolkitToolContext::GameOnly && !bInEditor);
}

}  // namespace ImGuiDeveloperToolkitSubsystemPrivate

void UImGuiDeveloperToolkitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PopulateTools();
	Configuration.Initialize();
}

void UImGuiDeveloperToolkitSubsystem::ToggleShow()
{
	bShow = !bShow;
}

bool UImGuiDeveloperToolkitSubsystem::IsShow() const
{
	return bShow;
}

void UImGuiDeveloperToolkitSubsystem::Tick(float DeltaTime)
{
	using namespace ImGuiDeveloperToolkitSubsystemPrivate;

	ImFont* const Font = Configuration.GetFont();
	if (Font != nullptr)
	{
		ImGui::PushFont(Font);
	}

	if (const ImGui::FScopedContext ScopedContext; ScopedContext)
	{
		TickMainWindow(DeltaTime);
		TickConfigurationWindow(DeltaTime);
		TickTools(DeltaTime);
	}

	if (Font != nullptr)
	{
		ImGui::PopFont();
	}
}

TStatId UImGuiDeveloperToolkitSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UImGuiImGuiSubsystem, STATGROUP_Tickables);
}

UWorld* UImGuiDeveloperToolkitSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UImGuiDeveloperToolkitSubsystem::IsTickableInEditor() const
{
	return true;
}

bool UImGuiDeveloperToolkitSubsystem::IsTickableWhenPaused() const
{
	return true;
}

void UImGuiDeveloperToolkitSubsystem::PopulateTools()
{
	if (!ensure(IsInGameThread()))
	{
		return;
	}

	const UClass* ToolSuperclass = UImGuiDeveloperToolkitTool::StaticClass();
	if (!ensure(IsValid(ToolSuperclass)))
	{
		return;
	}

	TArray<UClass*> ToolClasses;
	GetDerivedClasses(ToolSuperclass, ToolClasses);
	for (UClass* ToolClass : ToolClasses)
	{
		if (!IsValid(ToolClass) || ToolClass->HasAllClassFlags(CLASS_Abstract))
		{
			continue;
		}

		const UImGuiDeveloperToolkitTool* DefaultTool = GetDefault<UImGuiDeveloperToolkitTool>(ToolClass);
		if (!IsValid(DefaultTool) || !DefaultTool->ShouldCreateTool())
		{
			continue;
		}

		UImGuiDeveloperToolkitTool* Tool = NewObject<UImGuiDeveloperToolkitTool>(this, ToolClass);
		if (!IsValid(Tool))
		{
			continue;
		}

		const bool bAlreadyContains = Tools.ContainsByPredicate(
			[Name = Tool->GetToolName()](const UImGuiDeveloperToolkitTool* ContainedTool)
			{
				if (!IsValid(ContainedTool))
				{
					return false;
				}

				return ContainedTool->GetToolName() == Name;
			});

		if (bAlreadyContains)
		{
			continue;
		}

		Tools.Emplace(MoveTemp(Tool));
	}
}

void UImGuiDeveloperToolkitSubsystem::TickMainWindow(float DeltaTime)
{
	if (bShow)
	{
		ON_SCOPE_EXIT
		{
			ImGui::End();
		};

		if (ImGui::Begin("Developer Toolkit", &bShow, ImGuiWindowFlags_MenuBar))
		{
			TickMainMenu(DeltaTime);
			TickToolList(DeltaTime);
		}
	}
}

void UImGuiDeveloperToolkitSubsystem::TickConfigurationWindow(float DeltaTime)
{
	Configuration.Tick(DeltaTime);
}

void UImGuiDeveloperToolkitSubsystem::TickMainMenu(float DeltaTime)
{
	if (ImGui::BeginMenuBar())
	{
		Configuration.bShow = ImGui::MenuItem("Configuration") || Configuration.bShow;
		ImGui::EndMenuBar();
	}
}

void UImGuiDeveloperToolkitSubsystem::TickToolList(float DeltaTime)
{
	using namespace ImGuiDeveloperToolkitSubsystemPrivate;

	ImGui::InputText("Find tool...", ToolNameFilter.GetData(), ToolNameFilterSize);
	const FAnsiStringView FindToolTextView{ToolNameFilter.GetData()};

	for (UImGuiDeveloperToolkitTool* Tool : Tools)
	{
		if (!IsValid(Tool) || !IsWithinCurrentContext(*Tool))
		{
			continue;
		}

		const FAnsiString ToolName = Tool->GetToolName();
		if (FindToolTextView.IsEmpty() || ToolName.Contains(FindToolTextView))
		{
			if (ImGui::Button(*ToolName))
			{
				Tool->Show();
			}
		}
	}
}

void UImGuiDeveloperToolkitSubsystem::TickTools(float DeltaTime)
{
	using namespace ImGuiDeveloperToolkitSubsystemPrivate;

	for (UImGuiDeveloperToolkitTool* Tool : Tools)
	{
		if (!IsValid(Tool) || !IsWithinCurrentContext(*Tool))
		{
			continue;
		}

		Tool->Tick(DeltaTime);
	}
}
