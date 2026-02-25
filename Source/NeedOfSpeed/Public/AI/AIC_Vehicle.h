// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Vehicle.generated.h"

class ACPP_Road;
class UBoxComponent;

/**
 * AI Vehicle Controller
 */
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
	// -------------------------
	// Steering & Control
	// -------------------------
	FVector SteerTarget;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	APawn* ControllerVehicle;

	UPROPERTY()
	ACPP_Road* Road;

	UPROPERTY()
	UBoxComponent* FrontBox;
	UPROPERTY()
	UBoxComponent* LeftBox;
	UPROPERTY()
	UBoxComponent* RightBox;

	UPROPERTY()
	AActor* VehicleInFrontActor;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bOverrideTopSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float OverrideTopSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int SideOfRoad;
	
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	int OriginalSideOfRoad;

	// -------------------------
	// Overtake
	// -------------------------
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsOvertaking;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	float OvertakeStartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float OvertakeDuration;

	UPROPERTY()
	TArray<AActor*> AllVehicles;

	// -------------------------
	// Functions
	// -------------------------
	UFUNCTION(BlueprintCallable, Category = "AI")
	float CalculateSteering();

	UFUNCTION(BlueprintCallable, Category = "AI")
	float CalculateTopSpeed();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void CheckForOvertakes();

	UFUNCTION(BlueprintCallable, Category = "AI")
	int GetRaceRank();

	UFUNCTION(BlueprintCallable, Category = "AI")
	int GetTotalRacers();

private:
	// Helper functions
	void HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors, const TArray<AActor*>& RightActors);
	float HandleTargetSpeed(bool bVehicleInFront, AActor* FirstActor);
};
