#pragma once
#include "EdMode.h"
#include "FPlaySessionHeatmapResponseDto.h"
#include "HeatMapData.h"


class FCustomGizmoEdMode: public FEdMode
{
public:
	/** エディタモードID */
	const static FEditorModeID EM_CustomGizmoEdMode;

	FCustomGizmoEdMode();
	virtual ~FCustomGizmoEdMode();

	// FEdModeのオーバーライドメソッド
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

	void SetHeatmapData(const TArray<FPlaySessionHeatmapResponseDto>& NewHeatmapData);

	void CalculateBoundingBox();
private:
	void GenerateDrawPositions();
	FBox BoundingBox;
	float MaxDensityValue = 30.0f;
	float MaxDistance = 100.0f;
	float PointSize = 5.0f;
	bool bDrawZAxis = true;

	TArray<FPlaySessionHeatmapResponseDto> HeatmapArray;

	TArray<TPair<FVector, FColor>> DrawPositions;
};
