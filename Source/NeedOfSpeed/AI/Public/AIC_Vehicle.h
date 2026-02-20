// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Vehicle.generated.h"

/**
 * 
 */

class ACPP_Road;

UCLASS()
class NEEDOFSPEED_API AAIC_Vehicle : public AAIController
{
	GENERATED_BODY()

public:
	AAIC_Vehicle();

protected:
	virtual void BeginPlay() override;

private:
	virtual void Tick(float DeltaTime) override;

public:
	FVector SteerTarget;
	
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	APawn* ControllerVehicle;

	UPROPERTY()
	ACPP_Road* Road;

	UFUNCTION(BlueprintCallable, Category = "AI")
	float CalculateSteering();
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	float CalculateTopSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	void CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake);
};
