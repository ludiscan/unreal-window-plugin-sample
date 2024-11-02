// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomWIndowPlugin.h"
#include "CustomWIndowPluginStyle.h"
#include "CustomWIndowPluginCommands.h"
#include "HttpModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Interfaces/IHttpResponse.h"

static const FName CustomWIndowPluginTabName("CustomWIndowPlugin");

#define LOCTEXT_NAMESPACE "FCustomWIndowPluginModule"

void FCustomWIndowPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	StringList.Add(MakeShareable(new FString("Hello")));
	FCustomWIndowPluginStyle::Initialize();
	FCustomWIndowPluginStyle::ReloadTextures();

	FCustomWIndowPluginCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCustomWIndowPluginCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FCustomWIndowPluginModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCustomWIndowPluginModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(CustomWIndowPluginTabName, FOnSpawnTab::CreateRaw(this, &FCustomWIndowPluginModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FCustomWIndowPluginTabTitle", "CustomWIndowPlugin"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FCustomWIndowPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FCustomWIndowPluginStyle::Shutdown();

	FCustomWIndowPluginCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CustomWIndowPluginTabName);
}

TSharedRef<SDockTab> FCustomWIndowPluginModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				// APIホスト名の入力欄
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("HostNameLabel", "API Host:"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(3)
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(ApiHostName))
					.OnTextCommitted_Raw(this, &FCustomWIndowPluginModule::OnHostNameCommitted)
					.OnTextChanged_Raw(this, &FCustomWIndowPluginModule::OnHostNameCommitted, ETextCommit::Default)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1)
			.Padding(10)
			[
				// スクロール可能なリスト
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew( ListWidget, SList )
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				// リクエストを送るボタン
				SNew(SButton)
				.Text(LOCTEXT("RequestButton", "Submit"))
				.OnClicked_Raw(this, &FCustomWIndowPluginModule::OnSubmitClicked)
			]
		];
}

void FCustomWIndowPluginModule::OnHostNameCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	ApiHostName = InText.ToString();
	UE_LOG(LogTemp, Warning, TEXT("API Host set to: %s"), *ApiHostName);
}

FReply FCustomWIndowPluginModule::OnSubmitClicked()
{
	// HTTPリクエストを送信
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &FCustomWIndowPluginModule::OnResponseReceived);
	Request->SetURL(ApiHostName + "/your-endpoint"); // 適宜エンドポイントに合わせてください
	Request->SetVerb("GET");
	Request->ProcessRequest();
	return FReply::Handled();
}

void FCustomWIndowPluginModule::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		FString ResponseString = Response->GetContentAsString();
        
		// レスポンスをパースしてリストに追加
		StringList.Empty();
		TArray<FString> ParsedStrings;
		ResponseString.ParseIntoArray(ParsedStrings, TEXT(","), true); // カンマ区切りと仮定
        
		for (const FString& Item : ParsedStrings)
		{
			StringList.Add(MakeShareable(new FString(Item)));
		}

		if (StringListWidget.IsValid())
		{
			StringListWidget->RequestListRefresh();
		}
	}
}

TSharedRef<ITableRow> FCustomWIndowPluginModule::GenerateStringListRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return // リスト要素（行）を作成
        SNew( STableRow< TSharedPtr<FString> >, OwnerTable )
        .Padding( 2.f )
        [
            // ラベルとして要素を追加
            SNew( STextBlock ).Text( FText::FromString( *InItem.Get() ) )
        ];
}

void FCustomWIndowPluginModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(CustomWIndowPluginTabName);
}

void FCustomWIndowPluginModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCustomWIndowPluginCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCustomWIndowPluginCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomWIndowPluginModule, CustomWIndowPlugin)