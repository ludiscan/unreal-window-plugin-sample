// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionRecorder.h"

UPositionRecorder::UPositionRecorder(): StartTime(0), WorldContext(nullptr)
{
}

void UPositionRecorder::StartRecording(UWorld* Context) {
	this->WorldContext = Context;
	if (TimerHandle.IsValid() || StartTime != 0) {
		StopRecording();
		UE_LOG(LogTemp, Warning, TEXT("Recording already in progress. Stopping previous recording."));
	}
	StartTime = FDateTime::Now().GetTicks();
	this->WorldContext->GetTimerManager().SetTimer(TimerHandle, this, &UPositionRecorder::RecordPlayerPositions, 0.5f, true);
}

void UPositionRecorder::StopRecording() {
	if (TimerHandle.IsValid()) {
		WorldContext->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void UPositionRecorder::RecordPlayerPositions() {
	TArray<FPlayerPosition> CurrentPositions;

	for (FConstPlayerControllerIterator Iterator = WorldContext->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		if (const APlayerController* PC = Iterator->Get()) {
			if (APawn* Pawn = PC->GetPawn()) {
				FVector Location = Pawn->GetActorLocation();
				FPlayerPosition Position;
				Position.Player = PC->GetUniqueID();
				Position.X = Location.X;
				Position.Y = Location.Y;
				Position.Z = Location.Z;
				Position.OffsetTimestamp = (FDateTime::Now().GetTicks() - StartTime) / 10000;

				UE_LOG(LogTemp, Warning, TEXT("Player %d: X=%f, Y=%f, Z=%f time=%d"), Position.Player, Position.X, Position.Y, Position.Z, Position.OffsetTimestamp);
				CurrentPositions.Add(Position);
			}
		}
	}
	PositionData.Add(CurrentPositions);
}

const TArray<TArray<FPlayerPosition>>& UPositionRecorder::GetPositionData() const {
	return PositionData;
}