// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GasStationRoofActor.generated.h"

class UParticleSystem;

UCLASS()
class NEEDOFSPEED_API AGasStationRoofActor : public AActor
{
	GENERATED_BODY()

public:
	AGasStationRoofActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGeometryCollectionComponent> RoofGC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UParticleSystem> ExplosionParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraSystem> ExplosionNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FVector ParticleSpawnOffset = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FVector ParticleScale = FVector(3.0f, 3.0f, 3.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float DestructionStrain = 600000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float DestructionRadius = 99999.f;

	UFUNCTION(BlueprintCallable, Category = "Destruction")
	void TriggerDestruction();
	
	
	// UPROPERTY()
	// TSubclassOf<AActor> DestroyerActor;

private:
	UFUNCTION()
	void ApplyStrain();
};
