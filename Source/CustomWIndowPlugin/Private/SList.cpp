#include "SList.h"

/**
 * コンストラクタ
 */
void SList::Construct( const FArguments& Args )
{
	FName ColumnId;
	ChildSlot
	[
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal) // 横スクロールを有効にする
		+ SScrollBox::Slot()
		.AutoSize()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Filter:"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(4)
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(this, &SList::OnFilterTextChanged)
					.HintText(FText::FromString("Enter filter text..."))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(10, 0, 0, 0)
				[
					SNew(SButton)
					.Text(FText::FromString("Filter by Device ID"))
					.OnClicked_Raw(this, &SList::OnFilterByDeviceIdClicked)
				]
			]

			+ SVerticalBox::Slot()
			[
				// リストビューを追加
				SAssignNew(ListViewWidget, SListView<TSharedPtr<FString>>)
				.ItemHeight(24)
				.ListItemsSource(&FilteredItems)
				.OnGenerateRow(this, &SList::OnGenerateRowForList)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("SessionId")
					.DefaultLabel(FText::FromString("Session ID"))
					.FillWidth(2)
					.OnSort_Raw(this, &SList::OnSortColumn) // 並び替え
					.SortMode(this, &SList::GetSortModeForColumn, FName("SessionId"))

					+ SHeaderRow::Column("ProjectId")
					.DefaultLabel(FText::FromString("Project ID"))
					.FillWidth(2)
					.OnSort_Raw(this, &SList::OnSortColumn)
					.SortMode(this, &SList::GetSortModeForColumn, FName("ProjectId"))

					+ SHeaderRow::Column("Name")
					.DefaultLabel(FText::FromString("Name"))
					.FillWidth(3)
					.OnSort_Raw(this, &SList::OnSortColumn)
					.SortMode(this, &SList::GetSortModeForColumn, FName("Name"))

					+ SHeaderRow::Column("startTime")
					.DefaultLabel(FText::FromString("Start Time"))
					.FillWidth(3)
					.OnSort_Raw(this, &SList::OnSortColumn)
					.SortMode(this, &SList::GetSortModeForColumn, FName("startTime"))

					+ SHeaderRow::Column("DeviceId")
					.DefaultLabel(FText::FromString("Device ID"))
					.FillWidth(3)
					.OnSort_Raw(this, &SList::OnSortColumn)
					.SortMode(this, &SList::GetSortModeForColumn, FName("DeviceId"))

					+ SHeaderRow::Column("Platform")
					.DefaultLabel(FText::FromString("Platform"))
					.FillWidth(3)
					.OnSort_Raw(this, &SList::OnSortColumn)
					.SortMode(this, &SList::GetSortModeForColumn, FName("Platform"))
				)
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
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(playSession->SessionId))
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock).Text(FText::AsNumber(playSession->ProjectId))
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock).Text(FText::FromString(playSession->Name))
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock).Text(FText::FromString(playSession->StartTime))
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock).Text(FText::FromString(playSession->DeviceId))
			]
			+ SHorizontalBox::Slot()
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
	FilteredItems.Empty();
	Items.Empty();
	PlaySessionData.Empty();
}

void SList::RequestListRefresh()
{
	UpdateFilteredItems();
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
}

void SList::OnFilterTextChanged(const FText& InText)
{
	FilterText = InText.ToString();
	RequestListRefresh(); // リストを更新
}

