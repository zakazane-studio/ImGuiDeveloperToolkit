﻿// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkitConfiguration.generated.h"

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
struct ImFont;

USTRUCT()
struct IMGUIDEVELOPERTOOLKIT_API FImGuiDeveloperToolkitFont
{
	GENERATED_BODY();

	UPROPERTY(VisibleAnywhere)
	FUtf8String Name;

	UPROPERTY(VisibleAnywhere)
	int32 Size = -1;

	ImFont* Font = nullptr;
};

USTRUCT()
struct IMGUIDEVELOPERTOOLKIT_API FImGuiDeveloperToolkitConfiguration
{
	GENERATED_BODY()

	UPROPERTY(Transient, VisibleAnywhere)
	bool bShow = false;

	void Initialize();

	void Tick(float DeltaTime);

	ImFont* GetFont() const;

private:
	UPROPERTY(Transient, VisibleAnywhere)
	TMap<FUtf8String, FUtf8String> AvailableFontPathsByName;

	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitFont SelectedFont;

	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitFont DefaultFont;

	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitFont PreviousFont;

	FDelegateHandle SetSelectedFontDelegateHandle = {};

#if WITH_ENGINE
	TStrongObjectPtr<UTexture2D> FontAtlasTexturePtr = nullptr;
#else
	TSharedPtr<FSlateBrush> FontAtlasTexturePtr = nullptr;
#endif

	float ShowResetFontPopup_S = -1.f;

	void TickFontSelector(float DeltaTime);

	void TickResetFontPopup(float DeltaTime);

	TFuture<bool> LoadFonts();
};
