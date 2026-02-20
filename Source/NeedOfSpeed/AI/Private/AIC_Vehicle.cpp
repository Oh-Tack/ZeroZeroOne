// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/AI/Public/CPP_Road.h"
#include "NeedOfSpeed/AI/Public/AIC_Vehicle.h"
#include "NeedOfSpeed/AI/Public/UIn_isVehicle.h"
#include "Kismet/KismetMathLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "NeedOfSpeed/AI/Public/CPP_AI_McLaren.h"

AAIC_Vehicle::AAIC_Vehicle()
{
}

void AAIC_Vehicle::BeginPlay()
{
	Super::BeginPlay();

	ControllerVehicle = GetPawn();

	Road = Cast<ACPP_Road>(
		UGameplayStatics::GetActorOfClass(
			GetWorld(),
			ACPP_Road::StaticClass()
		)
	);
}

void AAIC_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ControllerVehicle)
	{
		return;
	}
	float Throttle;
	float Brake;
	IUIn_isVehicle::Execute_SetSteering(ControllerVehicle, CalculateSteering());

	CalculateThrottleBrake(CalculateTopSpeed(), Throttle, Brake);
	IUIn_isVehicle::Execute_SetThrottle(ControllerVehicle, Throttle);
	IUIn_isVehicle::Execute_SetBrake(ControllerVehicle, Brake);

	UE_LOG(LogTemp, Warning, TEXT("Throttle: %.1f	|	Brake: %.1f"), Throttle, Brake);
}

float AAIC_Vehicle::CalculateSteering()
{
	auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
	if (!Road || !ControllerVehicle || !AIVehicle)
	{
		return 0;
	}

	float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);

	float ForwardVision = UKismetMathLibrary::MapRangeClamped(
		Curr,
		AIVehicle->Min_Speed,
		AIVehicle->Max_Speed,
		4000.f,
		12000.f
	);

	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, SteerTarget);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(start, SteerTarget);
	FRotator ActorRot = ControllerVehicle->GetActorRotation();

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ActorRot);

	return UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -10.0, 10.0, -1, 1);
}

float AAIC_Vehicle::CalculateTopSpeed()
{
	auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
	if (!Road || !ControllerVehicle || !AIVehicle)
	{
		return 0.0f;
	}

	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
	Road->GetClosestLocationToPath_Implementation(start, 1500, SteerTarget);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(start, SteerTarget);
	FRotator ActorRot = ControllerVehicle->GetActorRotation();

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ActorRot);

	float TopSpeed = UKismetMathLibrary::MapRangeClamped(
		abs(DeltaRot.Yaw),
		0.0,
		AIVehicle->Angle,
		AIVehicle->Max_Speed,
		AIVehicle->Min_Speed
	);
	UE_LOG(LogTemp, Warning, TEXT("Top Speed: %.1f	|	%.1f"), TopSpeed, abs(DeltaRot.Yaw));

	return TopSpeed;
}

void AAIC_Vehicle::CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake)
{
	if (!ControllerVehicle)
		return;

	float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
	float SpeedDiff = Curr - TopSpeed;

	// 목표 속도보다 느리면 Throttle을 부드럽게 증가
	if (SpeedDiff < 0.0f)
	{
		// 속도 차이에 비례해 0~1 보간, 최소 0.5 이상
		Throttle = FMath::Clamp(0.5f + (-SpeedDiff) / 50.0f, 0.5f, 1.0f);
		Brake = 0.0f;
	}
	else
	{
		// 목표 속도를 살짝 초과하면 브레이크를 부드럽게
		Throttle = FMath::Clamp(1.0f - SpeedDiff / 50.0f, 0.0f, 0.05f);
		Brake = FMath::Clamp(SpeedDiff / 50.0f, 0.0f, 1.0f); // 브레이크 최대 50%
	}

	UE_LOG(LogTemp, Warning, TEXT("Speed: %.1f | TopSpeed: %.1f | Throttle: %.2f | Brake: %.2f"), Curr, TopSpeed, Throttle, Brake);
}