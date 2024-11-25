#pragma once
#include "FPlaySessionHeatmapResponseDto.h"
#include "HttpModule.h"
#include "PlaySessionCreate.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

struct FPlayerPosition {
    int32_t Player; // プレイヤー識別子
    float X;
    float Y;
    float Z; // オプション（ここでは必須と仮定）
    uint64_t OffsetTimestamp;
};

class LudiscanClient {
    TArray<uint8> ConstructBinaryData(int players, int stampCount, const TArray<TArray<FPlayerPosition>>& allPositions) {
        TArray<uint8> buffer;

        // メタ情報の追加（playersとstampcount）
        buffer.SetNum(8);
        buffer[0] = players & 0xFF;
        buffer[1] = (players >> 8) & 0xFF;
        buffer[2] = (players >> 16) & 0xFF;
        buffer[3] = (players >> 24) & 0xFF;

        buffer[4] = stampCount & 0xFF;
        buffer[5] = (stampCount >> 8) & 0xFF;
        buffer[6] = (stampCount >> 16) & 0xFF;
        buffer[7] = (stampCount >> 24) & 0xFF;

        // 各タイムスタンプごとのデータを追加
        for (int i = 0; i < stampCount; ++i) {
            for (int p = 0; p < players; ++p) {
                const FPlayerPosition& pos = allPositions[i][p];

                // プレイヤー識別子をリトルエンディアンで追加
                int32_t playerLE = pos.Player;
                uint8* playerPtr = reinterpret_cast<uint8*>(&playerLE);
                buffer.Append(playerPtr, sizeof(int32_t));

                // x, y, zをリトルエンディアンで追加
                float x = pos.X;
                float y = pos.Y;
                float z = pos.Z;
                buffer.Append(reinterpret_cast<uint8*>(&x), sizeof(float));
                buffer.Append(reinterpret_cast<uint8*>(&y), sizeof(float));
                buffer.Append(reinterpret_cast<uint8*>(&z), sizeof(float));

                // offsetTimestampをリトルエンディアンで追加
                uint64_t tsLE = pos.OffsetTimestamp;
                uint8* tsPtr = reinterpret_cast<uint8*>(&tsLE);
                buffer.Append(tsPtr, sizeof(uint64_t));
            }
        }

        return buffer;
    }

