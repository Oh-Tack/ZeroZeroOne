// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerDestructionActor.generated.h"

UCLASS()
class NEEDOFSPEED_API ATowerDestructionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATowerDestructionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category="Destruction")
	UGeometryCollectionComponent* GCComp;
	
	UFUNCTION()
	void onChaosHit(const FChaosPhysicsCollisionInfo& Info);
	
	void ApplyStrainAt(const FVector& WorldPos);
	
	bool bBrokenOnHit = false;
	
	// 튜닝 값
	UPROPERTY(EditAnywhere, Category="Destruction|Tuning")
	float InitialImpulse = 450000.f;
	
	UPROPERTY(EditAnywhere, Category="Destruction|Tuning")
	float BreakImpulseThreshold = 100000.f;
	
	UPROPERTY(EditAnywhere, Category="Destruction|Tuning")
	float StrainRadius = 2500.f;
	
	UPROPERTY(EditAnywhere, Category="Destruction|Tuning")
	float StrainMagnitude = 150000.f;
};
