#include "NeedOfSpeed/Public/AI/CPP_AIRaceManager.h"
#include "NeedOfSpeed/Public/AI/UIn_isVehicle.h"
#include "NeedOfSpeed/Public/AI/CPP_Road.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"

ACPP_AIRaceManager::ACPP_AIRaceManager()
{
	PrimaryActorTick.bCanEverTick = false; // Tick 대신 Timer 사용
}

ACPP_AIRaceManager* ACPP_AIRaceManager::GetInstance(UWorld* World)
{
	return Cast<ACPP_AIRaceManager>(UGameplayStatics::GetActorOfClass(World, ACPP_AIRaceManager::StaticClass()));
}

void ACPP_AIRaceManager::BeginPlay()
{
	Super::BeginPlay();

	// 도로 액터 찾기
	TargetRoad = Cast<ACPP_Road>(UGameplayStatics::GetActorOfClass(GetWorld(), ACPP_Road::StaticClass()));

	// 모든 AI 차량(및 플레이어)을 초기 테이블에 등록
	TArray<AActor*> FoundVehicles;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUIn_isVehicle::StaticClass(), FoundVehicles);

	for (AActor* V : FoundVehicles)
	{
		FRacerInfo Info;
		Info.Vehicle = V;
		RacerTable.Add(Info);
	}

	// 주기적 업데이트 시작
	GetWorldTimerManager().SetTimer(UpdateTimerHandle, this, &ACPP_AIRaceManager::UpdateRaceData, UpdateInterval, true);
}

void ACPP_AIRaceManager::UpdateRaceData()
{
	if (!TargetRoad || !TargetRoad->Spline) return;

	const float TrackLength = TargetRoad->Spline->GetSplineLength();
	const int32 FinishLap = 2;

	// ======================
	// 1️⃣ 거리 + 랩 계산
	// ======================
	for (auto& Info : RacerTable)
	{
		if (!IsValid(Info.Vehicle)) continue;

		FVector Loc = Info.Vehicle->GetActorLocation();

		float Key = TargetRoad->Spline->FindInputKeyClosestToWorldLocation(Loc);
		float NewDistance =
			TargetRoad->Spline->GetDistanceAlongSplineAtSplineInputKey(Key);

		if (Info.DistanceAlongSpline > TrackLength - 1000.f &&
			NewDistance < 1000.f)
		{
			Info.Lap++;
		}

		Info.PreviousDistance = Info.DistanceAlongSpline;
		Info.DistanceAlongSpline = NewDistance;
		Info.RaceProgress = Info.Lap * TrackLength + Info.DistanceAlongSpline;
	}

	// ======================
	// 2️⃣ 정렬
	// ======================
	RacerTable.Sort([](const FRacerInfo& A, const FRacerInfo& B)
	{
		return A.RaceProgress > B.RaceProgress;
	});

	// ======================
	// 3️⃣ Rank 부여
	// ======================
	if (RacerTable.Num() > 0)
	{
		LeadDistance = RacerTable[0].RaceProgress;

		for (int32 i = 0; i < RacerTable.Num(); i++)
		{
			RacerTable[i].Rank = i + 1;
		}
	}

	// ======================
	// 4️⃣ Finish 기록 ⭐
	// ======================
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (const FRacerInfo& Info : RacerTable)
	{
		if (Info.Lap >= FinishLap &&
			!FinishedSet.Contains(Info.Vehicle))
		{
			FinishedSet.Add(Info.Vehicle);

			FFinishRecord Record;
			Record.Vehicle = Info.Vehicle;
			Record.FinishRank = FinishedRacers.Num() + 1;
			Record.FinishTime = CurrentTime;

			FinishedRacers.Add(Record);

			// ⭐ 모든 차량 완주 이벤트
			OnRacerFinished.Broadcast(Record);

			APawn* Pawn = Cast<APawn>(Info.Vehicle);
			if (Pawn && Pawn->IsPlayerControlled())
			{
				OnPlayerFinished.Broadcast(Info.Vehicle);
			}

			UE_LOG(LogTemp, Warning,
				TEXT("FINISH: %s Rank %d Time %.2f"),
				*Info.Vehicle->GetActorLabel(),
				Record.FinishRank,
				Record.FinishTime);
		}
	}

	// ======================
	// 5️⃣ 로그 출력
	// ======================
	if (CurrentTime - LastLogTime >= 1.0f)
	{
		PrintCurrentRankings();
		LastLogTime = CurrentTime;
	}

	if (RespawnSlotCounter.Num() > 50)
	{
		RespawnSlotCounter.Empty();
	}
}

AActor* ACPP_AIRaceManager::GetVehicleByRank(int32 Rank)
{
	for (const auto& Info : RacerTable)
	{
		if (Info.Rank == Rank)
		{
			return Info.Vehicle;
		}
	}
	return nullptr;
}

int32 ACPP_AIRaceManager::AcquireRespawnSlot(float Distance)
{
	// 500cm 단위로 구간 나누기
	int32 Bucket = FMath::FloorToInt(Distance / 500.f);

	// 해당 구간 슬롯 카운터
	int32& Counter = RespawnSlotCounter.FindOrAdd(Bucket);

	int32 SlotIndex = Counter;
	Counter++;

	return SlotIndex;
}

FRacerInfo ACPP_AIRaceManager::GetRacerInfo(AActor* Vehicle) const
{
	for (const FRacerInfo& Info : RacerTable)
	{
		if (Info.Vehicle == Vehicle)
			return Info;
	}
	return FRacerInfo(); // 못 찾으면 기본값
}

int32 ACPP_AIRaceManager::GetRankOfVehicle(AActor* Vehicle)
{
	for (const auto& Info : RacerTable)
	{
		if (Info.Vehicle == Vehicle) return Info.Rank;
	}
	return 1;
}

float ACPP_AIRaceManager::GetDistanceOfVehicle(AActor* Vehicle)
{
	for (const auto& Info : RacerTable)
	{
		if (Info.Vehicle == Vehicle) return Info.DistanceAlongSpline;
	}
	return 0.f;
}

void ACPP_AIRaceManager::PrintCurrentRankings()
{
	UE_LOG(LogTemp, Warning, TEXT("=== CURRENT RANKINGS ==="));
	for (const auto& Info : RacerTable)
	{
		UE_LOG(LogTemp, Log, TEXT("%d위: %s | Lap: %d"), Info.Rank, *Info.Vehicle->GetActorLabel(), Info.Lap);
	}
}