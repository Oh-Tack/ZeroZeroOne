// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Powerplay/Taxi/TaxiExplosionActor.h"
#include "TaxiPowerplayZone.generated.h"

UCLASS()
class NEEDOFSPEED_API ATaxiPowerplayZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATaxiPowerplayZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UBoxComponent> ZoneBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Taxis")
	TArray<TObjectPtr<ATaxiExplosionActor>> Taxis;
	
private:
	bool bPlayerInZone = false;
	int32 CurrentTaxiIndex = 0;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void OnEPressed();
};
