// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "TaxiExplosionActor.generated.h"

class UStaticMeshComponent;
class UParticleSystem;

UCLASS()
class NEEDOFSPEED_API ATaxiExplosionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATaxiExplosionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// 택시 static mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> TaxiMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	TObjectPtr<UParticleSystem> ExplosionParticle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	TObjectPtr<UNiagaraSystem> ExplosionNiagara;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	FVector ParticleSpawnOffset = FVector(0.0f, 0.0f, 0.0f);
	
	// 이펙트 후 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|Timing")
	float LaunchDelay = 0.2f;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|LinearImpulse")
	FVector LaunchDirection = FVector(1.f, 0.f, 1.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|LinearImpulse")
	float LinearImpulseStrength = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|AngularImpulse")
	FVector AngularImpulseDegrees = FVector(-400.f, 0.f, 0.f);
	
	// 켜면 LaunchDirection 무시하고 가장 가까운 AI 차 방향으로 날아감
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|LinearImpulse")
	bool bLaunchTowardNearestAI = true;
	
	UFUNCTION(BlueprintCallable, Category = "Explosion")
	void TriggerExplosion();

private:
	void ExecuteLaunch();
	
	bool bHasExploded = false;
	FVector ComputedLaunchDirection;
};
