// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	FVector ParticleSpawnOffset = FVector(0.0f, 0.0f, 0.0f);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|RadialImpulse")
	FVector ExplosionOriginOffset = FVector(0.f, 300.f, -200.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|RadialImpulse")
	float ExplosionRadius = 800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|RadialImpulse")
	float RadialImpulseStrength = 800000.f;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|LinearImpulse")
	FVector LaunchDirection = FVector(0.f, -1.f, 1.5f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|LinearImpulse")
	float LinearImpulseStrength = 600000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Tuning|AngularImpulse")
	FVector AngularImpulseDegrees = FVector(0.f, 0.f, 36000.f);
	
	UFUNCTION(BlueprintCallable, Category = "Explosion")
	void TriggerExplosion();

private:
	bool bHasExploded = false;
};
