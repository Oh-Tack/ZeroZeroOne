// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_LearningVehicle.generated.h"

/**
 * 
 */
UCLASS()
class NEEDOFSPEED_API AAIC_LearningVehicle : public AAIController
{
	GENERATED_BODY()

public:
	AAIC_LearningVehicle();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	public:
		UFUNCTION(BlueprintCallable)
		void ApplyAction(float Steering, float Throttle, float Brake);
	
	UFUNCTION(BlueprintCallable)
	void ResetEpisode();
	
	FORCEINLINE APawn* GetVehiclePawn() const { return VehiclePawn; }
	
private:
	UPROPERTY()
	APawn* VehiclePawn;
};
