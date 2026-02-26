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

	// 1. 모든 차량의 스플라인 거리 계산
	for (auto& Info : RacerTable)
	{
		if (IsValid(Info.Vehicle))
		{
			FVector Loc = Info.Vehicle->GetActorLocation();
			float Key = TargetRoad->Spline->FindInputKeyClosestToWorldLocation(Loc);
			Info.DistanceAlongSpline = TargetRoad->Spline->GetDistanceAlongSplineAtSplineInputKey(Key);
		}
	}

	// 2. 거리순 정렬 (내림차순)
	RacerTable.Sort([](const FRacerInfo& A, const FRacerInfo& B) {
		return A.DistanceAlongSpline > B.DistanceAlongSpline;
	});

	// 3. 순위 부여 및 선두 거리 갱신
	if (RacerTable.Num() > 0)
	{
		LeadDistance = RacerTable[0].DistanceAlongSpline;
		for (int32 i = 0; i < RacerTable.Num(); ++i)
		{
			RacerTable[i].Rank = i + 1;
		}
	}
	
	// 4. 1초마다 한 번만 모든 차량의 순위를 출력
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastLogTime >= 1.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("======= [Race Manager] GLOBAL RANKINGS ======="));
		for (const auto& Info : RacerTable)
		{
			if (IsValid(Info.Vehicle))
			{
				UE_LOG(LogTemp, Log, TEXT("Rank %d: %s (Dist: %.0f)"), 
					Info.Rank, *Info.Vehicle->GetActorLabel(), Info.DistanceAlongSpline);
			}
		}
		LastLogTime = CurrentTime;
	}
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