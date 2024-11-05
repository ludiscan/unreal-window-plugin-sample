#pragma once
#include "EdMode.h"
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

private:
	void InitializeHeatMapData();
	void GenerateDrawPositions();

	TArray<FHeatMapData> HeatMapData;
	FBox BoundingBox;
	float MaxDensityValue = 1.0f;
	float MaxDistance = 100.0f;
	float PointSize = 5.0f;

	TArray<TPair<FVector, FColor>> DrawPositions;
};
