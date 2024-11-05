#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "FPlaySessionResponse.generated.h"

USTRUCT(BlueprintType)
struct FPlaySessionResponseDto
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SessionId;

	UPROPERTY()
	int32 ProjectId;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString DeviceId;

	UPROPERTY()
	FString Platform;

	UPROPERTY()
	FString AppVersion;

	UPROPERTY()
	FString StartTime;

	UPROPERTY()
	FString EndTime;

	UPROPERTY()
	bool bIsPlaying;
	// JSONの自動変換を可能にするために必要
	FPlaySessionResponseDto()
		: SessionId(0), ProjectId(0), bIsPlaying(false)
	{}

	static bool ParseArrayFromJson(const FString& JsonString, TArray<FPlaySessionResponseDto>& OutArray)
	{
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(Reader, JsonArray))
		{
			for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
			{
				TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
				if (JsonObject.IsValid())
				{
					FPlaySessionResponseDto Item;
					if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &Item))
					{
						OutArray.Add(Item);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Failed to convert JsonObject to FPlaySessionResponseDto"));
						return false;
					}
				}
			}
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON array"));
			return false;
		}
	}
};
