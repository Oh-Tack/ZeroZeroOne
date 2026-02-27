// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "RacingCar.generated.h"

/**
 * 
 */
UCLASS()
class NEEDOFSPEED_API ARacingCar : public AWheeledVehiclePawn
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
	float DriftExitBoostForce = 800.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup") // 앞바퀴
	int32 FrontWheelClass = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup") // 뒷바퀴
	int32 RearWheelClass = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyCar")
	class UChaosWheeledVehicleMovementComponent* ChaosMovement;
	
	virtual void Tick(float DeltaTime) override;
	
	// 드리프트 시 사용할 마찰력 계수
	UPROPERTY(EditAnywhere, Category = "Movement | Drift")
	float DriftFrictionScale = 0.5f;

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
