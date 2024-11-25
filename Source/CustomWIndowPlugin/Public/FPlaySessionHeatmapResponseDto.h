#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "FPlaySessionHeatmapResponseDto.generated.h"

USTRUCT(BlueprintType)
struct FPlaySessionHeatmapResponseDto
{
	GENERATED_BODY()
	
	UPROPERTY()
	float X;

	UPROPERTY()
	float Y;

	UPROPERTY()
	float Z;

	UPROPERTY()
	int32 Density;

	FPlaySessionHeatmapResponseDto(): X(0), Y(0), Z(0), Density(0)
	{
	}

	static bool ParseArrayFromJson(const FString& JsonString, TArray<FPlaySessionHeatmapResponseDto>& OutArray)
	{
		// JSON文字列をパースするためのリーダーを作成
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		// JSON文字列を配列に変換
		if (FJsonSerializer::Deserialize(Reader, JsonArray))
		{
			// 配列の各要素を処理
			for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
			{
				// 各要素をJsonObjectに変換
				TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
				if (JsonObject.IsValid())
				{
					FPlaySessionHeatmapResponseDto Item;

					// JsonObjectを構造体に変換
					if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &Item))
					{
						OutArray.Add(Item); // 成功したらOutArrayに追加
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Failed to convert JsonObject to FPlaySessionHeatmapResponseDto"));
						return false; // 変換失敗した場合はfalseを返す
					}
				}
			}
			return true; // 配列全体の変換が成功した場合
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON array"));
			return false; // JSONパースが失敗した場合
		}
	}
};
