// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LudiscanClient.h"
#include "UObject/Object.h"
#include "PositionRecorder.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMWINDOWPLUGIN_API UPositionRecorder : public UObject
{
	GENERATED_BODY()
public:
	UPositionRecorder();

	void StartRecording(UWorld* Context);
	void StopRecording();
	const TArray<TArray<FPlayerPosition>>& GetPositionData() const;

private:
	TArray<TArray<FPlayerPosition>> PositionData;
	FTimerHandle TimerHandle;
	uint64 StartTime;

	UPROPERTY()
	UWorld* WorldContext;

	void RecordPlayerPositions();
};
