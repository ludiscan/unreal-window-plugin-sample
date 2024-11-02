#include "SList.h"

#include "SList.h"

/**
 * コンストラクタ
 */
void SList::Construct( const FArguments& Args )
{
	ChildSlot
	[
		SNew( SVerticalBox )
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// ヘッダー追加
			SNew( SHeaderRow )
			+ SHeaderRow::Column( TEXT("ListName") )
			.DefaultLabel( FText::FromString( TEXT("リスト") ) )
			.FillWidth( 1.f )
			.HAlignHeader( HAlign_Center )
		]

		+ SVerticalBox::Slot()
		[
			// スクロールボックス追加
			SNew( SScrollBox )
			+ SScrollBox::Slot()
			[
				// リストビュー追加
				SAssignNew( ListViewWidget, SListView<TSharedPtr<FString>> )
				.ItemHeight( 24 )
				.ListItemsSource( &Items )
				.OnGenerateRow( this, &SList::OnGenerateRowForList )
			]
		]
	];
}

/**
 * リスト要素が生成されるときの処理
 */
TSharedRef<ITableRow> SList::OnGenerateRowForList( TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable )
{
	return
		// リスト要素（行）を作成
		SNew( STableRow< TSharedPtr<FString> >, OwnerTable)
		.Padding( 2.f )
		[
			SNew( STextBlock ).Text( FText::FromString( *Item.Get() ) )
		];
}

/**
 * 要素追加処理
 */
void SList::AddItem( const FString& item )
{
	// 新規追加
	Items.Add( MakeShareable( new FString( item ) ) );

	// リストビュー更新
	ListViewWidget->RequestListRefresh();
}

/**
 * リストクリア処理
 */
void SList::ClearList()
{
	// 要素配列クリア
	Items.Empty();

	// リストビュー更新
	ListViewWidget->RequestListRefresh();
}