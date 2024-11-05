#pragma once
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

struct PlayerPosition {
    int32_t player; // プレイヤー識別子
    float x;
    float y;
    float z; // オプション（ここでは必須と仮定）
    uint64_t offsetTimestamp;
};

class LudiscanClient {
    TArray<uint8> ConstructBinaryData(int players, int stampCount, const TArray<TArray<PlayerPosition>>& allPositions) {
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
                const PlayerPosition& pos = allPositions[i][p];

                // プレイヤー識別子をリトルエンディアンで追加
                int32_t playerLE = pos.player;
                uint8* playerPtr = reinterpret_cast<uint8*>(&playerLE);
                buffer.Append(playerPtr, sizeof(int32_t));

                // x, y, zをリトルエンディアンで追加
                float x = pos.x;
                float y = pos.y;
                float z = pos.z;
                buffer.Append(reinterpret_cast<uint8*>(&x), sizeof(float));
                buffer.Append(reinterpret_cast<uint8*>(&y), sizeof(float));
                buffer.Append(reinterpret_cast<uint8*>(&z), sizeof(float));

                // offsetTimestampをリトルエンディアンで追加
                uint64_t tsLE = pos.offsetTimestamp;
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
    FString bApiHostName;
public:

    void SetConfig(FString ApiHostName) {
        this->bApiHostName = ApiHostName;
    }

    void CreatePositionsPost(
        int projectId,
        int sessionId,
        int players,
        int stampCount,
        const TArray<TArray<PlayerPosition>>& allPositions
    ) {
        TArray<uint8> BinaryData = ConstructBinaryData(players, stampCount, allPositions);
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateHttpContent(BinaryData);
       
        Request->SetURL(bApiHostName + "/api/v0/projects/" + FString::FromInt(projectId) + "/play_session/" + FString::FromInt(sessionId) + "/player_position_log");
        Request->SetVerb("POST");

        // 完了時のログ出力
        Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
            if (bWasSuccessful && Response.IsValid()) {
                UE_LOG(LogTemp, Log, TEXT("Request succeeded: %s"), *Response->GetContentAsString());
            } else {
                UE_LOG(LogTemp, Error, TEXT("Request failed"));
            }
        });


        Request->ProcessRequest();
    }

    static void PlayGround()
    {
        int players = 4;
        int stampcount = 40;

        TArray<TArray<PlayerPosition>> allPositions;
        allPositions.Reserve(stampcount);

        for (int i = 0; i < stampcount; ++i) {
            TArray<PlayerPosition> positions;
            positions.Reserve(players);
            for (int p = 0; p < players; ++p) {
                positions.Add(PlayerPosition{
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
