#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "UIn_isVehicle.h"
#include "Components/SplineComponent.h"
#include "CPP_AI_McLaren.generated.h"

class ACPP_Road;
class USplineComponent;
class USceneComponent;
class UBoxComponent;

UCLASS()
class NEEDOFSPEED_API ACPP_AI_McLaren : public AWheeledVehiclePawn, public IUIn_isVehicle
{
	GENERATED_BODY()

public:
	ACPP_AI_McLaren();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY()
	class ACPP_AI_McLaren* CachedAIVehicle;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetThrottle_Implementation(float Throttle) override;
	virtual void SetSteering_Implementation(float Steering) override;
	virtual void SetBrake_Implementation(float Brake) override;
	virtual void GetCollisionBoxes_Implementation(UBoxComponent*& FrontBox, UBoxComponent*& LeftBox, UBoxComponent*& RightBox) override;
	virtual FVector GetFrontOfCar_Implementation() override;
	virtual float GetCurrentSpeed_Implementation() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	ACPP_Road* Road;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	USplineComponent* RoadSpline;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	USceneComponent* Sensor_F;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	UBoxComponent* FrontViewBox;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	UBoxComponent* LeftSideBox;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	UBoxComponent* RightSideBox;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
	float Max_Speed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
	float Min_Speed;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
	float Angle;
	
	// 충돌 처리 함수
	UFUNCTION()
	void OnVehicleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 리스폰 로직
	void RespawnVehicle();
	float RespawnLaneOffset = 0.f;
	
	// ===== Impact =====
	float CalculateImpactScore(
		const FVector& NormalImpulse,
		UPrimitiveComponent* OtherComp);

	void ApplyImpactSpin(
		const FVector& NormalImpulse,
		float ImpactScore);
	
	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TObjectPtr<class UNiagaraSystem> DestroyEffect;  // 파괴 이펙트
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> DestroySound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UMaterialInstanceDynamic> BrakeMID;
	
	float CurrentBrakeAmount = 0.f;
	float CurrentBrakeIntensity = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	float LaunchMultiplier = 5.f; // 충격으로 날아가는 힘 조절

	void IgnoreVehicleCollision(bool bIgnore);

	// ===== State =====
	bool bCollisionDisabled = false;

	FTimerHandle RespawnTimer;
	
	bool bDestroyCar = false;
	
	UPROPERTY()
	float StuckTimer = 0.f;

	UPROPERTY(EditAnywhere)
	float MaxStuckTime = 5.0f; // 2초 이상 멈춰있으면 꼈다고 판단
	
protected:
	bool bIsRespawning = false;
};
