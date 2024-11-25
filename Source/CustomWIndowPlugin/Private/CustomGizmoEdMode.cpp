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

void FCustomGizmoEdMode::SetHeatmapData(const TArray<FPlaySessionHeatmapResponseDto>& NewHeatmapData)
{
	// 渡されたヒートマップデータを格納
	HeatmapArray = NewHeatmapData;

	CalculateBoundingBox();
	// ヒートマップデータの初期化（位置生成）
	GenerateDrawPositions();
}

void FCustomGizmoEdMode::CalculateBoundingBox()
{
	// 初期値は非常に大きな値
	float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
	float maxX = FLT_MIN, maxY = FLT_MIN, maxZ = FLT_MIN;

	// HeatmapArrayからmin, maxを計算
	for (const FPlaySessionHeatmapResponseDto& Data : HeatmapArray)
	{
		minX = FMath::Min(minX, Data.X);
		minY = FMath::Min(minY, Data.Y);
		minZ = FMath::Min(minZ, Data.Z);

		maxX = FMath::Max(maxX, Data.X);
		maxY = FMath::Max(maxY, Data.Y);
		maxZ = FMath::Max(maxZ, Data.Z);
		MaxDensityValue = FMath::Max(MaxDensityValue, Data.Density);
	}

	// 計算したmin, maxをBoundingBoxに設定
	BoundingBox.Min = FVector(minX, minY, minZ);
	BoundingBox.Max = FVector(maxX, maxY, maxZ);
}
void FCustomGizmoEdMode::Enter()
{
	FEdMode::Enter();
	// 必要に応じて初期化コードを追加
}

void FCustomGizmoEdMode::Exit()
{
	// 必要に応じてクリーンアップコードを追加
	FEdMode::Exit();
}


void FCustomGizmoEdMode::GenerateDrawPositions()
{
	DrawPositions.Empty();
	const float StepSize = 100.0f;  // 一定間隔で配置する距離
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
				if (!bDrawZAxis && Z != StepsZ)  // Z軸の描画がOFFの場合、Z軸の最大値を除外
				{
					continue;
				}
				FVector Position = BoundingBox.Min + FVector(X * StepSize, Y * StepSize, Z * StepSize);

				// 最も近いデータ点を見つける
				float MinDistance = FLT_MAX;
				float ClosestDensity = 0.0f;
				for (const FPlaySessionHeatmapResponseDto& Data : HeatmapArray)
				{
					float Distance = FVector::Dist(Position, FVector(Data.X, Data.Y, Data.Z));
					if (Distance < MinDistance)
					{
						MinDistance = Distance;
						ClosestDensity = Data.Density;
					}
				}

				// 密度に基づいて色を決定
				FColor Color = FColor::MakeRedToGreenColorFromScalar(ClosestDensity / MaxDensityValue);
				Color.A = FMath::Clamp(255 - static_cast<int32>(MinDistance / MaxDistance * 255), 0, 255);  // 距離に応じて透明度調整

				// 描画データに追加
				DrawPositions.Add({Position, Color});
			}
		}
	}
}