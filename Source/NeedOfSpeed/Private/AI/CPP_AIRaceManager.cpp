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

	float TrackLength = TargetRoad->Spline->GetSplineLength();

	for (auto& Info : RacerTable)
	{
		if (!IsValid(Info.Vehicle)) continue;

		FVector Loc = Info.Vehicle->GetActorLocation();

		float Key = TargetRoad->Spline->FindInputKeyClosestToWorldLocation(Loc);
		float NewDistance = TargetRoad->Spline->GetDistanceAlongSplineAtSplineInputKey(Key);

		// 랩 체크 (루프 트랙 대응)
		if (NewDistance < Info.PreviousDistance - 2000.f)
		{
			Info.Lap++;
		}

		Info.PreviousDistance = Info.DistanceAlongSpline;
		Info.DistanceAlongSpline = NewDistance;

		// 레이스 진행도 계산
		Info.RaceProgress = Info.Lap * TrackLength + Info.DistanceAlongSpline;
	}

	// 진행도로 정렬
	RacerTable.Sort([](const FRacerInfo& A, const FRacerInfo& B) {
		return A.RaceProgress > B.RaceProgress;
	});

	if (RacerTable.Num() > 0)
	{
		LeadDistance = RacerTable[0].RaceProgress;

		for (int32 i = 0; i < RacerTable.Num(); i++)
		{
			RacerTable[i].Rank = i + 1;
		}
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
		UE_LOG(LogTemp, Log, TEXT("%d위: %s"), Info.Rank, *Info.Vehicle->GetActorLabel());
	}
}