     TSharedRef<IHttpRequest> CreateHttpContent(const TArray<uint8>& BinaryData) {
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

        // バイナリデータをリクエストに追加
        Request->SetContent(BinaryData);
        Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));
        
        // `multipart/form-data`形式でリクエストを構築
        FString Boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
        FString ContentType = FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary);
        Request->SetHeader(TEXT("Content-Type"), *ContentType);

        // `file`フィールドとしてバイナリデータを追加
        FString Payload;
        Payload += "--" + Boundary + "\r\n";
        Payload += "Content-Disposition: form-data; name=\"file\"; filename=\"data.bin\"\r\n";
        Payload += "Content-Type: application/octet-stream\r\n\r\n";

        // バイナリデータのペイロードを組み立て
        TArray<uint8> PayloadData;
        PayloadData.Append(reinterpret_cast<uint8*>(TCHAR_TO_UTF8(*Payload)), Payload.Len());
        PayloadData.Append(BinaryData);
        FString EndBoundary = "\r\n--" + Boundary + "--\r\n";
        PayloadData.Append(reinterpret_cast<uint8*>(TCHAR_TO_UTF8(*EndBoundary)), EndBoundary.Len());

        // リクエストにペイロードをセット
        Request->SetContent(PayloadData);
        
        return Request;
    }
    bool ConvertJsonToPlaySession(const FString& JsonString, FPlaySessionCreate& OutPlaySession)
    {
        // JSON文字列を解析
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            OutPlaySession.sessionId = JsonObject->GetIntegerField("sessionId");
            UE_LOG(LogTemp, Log, TEXT("Session ID: %d"), OutPlaySession.sessionId);
            OutPlaySession.projectId = JsonObject->GetIntegerField("projectId");
            UE_LOG(LogTemp, Log, TEXT("Project ID: %d"), OutPlaySession.projectId);
            OutPlaySession.name = JsonObject->GetStringField("name");
            UE_LOG(LogTemp, Log, TEXT("Name: %s"), *OutPlaySession.name);
            OutPlaySession.deviceId = JsonObject->GetStringField("deviceId");
            UE_LOG(LogTemp, Log, TEXT("Device ID: %s"), *OutPlaySession.deviceId);
            OutPlaySession.platform = JsonObject->GetStringField("platform");
            UE_LOG(LogTemp, Log, TEXT("Platform: %s"), *OutPlaySession.platform);
            OutPlaySession.appVersion = JsonObject->GetStringField("appVersion");
            UE_LOG(LogTemp, Log, TEXT("App Version: %s"), *OutPlaySession.appVersion);

            // Optionalな値はIsFieldPresentを使用してチェック
            if (FString StartTime; JsonObject->HasField("startTime") && JsonObject->TryGetStringField("startTime", StartTime))
            {
                OutPlaySession.startTime = StartTime;
                UE_LOG(LogTemp, Log, TEXT("Start Time: %s"), *OutPlaySession.startTime);
            }
            else
            {
                OutPlaySession.startTime = TEXT("N/A"); // デフォルト値
            }

            if (FString EndTime; JsonObject->HasField("endTime") && JsonObject->TryGetStringField("endTime", EndTime))
            {
                OutPlaySession.endTime = EndTime;
                UE_LOG(LogTemp, Log, TEXT("End Time: %s"), *OutPlaySession.endTime);
            }
            else
            {
                OutPlaySession.endTime = TEXT("N/A"); // デフォルト値
            }

            OutPlaySession.isPlaying = JsonObject->GetBoolField("isPlaying");

            return true;
        }

        return false;
    }

    FString bApiHostName;
    bool bIsDebug = false;
