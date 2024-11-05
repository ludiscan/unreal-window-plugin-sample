// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomWIndowPlugin.h"
#include "CustomWIndowPluginStyle.h"
#include "CustomWIndowPluginCommands.h"
#include "FPlaySessionResponse.h"
#include "HttpModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Interfaces/IHttpResponse.h"

static const FName CustomWIndowPluginTabName("CustomWIndowPlugin");

#define LOCTEXT_NAMESPACE "FCustomWIndowPluginModule"

void FCustomWIndowPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
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
		.Icon(FCustomWIndowPluginStyle::Get().GetBrush("CustomWIndowPlugin.OpenPluginWindow"))
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
				+ SHorizontalBox::Slot()
				.Padding(10, 0, 0, 0)
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("SessionLoadButton", "Load"))
					.OnClicked_Raw(this, &FCustomWIndowPluginModule::OnSessionLoadClicked)
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
			.Padding(0, 40)
			.AutoHeight()
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

FReply FCustomWIndowPluginModule::OnSessionLoadClicked()
{
	// HTTPリクエストを送信
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &FCustomWIndowPluginModule::OnResponseReceived);
	Request->SetURL(ApiHostName + "/api/v0/projects/1/play_session"); // 適宜エンドポイントに合わせてください
	Request->SetVerb("GET");
	Request->ProcessRequest();
	return FReply::Handled();
}

FReply FCustomWIndowPluginModule::OnSubmitClicked()
{
	auto selected = ListWidget->GetSelectedItem();
	if (!selected.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("No item selected."));
		return FReply::Handled();
	}
	UE_LOG(LogTemp, Warning, TEXT("Selected: %s"), *selected->Name);
	return FReply::Handled();
}

void FCustomWIndowPluginModule::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Response received!"));
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HTTPリクエストが失敗しました。"));
		return;
	}

	// レスポンスボディを取得
	FString ResponseContent = Response->GetContentAsString();

	// JSON文字列を解析して構造体に変換
	TArray<FPlaySessionResponseDto> PlaySessionData;
	if (FPlaySessionResponseDto::ParseArrayFromJson(ResponseContent, PlaySessionData))
	{
		UE_LOG(LogTemp, Warning, TEXT("JSONのパースに成功しました。"));
		// メインスレッドでの処理を実行
		AsyncTask(ENamedThreads::GameThread, [this, PlaySessionData]()
		{
			if (ListWidget.IsValid())
			{
				ListWidget->ClearList();
				for (const FPlaySessionResponseDto& Item : PlaySessionData)
				{
					// パースに成功した場合の処理
					UE_LOG(LogTemp, Log, TEXT("セッションID: %d"), Item.SessionId);
					UE_LOG(LogTemp, Log, TEXT("プロジェクトID: %d"), Item.ProjectId);
					UE_LOG(LogTemp, Log, TEXT("名前: %s"), *Item.Name);
					UE_LOG(LogTemp, Log, TEXT("デバイスID: %s"), *Item.DeviceId);
					UE_LOG(LogTemp, Log, TEXT("プラットフォーム: %s"), *Item.Platform);
					UE_LOG(LogTemp, Log, TEXT("アプリバージョン: %s"), *Item.AppVersion);
					UE_LOG(LogTemp, Log, TEXT("開始時間: %s"), *Item.StartTime);
					UE_LOG(LogTemp, Log, TEXT("終了時間: %s"), *Item.EndTime);
					UE_LOG(LogTemp, Log, TEXT("再生中: %s"), Item.bIsPlaying ? TEXT("true") : TEXT("false"));
					ListWidget->AddItem(Item);
				}
				ListWidget->RequestListRefresh();
			}
		});
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("JSONのパースに失敗しました。"));
	}
	// if (bWasSuccessful && Response.IsValid())
	// {
	// 	FString ResponseString = Response->GetContent()
 //        
	// 	// レスポンスをパースしてリストに追加
	// 	StringList.Empty();
	// 	TArray<FString> ParsedStrings;
	// 	ResponseString.ParseIntoArray(ParsedStrings, TEXT(","), true); // カンマ区切りと仮定
 //        
	// 	for (const FString& Item : ParsedStrings)
	// 	{
	// 		StringList.Add(MakeShareable(new FString(Item)));
	// 	}
	//
	// 	if (StringListWidget.IsValid())
	// 	{
	// 		StringListWidget->RequestListRefresh();
	// 	}
	// }
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