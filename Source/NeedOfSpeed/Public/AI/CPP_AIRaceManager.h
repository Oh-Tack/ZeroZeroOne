#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_AIRaceManager.generated.h"

USTRUCT(BlueprintType) // 블루프린트 타입으로 만들기
struct FRacerInfo
{
	GENERATED_BODY()

public: // 👈 반드시 public

	UPROPERTY(BlueprintReadWrite)
	AActor* Vehicle = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float DistanceAlongSpline = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float PreviousDistance = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 Lap = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Rank = 0;

	UPROPERTY(BlueprintReadWrite)
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
	
	UPROPERTY()
	TArray<FRacerInfo> RacerTable;
	
	

private:
	/** 주기적으로 순위를 갱신하는 핵심 로직 */
	void UpdateRaceData();

	UPROPERTY()
	class ACPP_Road* TargetRoad;

	float LeadDistance = 0.f;
	FTimerHandle UpdateTimerHandle;

	UPROPERTY(EditAnywhere, Category = "RaceSettings")
	float UpdateInterval = 0.1f;
	
	UFUNCTION(BlueprintCallable)
	AActor* GetVehicleByRank(int32 Rank);
	
	float LastLogTime = 0.0f;
	
	UPROPERTY()
	TMap<int32, int32> RespawnSlotCounter;

public:
	int32 AcquireRespawnSlot(float Distance);
	
	UFUNCTION(BlueprintCallable, Category="Race")
	FRacerInfo GetRacerInfo(AActor* Vehicle) const;
};