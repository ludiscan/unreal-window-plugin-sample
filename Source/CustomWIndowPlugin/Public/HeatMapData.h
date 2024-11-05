#pragma once

#include "HeatMapData.generated.h"

USTRUCT()
struct FHeatMapData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Position;

	UPROPERTY()
	float Density;
};