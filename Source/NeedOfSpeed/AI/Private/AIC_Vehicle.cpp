#include "NeedOfSpeed/AI/Public/AIC_Vehicle.h"
#include "NeedOfSpeed/AI/Public/CPP_Road.h"
#include "NeedOfSpeed/AI/Public/UIn_isVehicle.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NeedOfSpeed/AI/Public/CPP_AI_McLaren.h"

AAIC_Vehicle::AAIC_Vehicle()
{
	bIsOvertaking = false;
	OvertakeDuration = 2.0f; // 추월 상태 유지 2초
}

void AAIC_Vehicle::BeginPlay()
{
	Super::BeginPlay();

	ControllerVehicle = GetPawn();

	Road = Cast<ACPP_Road>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ACPP_Road::StaticClass())
	);

	SideOfRoad = CalculateSteering();

	// 경기 시작 시 차량 캐싱
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_AI_McLaren::StaticClass(), AllVehicles);
}

void AAIC_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ControllerVehicle) return;

	CheckForOvertakes();

	IUIn_isVehicle::Execute_SetSteering(ControllerVehicle, CalculateSteering());

	float Throttle, Brake;
	CalculateThrottleBrake(CalculateTopSpeed(), Throttle, Brake);

	IUIn_isVehicle::Execute_SetThrottle(ControllerVehicle, Throttle);
	IUIn_isVehicle::Execute_SetBrake(ControllerVehicle, Brake);
}

// -------------------------
// Steering & Speed
// -------------------------

float AAIC_Vehicle::CalculateSteering()
{
	auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
	if (!Road || !ControllerVehicle || !AIVehicle) return 0;

	float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
	float ForwardVision = UKismetMathLibrary::MapRangeClamped(
		Curr, AIVehicle->Min_Speed, AIVehicle->Max_Speed, 2000.0f, 6000.f
	);

	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, SideOfRoad, SteerTarget);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(start, SteerTarget);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());

	return UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -10.0, 10.0, -1, 1);
}

float AAIC_Vehicle::CalculateTopSpeed()
{
	if (bOverrideTopSpeed) return OverrideTopSpeed;

	auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
	if (!Road || !ControllerVehicle || !AIVehicle) return 0.0f;

	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
	Road->GetClosestLocationToPath_Implementation(start, 1500, SideOfRoad, SteerTarget);

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::FindLookAtRotation(start, SteerTarget),
		ControllerVehicle->GetActorRotation()
	);

	return UKismetMathLibrary::MapRangeClamped(
		abs(DeltaRot.Yaw), 0.0, AIVehicle->Angle, AIVehicle->Max_Speed, AIVehicle->Min_Speed
	);
}

// -------------------------
// Overtake Logic
// -------------------------

void AAIC_Vehicle::CheckForOvertakes()
{
	if (!ControllerVehicle) return;

	IUIn_isVehicle::Execute_GetCollisionBoxes(ControllerVehicle, FrontBox, LeftBox, RightBox);
	if (!FrontBox || !LeftBox || !RightBox) return;

	bOverrideTopSpeed = false;

	TArray<AActor*> FrontActors, LeftActors, RightActors;
	FrontBox->GetOverlappingActors(FrontActors);
	LeftBox->GetOverlappingActors(LeftActors);
	RightBox->GetOverlappingActors(RightActors);

	AActor* FirstActor = FrontActors.Num() > 0 ? FrontActors[0] : nullptr;
	VehicleInFrontActor = FirstActor;
	bool bVehicleInFront = FirstActor != nullptr;

	HandleLaneChange(bVehicleInFront, LeftActors, RightActors);
	float TargetTopSpeed = HandleTargetSpeed(bVehicleInFront, FirstActor);

	// Throttle & Brake
	float Throttle, Brake;
	CalculateThrottleBrake(TargetTopSpeed, Throttle, Brake);

	IUIn_isVehicle::Execute_SetThrottle(ControllerVehicle, Throttle);
	IUIn_isVehicle::Execute_SetBrake(ControllerVehicle, Brake);
}

// -------------------------
// Lane Change
// -------------------------

void AAIC_Vehicle::HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors,
                                    const TArray<AActor*>& RightActors)
{
	bool bOppositeLaneClear = (SideOfRoad == 1) ? (LeftActors.Num() == 0) : (RightActors.Num() == 0);

	if (bVehicleInFront && bOppositeLaneClear && !bIsOvertaking)
	{
		SideOfRoad = (SideOfRoad == 1) ? 0 : 1;
		bIsOvertaking = true;
		OvertakeStartTime = GetWorld()->GetTimeSeconds();

		UE_LOG(LogTemp, Warning, TEXT("[%s] OVERTAKING START"), *ControllerVehicle->GetName());
	}

	// 추월 상태 유지
	if (bIsOvertaking && GetWorld()->GetTimeSeconds() - OvertakeStartTime > OvertakeDuration)
	{
		bIsOvertaking = false;
		UE_LOG(LogTemp, Warning, TEXT("[%s] OVERTAKING END"), *ControllerVehicle->GetName());
	}
}

// -------------------------
// Target Speed
// -------------------------

float AAIC_Vehicle::HandleTargetSpeed(bool bVehicleInFront, AActor* FirstActor)
{
	float TargetTopSpeed = CalculateTopSpeed();

	if (bVehicleInFront && FirstActor)
	{
		float FrontDistance = FVector::Distance(ControllerVehicle->GetActorLocation(), FirstActor->GetActorLocation());
		float FrontSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(FirstActor);

		if (bIsOvertaking)
		{
			TargetTopSpeed = FMath::Max(TargetTopSpeed, FrontSpeed + 50.0f);
		}
		else if (FrontDistance < 500.f)
		{
			TargetTopSpeed = FMath::Min(TargetTopSpeed, FrontSpeed);
		}
	}

	// 꼴등 보정
	int MyRank = GetRaceRank();
	int TotalCars = GetTotalRacers();
	if (MyRank == TotalCars) TargetTopSpeed *= 1.2f;
	else if (MyRank == TotalCars - 1) TargetTopSpeed *= 1.1f;

	return TargetTopSpeed;
}

// -------------------------
// Throttle & Brake
// -------------------------

void AAIC_Vehicle::CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake)
{
	if (!ControllerVehicle) return;

	float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
	float SpeedDiff = Curr - TopSpeed;

	if (SpeedDiff < 0.0f)
	{
		Throttle = FMath::Clamp(0.5f + (-SpeedDiff) / 50.0f, 0.3f, 0.8f);
		Brake = 0.0f;
	}
	else
	{
		Throttle = FMath::Clamp(1.0f - SpeedDiff / 50.0f, 0.0f, 0.5f);
		Brake = FMath::Clamp(SpeedDiff / 50.0f, 0.0f, 1.0f);
	}
}

// -------------------------
// Rank & Vehicle Info
// -------------------------

int AAIC_Vehicle::GetRaceRank()
{
	AllVehicles.Sort([](const AActor& A, const AActor& B)
	{
		return A.GetActorLocation().X > B.GetActorLocation().X;
	});

	for (int i = 0; i < AllVehicles.Num(); i++)
	{
		if (AllVehicles[i] == ControllerVehicle)
			return i + 1;
	}

	return AllVehicles.Num();
}

int AAIC_Vehicle::GetTotalRacers()
{
	return AllVehicles.Num();
}
