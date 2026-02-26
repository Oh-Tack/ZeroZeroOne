// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/AI/CPP_Road.h"
#include "Components/SplineComponent.h"

// Sets default values
ACPP_Road::ACPP_Road()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));

	Spline->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACPP_Road::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACPP_Road::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPP_Road::GetClosestLocationToPath_Implementation(FVector AILocation, float GetComponentLocation, float SideOfRoad,
                                                        FVector& LocationToSteerTo)
{
	if (!Spline)
	{
		return;
	}

	float MoveToSpline = Spline->GetDistanceAlongSplineAtLocation(AILocation, ESplineCoordinateSpace::World);

	FVector Loc, RVec;
	if (MoveToSpline + GetComponentLocation > Spline->GetSplineLength())
	{
		Loc = Spline->GetLocationAtDistanceAlongSpline(
			(MoveToSpline + GetComponentLocation) - Spline->GetSplineLength(),
			ESplineCoordinateSpace::World);

		RVec = Spline->GetRightVectorAtDistanceAlongSpline(
			(MoveToSpline + GetComponentLocation) - Spline->GetSplineLength(),
			ESplineCoordinateSpace::World);
	}
	else
	{
		Loc = Spline->GetLocationAtDistanceAlongSpline(
			MoveToSpline + GetComponentLocation,
			ESplineCoordinateSpace::World);

		RVec = Spline->GetRightVectorAtDistanceAlongSpline(
			MoveToSpline + GetComponentLocation,
			ESplineCoordinateSpace::World);
	}

	RVec = SideOfRoad ? RVec * 700 : RVec * -700;
	LocationToSteerTo = Loc + RVec;
}
