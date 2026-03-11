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
	Approaching,
	Sliding,
	Impacted,
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crash")
	TObjectPtr<USplineComponent> FlightSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash")
	TObjectPtr<AActor> PlaneActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	float ApproachDuration = 8.f;

	// 메시 앞방향 보정 (Yaw 위주)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash")
	FRotator MeshRotationOffset = FRotator(0.f, -90.f, 0.f);

	// 시작 시 기수 하향 각도 (앞부분 숙이기)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	float NoseDownPitch = -8.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	int32 ImpactSplinePointIndex = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sliding")
	float SlidingSpeed = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sliding")
	float SlidingDuration = 3.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sound")
	TObjectPtr<USoundBase> ApproachSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sound")
	float ApproachSoundDelay = 1.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Sound")
	TObjectPtr<USoundBase> ImpactSound;

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

	FVector SlideDirection = FVector::ZeroVector;
	FRotator LandingRotation = FRotator::ZeroRotator;
	FVector LandingLocation = FVector::ZeroVector;

private:
	void SpawnImpactEffects();
	void ExecuteImpact();
	void ExecuteSliding(float DeltaTime);

	void AlignPlaneToSplineStart(bool bApplyNoseDown);
	FRotator MakeApproachRotation(const FVector& Direction, float Alpha) const;
	
	bool bEffectsSpawned = false;
};