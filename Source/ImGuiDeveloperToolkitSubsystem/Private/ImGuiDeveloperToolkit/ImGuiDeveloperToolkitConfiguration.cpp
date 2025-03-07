#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitConfiguration.h"

#include "ImGuiContext.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitWindow.h"
#include "ImageUtils.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace ImGuiDeveloperToolkitConfigurationPrivate
{

struct FConfigurationData
{
	FUtf8String FontName = {};
	int32 FontSize = -1;
};

FConfigurationData* GConfigurationData = nullptr;

void* ConfigurationHandler_ReadOpen(ImGuiContext* Ctx, ImGuiSettingsHandler* Handler, const char* Name)
{
	return reinterpret_cast<void*>(1);
}

void ConfigurationHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* Entry, const char* Line)
{
	using namespace ImGuiDeveloperToolkit;

	if (GConfigurationData == nullptr)
	{
		return;
	}

	const FUtf8StringView LineView{Line};

	int32 EqIdx = -1;
	if (!LineView.FindChar(UTF8CHAR{'='}, EqIdx))
	{
		return;
	}

	const FUtf8StringView Key = LineView.Left(EqIdx).TrimStartAndEnd();
	const FUtf8StringView Value = LineView.RightChop(EqIdx + 1).TrimStartAndEnd();

	if (Key == FUtf8StringView{"FontName"})
	{
		GConfigurationData->FontName = Value;
	}

	if (Key == FUtf8StringView{"FontSize"})
	{
		const FUtf8String ValueStr{Value};
		GConfigurationData->FontSize = FCStringUtf8::Atoi(*ValueStr);
	}
}

static void ConfigurationHandler_ApplyAll(ImGuiContext* Ctx, ImGuiSettingsHandler*)
{
	if (GConfigurationData == nullptr || !IsValid(GEngine))
	{
		return;
	}

	UImGuiDeveloperToolkitSubsystem* Subsystem = GEngine->GetEngineSubsystem<UImGuiDeveloperToolkitSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}

	Subsystem->Configuration.SetFont(GConfigurationData->FontName, GConfigurationData->FontSize);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
static void ConfigurationHandler_WriteAll(ImGuiContext* Ctx, ImGuiSettingsHandler* Handler, ImGuiTextBuffer* Buf)
{
	if (GConfigurationData == nullptr || GConfigurationData->FontName.IsEmpty() && GConfigurationData->FontSize <= 0)
	{
		return;
	}

	Buf->appendf("[ImGuiDeveloperToolkitConfiguration][Configuration]\n");
	Buf->appendf("FontName=%s\n", *GConfigurationData->FontName);
	Buf->appendf("FontSize=%d\n", GConfigurationData->FontSize);
}

static void ContextHook_Shutdown(ImGuiContext* Ctx, ImGuiContextHook* Hook)
{
	if (GConfigurationData == nullptr)
	{
		return;
	}

	delete GConfigurationData;
	GConfigurationData = nullptr;
}

}  // namespace ImGuiDeveloperToolkitConfigurationPrivate

