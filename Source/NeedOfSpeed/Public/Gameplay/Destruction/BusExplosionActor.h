// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "BusExplosionActor.generated.h"

class UStaticMeshComponent;
class UParticleSystem;

UCLASS()
class NEEDOFSPEED_API ABusExplosionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABusExplosionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 버스 static mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BusMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	TObjectPtr<UParticleSystem> ExplosionParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	TObjectPtr<UNiagaraSystem> ExplosionNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	FVector ParticleSpawnOffset = FVector(0.0f, 0.0f, 0.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	FVector ParticleScale = FVector(3.0f, 3.0f, 3.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|Timing")
	float LaunchDelay = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|LinearImpulse")
	FVector LaunchDirection = FVector(0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|LinearImpulse")
	float LinearImpulseStrength = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|AngularImpulse")
	FVector AngularImpulseDegrees = FVector(-600.f, 0.f, 0.f);

	UFUNCTION(BlueprintCallable, Category = "Explosion")
	void TriggerExplosion();

private:
	void ExecuteLaunch();

	bool bHasExploded = false;
};
