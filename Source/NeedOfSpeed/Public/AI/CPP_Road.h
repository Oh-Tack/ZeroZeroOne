// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include  "UIn_isPath.h"
#include "GameFramework/Actor.h"
#include "CPP_Road.generated.h"

class USplineComponent;

UCLASS()
class NEEDOFSPEED_API ACPP_Road : public AActor, public IUIn_isPath
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPP_Road();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Components")
	USplineComponent* Spline;

	virtual void GetClosestLocationToPath_Implementation(
		FVector AILocation,
		float GetComponentLocation,
		float SideOfRoad,
		FVector& LocationToSteerTo
	) override;
};