void FImGuiDeveloperToolkitConfiguration::Initialize()
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	const FString EngineFontsDir = FPaths::EngineContentDir() / TEXT("Slate/Fonts");
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> AvailableFontsStr;
	FileManager.FindFilesRecursive(AvailableFontsStr, *EngineFontsDir, TEXT("*.ttf"), true, false, false);
	Algo::Transform(
		AvailableFontsStr,
		AvailableFontPathsByName,
		[](const FString& FontPathStr)
		{
			FUtf8String Name{StringCast<UTF8CHAR>(*FPaths::GetBaseFilename(FontPathStr))};
			FUtf8String Path{StringCast<UTF8CHAR>(*FontPathStr)};
			return MakeTuple(MoveTemp(Name), MoveTemp(Path));
		});

	DefaultFont.Name = FUtf8String{FPaths::GetBaseFilename(FImGuiContext::GetDefaultFontPath())};
	DefaultFont.Size = FImGuiContext::DefaultFontSize;

	if (SelectedFont.Name.IsEmpty())
	{
		SelectedFont = DefaultFont;
	}

	FImGuiContext::GetOnPostCreateContext().AddLambda(
		[](ImGuiContext* Context)
		{
			if (Context == nullptr)
			{
				return;
			}

			GConfigurationData = new FConfigurationData;

			// Add .ini handler for stored configuration data
			ImGuiSettingsHandler IniHandler;
			IniHandler.TypeName = "ImGuiDeveloperToolkitConfiguration";
			IniHandler.TypeHash = ImHashStr("ImGuiDeveloperToolkitConfiguration");
			IniHandler.ReadOpenFn = ConfigurationHandler_ReadOpen;
			IniHandler.ReadLineFn = ConfigurationHandler_ReadLine;
			IniHandler.ApplyAllFn = ConfigurationHandler_ApplyAll;
			IniHandler.WriteAllFn = ConfigurationHandler_WriteAll;
			ImGui::AddSettingsHandler(&IniHandler);

			// Add context hook for imgui shutdown
			ImGuiContextHook Hook;
			Hook.Type = ImGuiContextHookType_Shutdown;
			Hook.Callback = &ContextHook_Shutdown;
			ImGui::AddContextHook(Context, &Hook);
		});
}

