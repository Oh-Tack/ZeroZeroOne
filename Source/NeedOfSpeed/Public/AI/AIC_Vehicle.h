// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Vehicle.generated.h"

// 전방 선언 (컴파일 속도 최적화)
class ACPP_Road;
class ACPP_AI_McLaren;
class UBoxComponent;

/**
 * 레이싱 AI 컨트롤러 클래스
 */
UCLASS()
class NEEDOFSPEED_API AAIC_Vehicle : public AAIController
{
	GENERATED_BODY()

public:
	AAIC_Vehicle();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// -------------------------
	// 주요 AI 로직 함수
	// -------------------------

	/** 현재 속도와 도로 곡률에 따른 조향값 계산 */
	float CalculateSteering();

	/** 전방 코너 및 상황에 따른 목표 최고 속도 계산 */
	float CalculateTopSpeed();

	/** 측면 장애물 발생 시 긴급 회피 처리 */
	void HandleEmergencyEvade(float DeltaTime);

	/** 전방 차량 감지 및 추월 점수 계산 */
	void CheckForOvertakes();

	/** 실질적인 차선 변경 결정 및 안전 검사 */
	void HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors, const TArray<AActor*>& RightActors);

	/** 계산된 목표 속도에 맞춰 스로틀/브레이크 제어 */
	void HandleTargetSpeed(float TargetTopSpeed, float DeltaTime, float Steering);

public:
	// -------------------------
	// 레이스 정보 인터페이스
	// -------------------------

	UFUNCTION(BlueprintCallable, Category = "Race")
	int GetRaceRank();

	UFUNCTION(BlueprintCallable, Category = "Race")
	int GetTotalRacers();

protected:
	// -------------------------
	// 참조 변수
	// -------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Reference")
	APawn* ControllerVehicle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Reference")
	ACPP_AI_McLaren* CachedAIVehicle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Reference")
	ACPP_Road* Road;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Reference")
	TArray<AActor*> AllVehicles;

	// -------------------------
	// 주행 상태 변수
	// -------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
	float TargetSideOfRoad; // 0.0(좌측) ~ 1.0(우측)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Movement")
	float CurrentSideOfRoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
	float LaneChangeSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Movement")
	float CurrentLaneChangeSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Movement")
	FVector SteerTarget;

	// -------------------------
	// 추월 및 모드 상태
	// -------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bIsOvertaking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float OvertakeDuration;

	float OvertakeStartTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bEmergencyBrake;

	float LastLogTime;

	// -------------------------
	// 충돌 감지 컴포넌트 포인터 (인터페이스를 통해 받아옴)
	// -------------------------

	UPROPERTY()
	UBoxComponent* FrontBox;

	UPROPERTY()
	UBoxComponent* LeftBox;

	UPROPERTY()
	UBoxComponent* RightBox;

	/** 현재 추월 대상으로 선정된 차량 */
	UPROPERTY(VisibleAnywhere, Category = "AI|State")
	AActor* VehicleInFrontActor;
};