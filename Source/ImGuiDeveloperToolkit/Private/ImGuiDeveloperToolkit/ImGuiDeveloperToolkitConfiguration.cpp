#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitConfiguration.h"

#include "ImGuiContext.h"
#include "ImageUtils.h"
#include "imgui.h"

void FImGuiDeveloperToolkitConfiguration::Initialize()
{
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
}

void FImGuiDeveloperToolkitConfiguration::Tick(float DeltaTime)
{
	if (bShow)
	{
		ON_SCOPE_EXIT
		{
			ImGui::End();
		};

		if (ImGui::Begin("Configuration", &bShow))
		{
			ImGui::ShowStyleSelector("Theme");
			TickFontSelector(DeltaTime);
		}
	}
}

ImFont* FImGuiDeveloperToolkitConfiguration::GetFont() const
{
	return SelectedFont.Font;
}

void FImGuiDeveloperToolkitConfiguration::TickFontSelector(float DeltaTime)
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
				if (bFontChanged && SuccessFuture.Get())
				{
					ShowResetFontPopup_S = 5;
				}
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

			ImGui::PushFont(SelectedFont.Font);
			ImGui::Text("The quick brown fox jumps over the lazy dog");
			ImGui::PopFont();

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
	const TSharedPtr<TPromise<bool>> Promise = MakeShared<TPromise<bool>>();

	const TSharedPtr<FImGuiContext> ImGuiContext = FImGuiContext::Get(ImGui::GetCurrentContext());
	if (!ImGuiContext.IsValid())
	{
		Promise->SetValue(false);
		return Promise->GetFuture();
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
