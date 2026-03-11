// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GasStationRoofActor.h"
#include "GasStationExplosionActor.generated.h"

class UStaticMeshComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<AGasStationRoofActor> RoofActor;

	UFUNCTION(BlueprintCallable, Category = "Explosion")
	void TriggerExplosion();

private:
	bool bHasExploded = false;
};
