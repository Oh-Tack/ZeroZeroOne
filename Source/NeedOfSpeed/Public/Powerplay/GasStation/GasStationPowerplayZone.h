// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Powerplay/GasStation/GasStationExplosionActor.h"
#include "GameFramework/Actor.h"
#include "GasStationPowerplayZone.generated.h"

UCLASS()
class NEEDOFSPEED_API AGasStationPowerplayZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGasStationPowerplayZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> ZoneBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GasStation")
	TObjectPtr<AGasStationExplosionActor> TargetGasStation;

private:
	bool bPlayerInZone = false;
	bool bTriggered = false;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void OnEPressed();

};