void FImGuiDeveloperToolkitConfiguration::Tick(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkit;

	if (bShow)
	{
		ON_SCOPE_EXIT
		{
			ImGui::End();
		};

		SetNextWindowPosAndSizeWithinMainViewport(ImVec2{.7f, .2f}, ImVec2{.25f, .25f}, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Configuration", &bShow))
		{
			ImGui::ShowStyleSelector("Theme");
			TickFontSelector(DeltaTime);
		}
	}
}

void FImGuiDeveloperToolkitConfiguration::SetFont(const FUtf8String& Name, const int32 Size)
{
	SelectedFont.Name = Name;
	SelectedFont.Size = Size;
	LoadFonts();
}

ImFont* FImGuiDeveloperToolkitConfiguration::GetFont() const
{
	return SelectedFont.Font;
}

void FImGuiDeveloperToolkitConfiguration::TickFontSelector(const float DeltaTime)
{
	const bool bFontChanged = [this]
	{
		bool bResult = false;

		if (!AvailableFontPathsByName.IsEmpty())
		{
			if (ImGui::BeginCombo(
					"Font",
					SelectedFont.Name.IsEmpty() ? "Select font" : reinterpret_cast<const char*>(*SelectedFont.Name)))
			{
				for (const auto& [Name, Path] : AvailableFontPathsByName)
				{
					if (ImGui::Selectable(reinterpret_cast<const char*>(*Name), SelectedFont.Name == Name))
					{
						SelectedFont.Name = Name;
						bResult = true;
					}
				}

				ImGui::EndCombo();
			}
		}

		return bResult;
	}();

	const bool bSizeChanged = ImGui::SliderInt("Font size", &SelectedFont.Size, 8, 32);

	if (bFontChanged || bSizeChanged)
	{
		LoadFonts().Then(
			[this, bFontChanged](TFuture<bool>&& SuccessFuture)
			{
				if (!SuccessFuture.Get() || !bFontChanged)
				{
					return;
				}

				ShowResetFontPopup_S = 5;
			});
	}

	TickResetFontPopup(DeltaTime);
}

void FImGuiDeveloperToolkitConfiguration::TickResetFontPopup(const float DeltaTime)
{
	if (ShowResetFontPopup_S > 0.f)
	{
		ImGui::OpenPopup("Keep font?");

		ImGui::PushFont(DefaultFont.Font);

		if (ImGui::BeginPopupModal("Keep font?"))
		{
			ImGui::Text("Example text:");

			ImGui::NewLine();

			ImGui::PushFont(SelectedFont.Font);
			ImGui::Text("The quick brown fox jumps over the lazy dog");
			ImGui::PopFont();

			ImGui::NewLine();

			ImGui::Text("Keep selected font?");

			bool bClosePopup = false;

			if (ImGui::Button("Yes"))
			{
				bClosePopup = true;
			}

			ImGui::SameLine();

			FAnsiStringBuilderBase NoBuilder;
			NoBuilder.Appendf("No (%d)", FMath::FloorToInt32(ShowResetFontPopup_S));
			ShowResetFontPopup_S -= DeltaTime;

			if (ImGui::Button(NoBuilder.ToString()) || ShowResetFontPopup_S <= 0.f)
			{
				SelectedFont = DefaultFont;
				bClosePopup = true;
			}

			if (bClosePopup)
			{
				ShowResetFontPopup_S = -1.f;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopFont();
	}
}

TFuture<bool> FImGuiDeveloperToolkitConfiguration::LoadFonts()
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	const TSharedPtr<TPromise<bool>> Promise = MakeShared<TPromise<bool>>();

	const TSharedPtr<FImGuiContext> ImGuiContext = FImGuiContext::Get(ImGui::GetCurrentContext());
	if (!ImGuiContext.IsValid())
	{
		Promise->SetValue(false);
		return Promise->GetFuture();
	}

	if (GConfigurationData != nullptr)
	{
		GConfigurationData->FontName = SelectedFont.Name;
		GConfigurationData->FontSize = SelectedFont.Size;
	}

	SetSelectedFontDelegateHandle = ImGuiContext->OnPreFrame.AddLambda(
		[this, ImGuiContext, Promise]
		{
			const ImGuiIO& IO = ImGui::GetIO();

			if (IO.Fonts == nullptr)
			{
				Promise->SetValue(false);
				return;
			}

			const FUtf8String* FontPath = AvailableFontPathsByName.Find(SelectedFont.Name);
			const FUtf8String DefaultFontPath{StringCast<UTF8CHAR>(*FImGuiContext::GetDefaultFontPath())};

			IO.Fonts->Clear();

			DefaultFont.Font = IO.Fonts->AddFontFromFileTTF(
				reinterpret_cast<const char*>(*DefaultFontPath), FImGuiContext::DefaultFontSize);
			if (DefaultFont.Font == nullptr)
			{
				DefaultFont.Font = IO.Fonts->AddFontDefault();
			}

			if (FontPath != nullptr)
			{
				SelectedFont.Font =
					IO.Fonts->AddFontFromFileTTF(reinterpret_cast<const char*>(**FontPath), SelectedFont.Size);
			}
			else
			{
				SelectedFont.Font = nullptr;
			}

			uint8* TextureData;
			int32 TextureWidth, TextureHeight, BytesPerPixel;
			IO.Fonts->GetTexDataAsRGBA32(&TextureData, &TextureWidth, &TextureHeight, &BytesPerPixel);

#if WITH_ENGINE
			const FImageView TextureView(TextureData, TextureWidth, TextureHeight, ERawImageFormat::BGRA8);
			FontAtlasTexturePtr.Reset(FImageUtils::CreateTexture2DFromImage(TextureView));
#else
			FontAtlasTexturePtr = FSlateDynamicImageBrush::CreateWithImageData(
				TEXT("ImGuiFontAtlas"),
				FVector2D(TextureWidth, TextureHeight),
				TArray(TextureDataRaw, TextureWidth * TextureHeight * BytesPerPixel));
#endif

			IO.Fonts->SetTexID(FontAtlasTexturePtr.Get());

			Promise->SetValue(SelectedFont.Font != nullptr);

			ImGuiContext->OnPreFrame.Remove(SetSelectedFontDelegateHandle);
			SetSelectedFontDelegateHandle.Reset();
		});

	return Promise->GetFuture();
}
