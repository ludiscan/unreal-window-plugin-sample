#include "../Public/CustomGizmoEdMode.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

const FEditorModeID FCustomGizmoEdMode::EM_CustomGizmoEdMode = TEXT("EM_CustomGizmoEdMode");

FCustomGizmoEdMode::FCustomGizmoEdMode()
{
}

FCustomGizmoEdMode::~FCustomGizmoEdMode()
{
}

void FCustomGizmoEdMode::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Draw(View, PDI);
	// 描画位置と色をもとに点を描画
	for (const TPair<FVector, FColor>& Data : DrawPositions)
	{
		PDI->DrawPoint(Data.Key, Data.Value, 20.0f, SDPG_Foreground);
	}
}

void FCustomGizmoEdMode::Enter()
{
	FEdMode::Enter();
	// 必要に応じて初期化コードを追加
	InitializeHeatMapData();
}

void FCustomGizmoEdMode::Exit()
{
	// 必要に応じてクリーンアップコードを追加
	FEdMode::Exit();
}

void FCustomGizmoEdMode::InitializeHeatMapData()
{
	// サンプルデータの生成
	const int32 NumPoints = 52;   // 点の数
	const FVector BoundsMin(-1000.0f, -1000.0f, 0.0f);  // ボックスの最小位置
	const FVector BoundsMax(1000.0f, 1000.0f, 10.0f); // ボックスの最大位置
	MaxDensityValue = 5.0f;

	BoundingBox = FBox(BoundsMin, BoundsMax); // バウンディングボックスを定義

	// ヒートマップデータと描画情報のリストをクリア
	HeatMapData.Empty();
	DrawPositions.Empty();

	// サンプルデータ生成
	for (int32 i = 0; i < NumPoints; ++i)
	{
		FVector Position = FMath::RandPointInBox(BoundingBox);  // ランダムな位置
		float Density = FMath::FRandRange(0.1f, MaxDensityValue);  // ランダムな密度
		HeatMapData.Add({Position, Density});
	}

	// 描画リストを生成
	GenerateDrawPositions();
}


void FCustomGizmoEdMode::GenerateDrawPositions()
{
	const float StepSize = 25.0f;  // 一定間隔で配置する距離
	const FVector BoxSize = BoundingBox.Max - BoundingBox.Min;  // ボックスのサイズ
	const int32 StepsX = FMath::CeilToInt(BoxSize.X / StepSize);
	const int32 StepsY = FMath::CeilToInt(BoxSize.Y / StepSize);
	const int32 StepsZ = FMath::CeilToInt(BoxSize.Z / StepSize);

	// 描画位置と色のリストを生成
	for (int32 X = 0; X <= StepsX; ++X)
	{
		for (int32 Y = 0; Y <= StepsY; ++Y)
		{
			for (int32 Z = 0; Z <= StepsZ; ++Z)
			{
				FVector Position = BoundingBox.Min + FVector(X * StepSize, Y * StepSize, Z * StepSize);

				// 最も近いデータ点を見つける
				float MinDistance = FLT_MAX;
				float ClosestDensity = 0.0f;
				for (const FHeatMapData& Data : HeatMapData)
				{
					float Distance = FVector::Dist(Position, Data.Position);
					if (Distance < MinDistance)
					{
						MinDistance = Distance;
						ClosestDensity = Data.Density;
					}
				}

				// 密度に基づいて色を決定
				FColor Color = FColor::MakeRedToGreenColorFromScalar(ClosestDensity / MaxDensityValue);
				Color.A = FMath::Clamp(255 - static_cast<int32>(MinDistance / BoxSize.Size() * 255), 0, 255);  // 距離に応じて透明度調整

				// 描画データに追加
				DrawPositions.Add({Position, Color});
			}
		}
	}
}