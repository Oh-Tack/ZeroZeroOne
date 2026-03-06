#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Vehicle.generated.h"

// 전방 선언 (컴파일 속도 향상 및 의존성 감소)
class UBoxComponent;
class ACPP_Road;

UCLASS()
class NEEDOFSPEED_API AAIC_Vehicle : public AAIController
{
    GENERATED_BODY()

public:
    AAIC_Vehicle();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // -------------------------
    // 주요 로직 함수
    // -------------------------
    float CalculateSteering();
    float CalculateTopSpeed();
    void CheckForOvertakes();
    void HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors, const TArray<AActor*>& RightActors);
    void HandleEmergencyEvade(float DeltaTime);
    float HandleTargetSpeed();
    void CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake);

    // 순위 관련 유틸리티
    UFUNCTION(BlueprintCallable, Category = "AI|Race")
    int32 GetRaceRank();

    UFUNCTION(BlueprintCallable, Category = "AI|Race")
    int32 GetTotalRacers();

    void LogRaceRankings();
    void CheckForObstacles(float DeltaTime);

public:
    // -------------------------
    // 참조 및 컴포넌트 (UPROPERTY 추가로 GC 보호)
    // -------------------------
    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal")
    AActor* ControllerVehicle;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal")
    ACPP_Road* Road;

    // 가비지 컬렉션으로부터 보호하기 위해 TArray에도 UPROPERTY()를 붙여주는 것이 좋습니다.
    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal")
    TArray<AActor*> AllVehicles;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal")
    UBoxComponent* FrontBox;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal")
    UBoxComponent* LeftBox;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal")
    UBoxComponent* RightBox;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Internal")
    AActor* VehicleInFrontActor;

    // -------------------------
    // 상태 및 제어 변수
    // -------------------------
    UPROPERTY(BlueprintReadOnly, Category = "AI|Movement")
    FVector SteerTarget;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
    float TargetSideOfRoad;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Movement")
    float CurrentSideOfRoad;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
    float LaneChangeSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Movement")
    float CurrentLaneChangeSpeed;

    // 상태 플래그
    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    bool bIsOvertaking;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    bool bEmergencyBrake; // 양쪽 차선이 막혔을 때 사용하는 플래그

    float OvertakeStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Overtake")
    float OvertakeDuration;

    // -------------------------
    // 디버그 및 기타
    // -------------------------
    float LastLogTime;
};