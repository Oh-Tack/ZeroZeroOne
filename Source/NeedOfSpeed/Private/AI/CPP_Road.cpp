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

	// 스플라인 시작점으로부터 떨어진 거리
	float MoveToSpline = Spline->GetDistanceAlongSplineAtLocation(AILocation, ESplineCoordinateSpace::World);

	FVector Loc, RVec;
	if (MoveToSpline + GetComponentLocation > Spline->GetSplineLength())	// 한 바퀴를 돌 때 예외 처리
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
		Loc = Spline->GetLocationAtDistanceAlongSpline(	// 직선 거리가 아닌 스플라인 곡선을 따라간 위치
			MoveToSpline + GetComponentLocation,
			ESplineCoordinateSpace::World);

		RVec = Spline->GetRightVectorAtDistanceAlongSpline( // 직선 거리가 아닌 스플라인 곡선을 따라간 후 도로의 진향 방향(회전 값)
			MoveToSpline + GetComponentLocation,
			ESplineCoordinateSpace::World);
	}

	RVec = SideOfRoad ? RVec * 200 : RVec * -200;
	LocationToSteerTo = Loc + RVec;
}