void SList::UpdateFilteredItems()
{
	FilteredItems.Empty();
	if (FilterText.IsEmpty())
	{
		FilteredItems = Items;
	}
	else
	{
		for (const auto& Item : Items)
		{
			auto Dto = PlaySessionData.FindByPredicate([Item](const TSharedPtr<FPlaySessionResponseDto>& dto) {
				return FString::FromInt(dto->SessionId) == *Item;
			})->Get();
			if (Dto->Name.Contains(FilterText))
			{
				FilteredItems.Add(Item);
				continue;
			}
			if (Dto->DeviceId.Contains(FilterText))
			{
				FilteredItems.Add(Item);
				continue;
			}
			if (Dto->Platform.Contains(FilterText))
			{
				FilteredItems.Add(Item);
				continue;
			}
			if (Dto->StartTime.Contains(FilterText))
			{
				FilteredItems.Add(Item);
				continue;
			}
			if (Dto->ProjectId == FCString::Atoi(*FilterText))
			{
				FilteredItems.Add(Item);
				continue;
			}
			if (FString::FromInt(Dto->SessionId).Contains(FilterText))
			{
				FilteredItems.Add(Item);
				continue;
			}
		}
	}
    
	// フィルタリング後に並び替えを適用
	UpdateSortedItems();
}

void SList::OnSortColumn(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type NewSortMode)
{
	SortColumn = ColumnId;
	UE_LOG(LogTemp, Log, TEXT("SortColumn: %s"), *SortColumn.ToString());
	bSortAscending = (NewSortMode == EColumnSortMode::Ascending);
	UpdateSortedItems(); // 並び替え更新
	RequestListRefresh(); // リストを更新
}

EColumnSortMode::Type SList::GetSortModeForColumn(FName ColumnId) const
{
	if (SortColumn == ColumnId)
	{
		return bSortAscending ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
	}
	return EColumnSortMode::None;
}

void SList::UpdateSortedItems()
{
	UE_LOG(LogTemp, Log, TEXT("SortColumn: %s"), *SortColumn.ToString());
	UE_LOG(LogTemp, Log, TEXT("SortAscending: %d"), bSortAscending);
	for (TSharedPtr<FString>& FilteredItem : FilteredItems)
	{
		UE_LOG(LogTemp, Log, TEXT("FilteredItem: %s"), **FilteredItem.Get());
	} 
	FilteredItems.Sort([this](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B)
	{
		auto ItemA = PlaySessionData.FindByPredicate([A](const TSharedPtr<FPlaySessionResponseDto>& dto) {
			return FString::FromInt(dto->SessionId) == *A;
		})->Get();
		auto ItemB = PlaySessionData.FindByPredicate([B](const TSharedPtr<FPlaySessionResponseDto>& dto) {
			return FString::FromInt(dto->SessionId) == *B;
		})->Get();
		// SortColumnに基づく並び替え（例: SessionId列）
		if (SortColumn == "SessionId")
		{
			return bSortAscending ? ItemA->SessionId < ItemB->SessionId : ItemA->SessionId > ItemB->SessionId;
		}
		else if (SortColumn == "ProjectId")
		{
			return bSortAscending ? ItemA->ProjectId < ItemB->ProjectId : ItemA->ProjectId > ItemB->ProjectId;
		}
		else if (SortColumn == "Name")
		{
			return bSortAscending ? ItemA->Name < ItemB->Name : ItemA->Name > ItemB->Name;
		}
		else if (SortColumn == "startTime")
		{
			return bSortAscending ? ItemA->StartTime < ItemB->StartTime : ItemA->StartTime > ItemB->StartTime;
		}
		else if (SortColumn == "DeviceId")
		{
			return bSortAscending ? ItemA->DeviceId < ItemB->DeviceId : ItemA->DeviceId > ItemB->DeviceId;
		}
		else if (SortColumn == "Platform")
		{
			return bSortAscending ? ItemA->Platform < ItemB->Platform : ItemA->Platform > ItemB->Platform;
		}
		return false;
	});

	for (TSharedPtr<FString>& String : FilteredItems)
	{
		UE_LOG(LogTemp, Log, TEXT("FilteredItem: %s"), **String.Get());
	} 
	
}

FReply SList::OnFilterByDeviceIdClicked()
{
	// Unreal Engine の API でデバイス ID を取得
	FString DeviceId = FPlatformProcess::ComputerName();

	UE_LOG(LogTemp, Log, TEXT("Current Device ID: %s"), *DeviceId);

	// フィルタを適用
	FilterText = DeviceId;
	UpdateFilteredItems();
	RequestListRefresh();

	return FReply::Handled();
}