// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Powerplay/Satellite/SatelliteSignActor.h"
#include "SatellitePoleActor.generated.h"

UCLASS()
class NEEDOFSPEED_API ASatellitePoleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASatellitePoleActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PoleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse")
	TObjectPtr<ASatelliteSignActor> SignActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Pole")
	FVector FallDirection = FVector(1.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Pole")
	float ImpulseStrength = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Sign")
	float SignReleaseDelay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Effects")
	TObjectPtr<UNiagaraSystem> CollapseNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Sound")
	TObjectPtr<USoundBase> CollapseSound;

	UFUNCTION(BlueprintCallable, Category = "Collapse")
	void TriggerCollapse();

private:
	bool bHasCollapsed = false;
};
