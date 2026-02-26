#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_LearningVehicle.generated.h"

class ACPP_Road;
class UBoxComponent;

/**
 * AI 컨트롤러: 앞차 감속, 추월, 충돌 회피 포함
 */
UCLASS()
class NEEDOFSPEED_API AAIC_LearningVehicle : public AAIController
{
    GENERATED_BODY()

public:
    AAIC_LearningVehicle();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // -------------------------
    // Core
    // -------------------------
    APawn* ControllerVehicle;
    ACPP_Road* Road;

    // -------------------------
    // Steering / Spline
    // -------------------------
    FVector SteerTarget;
    float LateralOffset;
    float TargetOffset;
    float OffsetSize = 400.f; // 추월 옆으로 이동 거리

    // -------------------------
    // Overtake / Lane Change
    // -------------------------
    bool bIsOvertaking;
    float OvertakeStartTime;
    float OvertakeDuration;
    bool bOverrideTopSpeed;
    float OverrideTopSpeed;

    // -------------------------
    // Collision Boxes
    // -------------------------
    UBoxComponent* FrontBox;
    UBoxComponent* LeftBox;
    UBoxComponent* RightBox;
    AActor* VehicleInFrontActor;

    // -------------------------
    // Race Info
    // -------------------------
    TArray<AActor*> AllVehicles;

private:
    // -------------------------
    // Core Logic
    // -------------------------
    float CalculateSteering();
    float CalculateTopSpeed();
    void CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake);

    // -------------------------
    // Overtake / Lane Change
    // -------------------------
    void UpdateOvertake(float DeltaTime);

    // -------------------------
    // Front Vehicle Detection
    // -------------------------
    AActor* GetFrontVehicle();

    // -------------------------
    // Race Info
    // -------------------------
    int GetRaceRank();
    int GetTotalRacers();
};