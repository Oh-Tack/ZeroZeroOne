// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "AI/UIn_isVehicle.h"
#include "RacingCar.generated.h"

/**
 * 
 */
UCLASS()
class NEEDOFSPEED_API ARacingCar : public AWheeledVehiclePawn, public IUIn_isVehicle
{
	GENERATED_BODY()
	
public:
	ARacingCar();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* IA_Throttle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* IA_Brake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* IA_Steer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* IA_Drift;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bDriftKeyPressed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerPlay")
	float PowerPlayGauge;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerPlay")
	bool bISDrifting;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerPlay")
	float DriftGaugeRate = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerPlay")
	float DriftExitBoostForce = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category = "PowerPlay")
	float BoostConsumptionRate = 1.0f; // 초당 1.0씩 소모 (3.0 만점이면 3초 유지)

	UPROPERTY(EditAnywhere, Category = "PowerPlay")
	float BoostAccelerationForce = 5000.0f; // 부스트 가속 세기

	UPROPERTY(BlueprintReadOnly, Category = "PowerPlay")
	bool bIsBoosting = false; // 현재 부스트 중인지 여부 (위젯 연결용)
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing")
	int32 CurrentRank = 1; // 현재 순위
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Racing")
	int32 TotalParticipants = 5; // 총 참가자 수

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing")
	int32 CurrentLap = 1; // 현재 바퀴 수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Racing")
	int32 TotalLaps = 3; // 총 바퀴 수
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyCar")
	class UChaosWheeledVehicleMovementComponent* ChaosMovement;
	
	
	virtual void Tick(float DeltaTime) override;
	
	// 드리프트 시 사용할 마찰력 계수
	UPROPERTY(EditAnywhere, Category = "Movement | Drift")
	float DriftFrictionScale = 0.05f;

	// 기본 마찰력 계수
	float DefaultFrictionScale = 1.0f;
	
	bool bWasDriftingLastFrame;
	
	
    
	// 입력 처리 함수 
	void Throttle(const struct FInputActionValue& Value);
	void Brake(const struct FInputActionValue& Value);
	void Steer(const struct FInputActionValue& Value);
	void StartDrift();
	void StopDrift();
	// void DriftingPhysics(bool bIsDrfting);
	// 드리프트 탈출 부스터 함수
	void ApplyExitBoost();
	

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
 