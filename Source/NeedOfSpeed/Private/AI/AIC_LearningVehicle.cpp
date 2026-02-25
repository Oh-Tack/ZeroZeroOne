// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/AI/AIC_LearningVehicle.h"
#include "NeedOfSpeed/Public/AI/UIn_isVehicle.h"

AAIC_LearningVehicle::AAIC_LearningVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAIC_LearningVehicle::BeginPlay()
{
	Super::BeginPlay();
	VehiclePawn = GetPawn();
}

void AAIC_LearningVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ApplyAction(0.f, 0.6f, 0.f);
}

void AAIC_LearningVehicle::ApplyAction(float Steering, float Throttle, float Brake)
{
	if (!VehiclePawn) return;
	
	// 안전 Clamp (ML 초기엔 값 튐)
	Steering = FMath::Clamp(Steering, -1.0f, 1.0f);
	Throttle = FMath::Clamp(Throttle, 0.0f, 1.0f);
	Brake = FMath::Clamp(Brake, 0.0f, 1.0f);
	
	IUIn_isVehicle::Execute_SetSteering(VehiclePawn, Steering);
	IUIn_isVehicle::Execute_SetThrottle(VehiclePawn, Throttle);
	IUIn_isVehicle::Execute_SetBrake(VehiclePawn, Brake);
}

void AAIC_LearningVehicle::ResetEpisode()
{
	if (!VehiclePawn) return;
	
	VehiclePawn->SetActorLocation(FVector::ZeroVector);
	VehiclePawn->SetActorRotation(FRotator::ZeroRotator);
	
	IUIn_isVehicle::Execute_SetThrottle(VehiclePawn, 0.f);
	IUIn_isVehicle::Execute_SetBrake(VehiclePawn, 1.f);
}
