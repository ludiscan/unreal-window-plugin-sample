#pragma once
#include "FPlaySessionResponse.h"

class SList : public SCompoundWidget
{
private:
	TSharedPtr<FPlaySessionResponseDto> SelectedItem;
	void OnFilterTextChanged(const FText& InText);
	void UpdateFilteredItems();
	void OnSortColumn(EColumnSortPriority::Type SortPriority, const FName& ColumnId, EColumnSortMode::Type NewSortMode);
	EColumnSortMode::Type GetSortModeForColumn(FName ColumnId) const;
	void UpdateSortedItems();
	FReply OnFilterByDeviceIdClicked();

public:
	SLATE_BEGIN_ARGS( SList ) {}
	SLATE_END_ARGS()
 
	/** コンストラクタ */ 
	void Construct( const FArguments& Args );
 
	/** 行追加イベント */
	TSharedRef<ITableRow> OnGenerateRowForList( TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable );
    
	/** 要素追加処理 */
	void AddItem( const FPlaySessionResponseDto& item );
    
	/** リストクリア処理 */
	void ClearList();

	void RequestListRefresh();


	TSharedPtr<FPlaySessionResponseDto> GetSelectedItem() const { return SelectedItem; }

	void OnSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
	{
		if (Item == nullptr)
		{
			SelectedItem = nullptr;
			return;
		}
		// 選択された項目のDTOを取得して保存
		auto Element = PlaySessionData.FindByPredicate([Item](const TSharedPtr<FPlaySessionResponseDto>& dto) {
			return FString::FromInt(dto->SessionId) == *Item;
		});
		if (Element)
		{
			SelectedItem = *Element;
		}
	}
 
protected:
	/** ヘッダー */
	TSharedPtr<SHeaderRow> Header;

	/** リストビュー表示アイテム */
	TArray<TSharedPtr<FString>> Items;

	TArray<TSharedPtr<FPlaySessionResponseDto>> PlaySessionData;

	/** リストビューウィジェット */
	TSharedPtr< SListView<TSharedPtr<FString>> > ListViewWidget;

	FString FilterText; // フィルタ用文字列
	bool bSortAscending = true; // 並び替え方向（昇順・降順）
	FName SortColumn = "SessionId";

	TArray<TSharedPtr<FString>> FilteredItems;
};
