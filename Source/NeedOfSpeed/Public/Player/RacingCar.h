// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "AI/UIn_isVehicle.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
	float DriftExitBoostForce = 500.0f;
	
	UPROPERTY(EditAnywhere, Category = "PowerPlay")
	float BoostConsumptionRate = 1.0f; // 초당 1.0씩 소모 (3.0 만점이면 3초 유지)

	UPROPERTY(EditAnywhere, Category = "PowerPlay")
	float BoostAccelerationForce = 3000.0f; // 부스트 가속 세기

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	TArray<class UPointLightComponent*> BrakeLights;

	// 라이트 밝기 설정 변수
	UPROPERTY(EditAnywhere, Category = "Light")
	float NormalBrakeIntensity = 10.0f; // 평소 밝기

	UPROPERTY(EditAnywhere, Category = "Light")
	float ActiveBrakeIntensity = 100.0f; // 브레이크 밟았을 때 밝기
	
	
	// 후방 주시용 스프링 암, 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* RearSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* RearCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FrontCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* IA_LookBack;
	

	// 카메라 전환 함수
	void StartLookBack();
	void StopLookBack();
	
	virtual void Tick(float DeltaTime) override;
	
	// 드리프트 시 사용할 마찰력 계수
	UPROPERTY(EditAnywhere, Category = "Movement | Drift")
	float DriftFrictionScale = 0.05f;

	// 기본 마찰력 계수
	float DefaultFrictionScale = 1.0f;
	
	bool bWasDriftingLastFrame;
	
	
	UPROPERTY(EditAnywhere, Category = "PowerPlay|Camera")
	TSubclassOf<class UCameraShakeBase> BoostShakeClass; // 부스트 쉐이크 블루프린트

	UPROPERTY(EditAnywhere, Category = "PowerPlay|Camera")
	TSubclassOf<class UCameraShakeBase> HighSpeedShakeClass; // 고속 주행 쉐이크 블루프린트

	// 현재 재생 중인 쉐이크를 추적하는 포인터
	UPROPERTY()
	class UCameraShakeBase* ActiveBoostShake;

	UPROPERTY()
	class UCameraShakeBase* ActiveHighSpeedShake;

	// FOV (시야각) 설정
	UPROPERTY(EditAnywhere, Category = "PowerPlay|Camera")
	float NormalFOV = 90.0f;

	UPROPERTY(EditAnywhere, Category = "PowerPlay|Camera")
	float BoostFOV = 100.0f;
	
	// 충돌 카메라
	UPROPERTY(EditAnywhere, Category = "PowerPlay|Camera")
	TSubclassOf<class UCameraShakeBase> ImpactShakeClass; // 충돌용 단발성 쉐이크

	// 충돌이벤트
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	class UNiagaraComponent* DriftSmokeLeft;  // 왼쪽 뒷바퀴 연기

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	class UNiagaraComponent* DriftSmokeRight; // 오른쪽 뒷바퀴 연기
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	class UAudioComponent* BoostAudio;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	class UAudioComponent* DriftAudio;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	class UAudioComponent* EngineAudio;
    
	// 입력 처리 함수 
	void Throttle(const struct FInputActionValue& Value);
	void Brake(const struct FInputActionValue& Value);
	void Steer(const struct FInputActionValue& Value);
	void StartDrift();
	void StopDrift();
	// void DriftingPhysics(bool bIsDrfting);
	// 드리프트 탈출 부스터 함수
	void ApplyExitBoost();
	
private:
	const FName SkidIntensityParam = FName(TEXT("SkidIntensity"));

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:
	// 결승선을 통과 함수
	UFUNCTION(BlueprintCallable, Category = "Racing")
	void PassFinishLine();
    
	// 중복방지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing")
	bool bCanLap = true;
	
	void UpdateRaceRank();

	// 트랙의 스플라인을 저장할 포인터
	UPROPERTY()
	class USplineComponent* TrackSpline;

	// 타이머 핸들 (매 프레임 계산하면 무거우니 0.2초마다 계산)
	FTimerHandle RankTimerHandle;
};
 