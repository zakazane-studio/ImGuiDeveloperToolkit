#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "Containers/AnsiString.h"
#include "imgui.h"

namespace ImGuiDeveloperToolkit::PropertyInspector
{

namespace Private
{

void Inspect(const char* Label, const FProperty& Property, void* Outer);

void Inspect(const char* Label, const FIntProperty& IntProperty, void* Outer)
{
	ImGui::TableNextRow();

	ImGui::TableNextColumn();
	ImGui::Text("%s", Label);

	ImGui::TableNextColumn();

	int32* Ptr = IntProperty.ContainerPtrToValuePtr<int32>(Outer);
	int32 Value = IntProperty.GetPropertyValue(Ptr);
	ImGui::PushID(Label);
	if (ImGui::InputInt("", &Value))
	{
		IntProperty.SetPropertyValue(Ptr, Value);
		// make sure property changed gets propagated, maybe even transact?
	}
	ImGui::PopID();
}

void Inspect(const char* Label, const FArrayProperty& ArrayProperty, void* Outer)
{
	void* ArrayData = ArrayProperty.ContainerPtrToValuePtr<void>(Outer);

	if (ArrayData == nullptr || ArrayProperty.Inner == nullptr)
	{
		return;
	}

	FScriptArrayHelper ArrayHelper{&ArrayProperty, ArrayData};

	const int32 NumElements = ArrayHelper.Num();

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool bShowElements = ImGui::TreeNode(Label);
	ImGui::TableNextColumn();
	ImGui::Text("[%d]", NumElements);

	if (!bShowElements)
	{
		return;
	}

	ImGui::PushID(Label);

	for (int32 Index = 0; Index < NumElements; ++Index)
	{
		FAnsiStringBuilderBase LabelBuilder;
		LabelBuilder.Append("[").Append(FAnsiString::FromInt(Index)).Append("]");

		Inspect(LabelBuilder.ToString(), *ArrayProperty.Inner, ArrayHelper.GetRawPtr(Index));
	}

	ImGui::PopID();

	ImGui::TreePop();
}

void Inspect(const char* Label, const UStruct& Struct, void* Instance)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool bShowMembers = ImGui::TreeNode(Label);
	ImGui::TableNextColumn();

	const char* const TypeDisplayName =
		reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Struct.GetDisplayNameText().ToString()).Get());
	ImGui::Text("{%s}", TypeDisplayName);

	if (!bShowMembers)
	{
		return;
	}

	for (TFieldIterator<FProperty> It(&Struct); It; ++It)
	{
		const FProperty* const Property = *It;

		if (!Property)
		{
			continue;
		}

		const char* const PropertyName =
			reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Property->GetName()).Get());
		const char* const DisplayName =
			reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Property->GetDisplayNameText().ToString()).Get());

		ImGui::PushID(PropertyName);
		Inspect(DisplayName, *Property, Instance);
		ImGui::PopID();
	}

	ImGui::TreePop();
}

void Inspect(const char* Label, const FStructProperty& StructProperty, void* Outer)
{
	if (!IsValid(StructProperty.Struct))
	{
		return;
	}

	const char* const PropertyName =
		reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*StructProperty.GetName()).Get());
	const char* const DisplayName =
		reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*StructProperty.GetDisplayNameText().ToString()).Get());

	ImGui::PushID(PropertyName);
	Inspect(DisplayName, *StructProperty.Struct, StructProperty.ContainerPtrToValuePtr<void>(Outer));
	ImGui::PopID();
}

void Inspect(const char* Label, const FProperty& Property, void* Outer)
{
	if (const FArrayProperty* const ArrayProperty = CastField<FArrayProperty>(&Property))
	{
		Inspect(Label, *ArrayProperty, Outer);
	}
	if (const FIntProperty* const IntProperty = CastField<FIntProperty>(&Property))
	{
		Inspect(Label, *IntProperty, Outer);
	}
	if (const FStructProperty* const StructProperty = CastField<FStructProperty>(&Property))
	{
		Inspect(Label, *StructProperty, Outer);
	}
}

}  // namespace Private

void Inspect(const char* Label, const UStruct& Struct, void* Instance)
{
	if (Instance == nullptr)
	{
		return;
	}

	constexpr int TableFlags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable
							   | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
	if (ImGui::BeginTable("Struct", 2, TableFlags))
	{
		ImGui::TableSetupColumn("Index" /*, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide*/);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		Private::Inspect(Label, Struct, Instance);

		ImGui::EndTable();
	}
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
