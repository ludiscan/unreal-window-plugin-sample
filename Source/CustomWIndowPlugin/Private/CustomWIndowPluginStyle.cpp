// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomWIndowPluginStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FCustomWIndowPluginStyle::StyleInstance = nullptr;

void FCustomWIndowPluginStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FCustomWIndowPluginStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FCustomWIndowPluginStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("CustomWIndowPluginStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FCustomWIndowPluginStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("CustomWIndowPluginStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("CustomWIndowPlugin")->GetBaseDir() / TEXT("Resources"));

	Style->Set("CustomWIndowPlugin.OpenPluginWindowIcon", new IMAGE_BRUSH_SVG(TEXT("my-icon"), Icon20x20));

	return Style;
}

void FCustomWIndowPluginStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FCustomWIndowPluginStyle::Get()
{
	return *StyleInstance;
}
