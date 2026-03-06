#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_AIRaceManager.generated.h"

USTRUCT()
struct FRacerInfo
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Vehicle = nullptr;

	UPROPERTY()
	float DistanceAlongSpline = 0.f;

	UPROPERTY()
	float PreviousDistance = 0.f;

	UPROPERTY()
	int32 Lap = 0;

	UPROPERTY()
	int32 Rank = 0;

	UPROPERTY()
	float RaceProgress = 0.f;
};

UCLASS()
class NEEDOFSPEED_API ACPP_AIRaceManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPP_AIRaceManager();

protected:
	virtual void BeginPlay() override;

public:	
	// 전역 접근을 위한 정적 함수
	static ACPP_AIRaceManager* GetInstance(UWorld* World);

	// AI 컨트롤러가 호출할 데이터 제공 함수
	int32 GetRankOfVehicle(AActor* Vehicle);
	float GetDistanceOfVehicle(AActor* Vehicle);
	float GetLeadDistance() const { return LeadDistance; }
	int32 GetTotalRacers() const { return RacerTable.Num(); }

	// 디버그용 순위 출력
	void PrintCurrentRankings();

private:
	/** 주기적으로 순위를 갱신하는 핵심 로직 */
	void UpdateRaceData();

	UPROPERTY()
	TArray<FRacerInfo> RacerTable;

	UPROPERTY()
	class ACPP_Road* TargetRoad;

	float LeadDistance = 0.f;
	FTimerHandle UpdateTimerHandle;

	UPROPERTY(EditAnywhere, Category = "RaceSettings")
	float UpdateInterval = 0.1f;
	
	UFUNCTION(BlueprintCallable)
	AActor* GetVehicleByRank(int32 Rank);
	
	float LastLogTime = 0.0f;
};