public:

    void SetConfig(
        FString ApiHostName,
        bool isDebug = false
    ) {
        this->bApiHostName = ApiHostName;
        this->bIsDebug = isDebug;
    }

    void CreatePositionsPost(
        int projectId,
        int sessionId,
        int players,
        int stampCount,
        const TArray<TArray<FPlayerPosition>>& allPositions,
        TFunction<void()> OnSuccess = []() {}
    ) {
        TArray<uint8> BinaryData = ConstructBinaryData(players, stampCount, allPositions);
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateHttpContent(BinaryData);
       
        Request->SetURL(bApiHostName + "/api/v0/projects/" + FString::FromInt(projectId) + "/play_session/" + FString::FromInt(sessionId) + "/player_position_log");
        Request->SetVerb("POST");

        if (bIsDebug)
        {
        }
        // 完了時のログ出力
        Request->OnProcessRequestComplete().BindLambda([OnSuccess](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
            if (bWasSuccessful && Response.IsValid()) {
                UE_LOG(LogTemp, Log, TEXT("Request succeeded: %s"), *Response->GetContentAsString());
                OnSuccess();
            } else {
                UE_LOG(LogTemp, Error, TEXT("Request failed"));
            }
        });


        Request->ProcessRequest();
    }

    void FinishedSession(
        int projectId,
        int sessionId,
        TFunction<void(FPlaySessionCreate)> OnSuccess = [](FPlaySessionCreate PlaySession) {}
    )
    {
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        Request->SetURL(bApiHostName + "/api/v0/projects/" + FString::FromInt(projectId) + "/play_session/" + FString::FromInt(sessionId) + "/finish");
        Request->SetVerb("POST");

        Request->OnProcessRequestComplete().BindLambda([this, OnSuccess](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
            if (bWasSuccessful && Response.IsValid()) {
                FString ResponseContent = Response->GetContentAsString();
                FPlaySessionCreate PlaySession;
                if (ConvertJsonToPlaySession(ResponseContent, PlaySession)) {
                    OnSuccess(PlaySession);
                }
            }
        });
        Request->ProcessRequest();
    }

    void GetHeatMap(
        int projectId,
        int sessionId,
        TFunction<void(TArray<FPlaySessionHeatmapResponseDto>)> OnSuccess,
        TFunction<void(FString)> OnFailure = [](FString Message) {},
        int stepSize = 300,
        bool zVisualize = false
    )
    {
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        FString Url = bApiHostName + FString::Printf(TEXT("/api/v0/projects/%d/play_session/%d/player_position_log/heatmap"), projectId, sessionId);
        Url += FString::Printf(TEXT("?stepSize=%d"), stepSize);
        Url += zVisualize ? TEXT("&zVisualize=true") : TEXT("&zVisualize=false");
        Request->SetURL(Url);
        Request->SetVerb("GET");
        // 完了時のログ出力
        Request->OnProcessRequestComplete().BindLambda([OnSuccess, OnFailure](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
            if (bWasSuccessful && Response.IsValid())
            {
                // レスポンスを文字列として取得
                FString ResponseContent = Response->GetContentAsString();

                // レスポンスのJSONを解析
                TArray<FPlaySessionHeatmapResponseDto> HeatmapList;
                
                if (ResponseContent != "[]" && FPlaySessionHeatmapResponseDto::ParseArrayFromJson(ResponseContent, HeatmapList))
                {
                    // JSONの解析が成功した場合
                    return OnSuccess(HeatmapList);
                }
                UE_LOG(LogTemp, Error, TEXT("Failed to deserialize the JSON response."));
                return OnFailure("Failed to deserialize the JSON response.");
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Request failed"));
                return OnFailure("Request failed");
            }
        });


        Request->ProcessRequest();
    }

    void CreateSession(
        int projectId,
        const FString& Name,
        const FString& DeviceId,
        const FString& Platform,
        const FString& AppVersion,
        TFunction<void(FPlaySessionCreate)> OnResponse
    )
    {
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        Request->SetURL(bApiHostName + "/api/v0/projects/" + FString::FromInt(projectId) + "/play_session");
        Request->SetVerb("POST");
        // JSONデータの作成
        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
        JsonObject->SetStringField("name", Name);
        JsonObject->SetStringField("deviceId", DeviceId);
        JsonObject->SetStringField("platform", Platform);
        JsonObject->SetStringField("appVersion", AppVersion);

        FString RequestBody;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

        // リクエストボディ設定
        Request->SetContentAsString(RequestBody);
        Request->SetHeader("Content-Type", "application/json");

        Request->OnProcessRequestComplete().BindLambda([OnResponse, this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
            if (bWasSuccessful && Response.IsValid())
            {
                FString ResponseContent = Response->GetContentAsString();
                FPlaySessionCreate PlaySession;
                if (ConvertJsonToPlaySession(ResponseContent, PlaySession))
                {
                    OnResponse(PlaySession);
                }
            }
        });
        Request->ProcessRequest();
    }
    
    

    static void PlayGround()
    {
        int players = 4;
        int stampcount = 40;

        TArray<TArray<FPlayerPosition>> allPositions;
        allPositions.Reserve(stampcount);

        for (int i = 0; i < stampcount; ++i) {
            TArray<FPlayerPosition> positions;
            positions.Reserve(players);
            for (int p = 0; p < players; ++p) {
                positions.Add(FPlayerPosition{
                    p + 1, // プレイヤーID: 1, 2, 3, 4
                    static_cast<float>(p * 1.0 + i * 0.1), // x
                    static_cast<float>(p * 2.0 + i * 0.2), // y
                    static_cast<float>(p * 3.0 + i * 0.3), // z
                    static_cast<uint64_t>(i * 1000 + p)      // offsetTimestamp
                });
            }
            allPositions.Add(positions);
        }

        auto ludiscanClient = MakeShared<LudiscanClient>();
        ludiscanClient->SetConfig("http://localhost:3000");
        ludiscanClient->CreatePositionsPost(
            1,
            1,
            players,
            stampcount,
            allPositions
        );
    }
};
