// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "NiagaraSystem.h"
#include "Camera/CameraShakeBase.h"
#include "PlaneCrashActor.generated.h"

class UParticleSystem;
class USoundBase;

UENUM(BlueprintType)
enum class EPlaneCrashPhase : uint8
{
	Idle,
	Approaching,  // 스플라인 경로 따라 강하 중
	Sliding,      // 착지 후 슬라이딩
	Impacted,     // 완전 정지
};

UCLASS()
class NEEDOFSPEED_API APlaneCrashActor : public AActor
{
	GENERATED_BODY()

public:
	APlaneCrashActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	// 비행 경로 스플라인 (에디터에서 포인트 배치)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crash")
	TObjectPtr<USplineComponent> FlightSpline;

	// 비행기 액터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash")
	TObjectPtr<AActor> PlaneActor;

	// 강하 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	float ApproachDuration = 8.f;
	
	// 메시 forward 축 보정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash")
	FRotator MeshRotationOffset = FRotator(0.f, -90.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	float NoseDownPitch = -8.f;
	
	// 착지 후 슬라이딩 속도 (cm/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sliding")
	float SlidingSpeed = 2000.f;

	// 착지 후 슬라이딩 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sliding")
	float SlidingDuration = 3.f;

	// 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TObjectPtr<UParticleSystem> ImpactParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TObjectPtr<UNiagaraSystem> ImpactNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TObjectPtr<UNiagaraSystem> TrailNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TObjectPtr<UNiagaraSystem> SlidingSparkNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	FVector ImpactParticleOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	FVector ImpactParticleScale = FVector(5.f, 5.f, 5.f);

	// 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sound")
	TObjectPtr<USoundBase> ApproachSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sound")
	TObjectPtr<USoundBase> ImpactSound;

	// 카메라 쉐이크
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TSubclassOf<UCameraShakeBase> ImpactCameraShake;

	UFUNCTION(BlueprintImplementableEvent, Category = "Crash")
	void OnApproachStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Crash")
	void OnImpact();

	UFUNCTION(BlueprintCallable, Category = "Crash")
	void TriggerCrash();

private:
	EPlaneCrashPhase CurrentPhase = EPlaneCrashPhase::Idle;
	float ApproachElapsed = 0.f;
	float SlidingElapsed = 0.f;
	FVector SlideDirection;
	FRotator LandingRotation;
	FVector LandingLocation;

	void ExecuteImpact();
	void ExecuteSliding(float DeltaTime);
};
