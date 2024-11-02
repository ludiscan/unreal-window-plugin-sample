#pragma once

class SList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SList ) {}
	SLATE_END_ARGS()
 
	/** コンストラクタ */ 
	void Construct( const FArguments& Args );
 
	/** 行追加イベント */
	TSharedRef<ITableRow> OnGenerateRowForList( TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable );
    
	/** 要素追加処理 */
	void AddItem( const FString& item );
    
	/** リストクリア処理 */
	void ClearList();
 
protected:
	/** ヘッダー */
	TSharedPtr<SHeaderRow> Header;

	/** リストビュー表示アイテム */
	TArray<TSharedPtr<FString>> Items;

	/** リストビューウィジェット */
	TSharedPtr< SListView<TSharedPtr<FString>> > ListViewWidget;
};