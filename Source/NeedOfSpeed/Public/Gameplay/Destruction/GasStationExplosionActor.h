// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GasStationExplosionActor.generated.h"

class UStaticMeshComponent;
class UParticleSystem;

UCLASS()
class NEEDOFSPEED_API AGasStationExplosionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGasStationExplosionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GasStationMesh;

	// 천장 Geometry Collection (Chaos 파괴용) 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGeometryCollectionComponent> RoofGC;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	TObjectPtr<UParticleSystem> ExplosionParticle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	TObjectPtr<UNiagaraSystem> ExplosionNiagara;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	FVector ParticleSpawnOffset = FVector(0.0f, 0.0f, 0.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Effects")
	FVector ParticleScale = FVector(3.0f, 3.0f, 3.0f);
	
	// Chaos 파괴 강도 (클수록 확실히 부서짐)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Chaos")
	float DestructionStrain = 99999.f;

	// 파괴 영향 반경 (천장만 커버하도록 작게 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Chaos")
	float DestructionRadius = 300.f;

	// 액터 원점에서 천장까지의 높이 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Chaos")
	float RoofHeightOffset = 300.f;

	UFUNCTION(BlueprintCallable, Category = "Explosion")
	void TriggerExplosion();

private:
	bool bHasExploded = false;
};
