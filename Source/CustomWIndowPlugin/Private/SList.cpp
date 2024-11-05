#include "SList.h"

/**
 * コンストラクタ
 */
void SList::Construct( const FArguments& Args )
{
	ChildSlot
	[
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal) // 横スクロールを有効にする
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				// ヘッダー行
				SNew(SHeaderRow)
				+ SHeaderRow::Column("SessionId")
				.DefaultLabel(FText::FromString("Session ID"))
				.HAlignHeader(HAlign_Left)

				+ SHeaderRow::Column("ProjectId")
				.DefaultLabel(FText::FromString("Project ID"))
				.HAlignHeader(HAlign_Left)

				+ SHeaderRow::Column("Name")
				.DefaultLabel(FText::FromString("Name"))
				.HAlignHeader(HAlign_Left)

				+ SHeaderRow::Column("startTime")
				.DefaultLabel(FText::FromString("Start Time"))
				.HAlignHeader(HAlign_Left)

				+ SHeaderRow::Column("DeviceId")
				.DefaultLabel(FText::FromString("Device ID"))
				.HAlignHeader(HAlign_Left)

				+ SHeaderRow::Column("Platform")
				.DefaultLabel(FText::FromString("Platform"))
				.HAlignHeader(HAlign_Left)
			]

			+ SVerticalBox::Slot()
			[
				// リストビューを追加
				SAssignNew(ListViewWidget, SListView<TSharedPtr<FString>>)
				.ItemHeight(24)
				.ListItemsSource(&Items)
				.OnGenerateRow(this, &SList::OnGenerateRowForList)
				.SelectionMode(ESelectionMode::Type::SingleToggle) // 単一選択モード
				.OnSelectionChanged_Raw(this, &SList::OnSelectionChanged)
			]
		]
	];
}

/**
 * リスト要素が生成されるときの処理
 */
TSharedRef<ITableRow> SList::OnGenerateRowForList( TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable )
{
	FPlaySessionResponseDto* playSession = PlaySessionData.FindByPredicate([Item](const TSharedPtr<FPlaySessionResponseDto>& dto) {
		return FString::FromInt(dto->SessionId) == *Item;
	})->Get();
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(5.f, 2.f)) // マージンを設定して見た目を改善
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(playSession->SessionId))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock).Text(FText::AsNumber(playSession->ProjectId))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(playSession->Name))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(playSession->StartTime))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(playSession->DeviceId))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(playSession->Platform))
			]
		];
}

/**
 * 要素追加処理
 */
void SList::AddItem( const FPlaySessionResponseDto& item )
{
	PlaySessionData.Add(MakeShareable(new FPlaySessionResponseDto(item)));
	// 新規追加 sessionID
	Items.Add(MakeShareable(new FString(FString::FromInt(item.SessionId))));
}

/**
 * リストクリア処理
 */
void SList::ClearList()
{
	// 要素配列クリア
	Items.Empty();
	PlaySessionData.Empty();
}

void SList::RequestListRefresh()
{
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
}
