// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlaySessionCreate.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPlaySessionCreate
{
	GENERATED_BODY()

	UPROPERTY()
	int sessionId;

	UPROPERTY()
	int projectId;

	UPROPERTY()
	FString name;

	UPROPERTY()
	FString deviceId;

	UPROPERTY()
	FString platform;

	UPROPERTY()
	FString appVersion;

	UPROPERTY()
	FString startTime;

	UPROPERTY()
	FString endTime;
	
	UPROPERTY()
	bool isPlaying;
};
