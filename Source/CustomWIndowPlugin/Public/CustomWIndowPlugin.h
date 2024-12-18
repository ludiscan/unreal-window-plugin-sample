// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SList.h"
#include "Interfaces/IHttpRequest.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FCustomWIndowPluginModule : public IModuleInterface, public TSharedFromThis<FCustomWIndowPluginModule>
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** タブ生成のためのコールバック */
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	/** プラグインボタンクリックの処理 */
	void PluginButtonClicked();

private:
	/** メニューの登録 */
	void RegisterMenus();

	/** ホスト名入力のコミット処理 */
	void OnHostNameCommitted(const FText& InText, ETextCommit::Type InCommitType);
	FReply OnSessionLoadClicked();

	/** Submitボタンが押された際のリクエスト送信 */
	FReply OnSubmitClicked();

	/** HTTPリクエストのレスポンス処理 */
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnShowDialog(FString Message);

private:
	/** コマンドリスト */
	TSharedPtr<class FUICommandList> PluginCommands = nullptr;

	/** APIホスト名 */
	FString ApiHostName = "http://localhost:3000";

	/** リスト表示用のウィジェット */
	TSharedPtr<SListView<TSharedPtr<FString>>> StringListWidget = nullptr;
	TSharedPtr<SList> ListWidget;


};
