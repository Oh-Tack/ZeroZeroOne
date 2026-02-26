#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Vehicle.generated.h"

/**
 * 레이싱 AI 컨트롤러 클래스
 */
class UBoxComponent;
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
	/** 조향각 계산 */
	float CalculateSteering();
	
	/** 도로 곡률 등에 따른 목표 최고 속도 계산 */
	float CalculateTopSpeed();

	/** 추월 가능 여부 판단 및 상태 업데이트 */
	void CheckForOvertakes();

	/** 차선 변경 로직 */
	void HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors, const TArray<AActor*>& RightActors);

	/** 돌발 상황 시 긴급 회피 로직 */
	void HandleEmergencyEvade(float DeltaTime);

	/** 상황별 최종 목표 속도 결정 (추월/순위 보정 포함) */
	float HandleTargetSpeed();

	/** 속도 차이에 따른 액셀/브레이크 값 계산 */
	void CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake);

	/** 현재 레이스 순위 가져오기 */
	int32 GetRaceRank();

	/** 전체 참가 차량 수 가져오기 */
	int32 GetTotalRacers();

	/** 순위 로그 출력 */
	void LogRaceRankings();

public:
	// -------------------------
	// 참조 및 컴포넌트
	// -------------------------
	UPROPERTY()
	AActor* ControllerVehicle;

	UPROPERTY()
	class ACPP_Road* Road;

	UPROPERTY()
	TArray<AActor*> AllVehicles;

	// 센서용 콜리전 박스
	UPROPERTY()
	UBoxComponent* FrontBox;

	UPROPERTY()
	UBoxComponent* LeftBox;

	UPROPERTY()
	UBoxComponent* RightBox;

	UPROPERTY()
	AActor* VehicleInFrontActor;

	// -------------------------
	// 상태 및 제어 변수
	// -------------------------
	FVector SteerTarget;
	
	// 차선 관련 (0.0: 왼쪽, 1.0: 오른쪽)
	float TargetSideOfRoad;
	float CurrentSideOfRoad;

	UPROPERTY(EditAnywhere, Category = "AI|Movement")
	float LaneChangeSpeed;

	float CurrentLaneChangeSpeed;

	// 추월 상태 관련
	bool bIsOvertaking;
	float OvertakeStartTime;
	float OvertakeDuration;

	bool bOverrideTopSpeed;

	// 로그용
	float LastLogTime;
};