// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIn_isVehicle.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUIn_isVehicle : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NEEDOFSPEED_API IUIn_isVehicle
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Vehicle")
	void SetThrottle(float Throttle);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Vehicle")
	void SetSteering(float Steering);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Vehicle")
	void SetBrake(float Brake);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Vehicle")
	FVector GetFrontOfCar();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Vehicle")
	float GetCurrentSpeed();
};
