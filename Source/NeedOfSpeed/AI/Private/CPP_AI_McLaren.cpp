#include "NeedOfSpeed/AI/Public/CPP_AI_McLaren.h"

#include "NeedOfSpeed/AI/Public/CPP_McLaren_Front.h"
#include "NeedOfSpeed/AI/Public/CPP_McLaren_Rear.h"
#include "NeedOfSpeed/AI/Public/CPP_Road.h"
#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "AIController.h"


ACPP_AI_McLaren::ACPP_AI_McLaren()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	USkeletalMeshComponent* MeshComp = GetMesh();

	AIControllerClass = LoadClass<AAIController>(nullptr, TEXT("/Script/NeedOfSpeed.AIC_Vehicle"));

	Sensor_L = CreateDefaultSubobject<USceneComponent>(TEXT("Sensor_L"));
	Sensor_L->SetupAttachment(MeshComp);
	Sensor_L->SetRelativeLocation(FVector(200.0f, -130.0f, 0.0f));

	Sensor_R = CreateDefaultSubobject<USceneComponent>(TEXT("Sensor_R"));
	Sensor_R->SetupAttachment(MeshComp);
	Sensor_R->SetRelativeLocation(FVector(200.0f, 130.0f, 0.0f));

	Sensor_F = CreateDefaultSubobject<USceneComponent>(TEXT("Sensor_F"));
	Sensor_F->SetupAttachment(MeshComp);
	Sensor_F->SetRelativeLocation(FVector(280.0f, 0.0f, 20.0f));

	if (MeshComp)
	{
		MeshComp->SetSimulatePhysics(true);

		static ConstructorHelpers::FClassFinder<UAnimInstance>
			AnimBP(TEXT("/Game/Resources/McLaren/ABP_McLaren_Wheel"));
		if (AnimBP.Succeeded())
		{
			MeshComp->SetAnimInstanceClass(AnimBP.Class);
		}
	}

	if (UChaosWheeledVehicleMovementComponent* MovementComponent =
		Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		
		
		MovementComponent->WheelSetups.SetNum(4);

		MovementComponent->WheelSetups[0].WheelClass = UCPP_McLaren_Front::StaticClass();
		MovementComponent->WheelSetups[0].BoneName = FName("FL");
		MovementComponent->WheelSetups[0].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

		MovementComponent->WheelSetups[1].WheelClass = UCPP_McLaren_Front::StaticClass();
		MovementComponent->WheelSetups[1].BoneName = FName("FR");
		MovementComponent->WheelSetups[1].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

		MovementComponent->WheelSetups[2].WheelClass = UCPP_McLaren_Rear::StaticClass();
		MovementComponent->WheelSetups[2].BoneName = FName("RL");
		MovementComponent->WheelSetups[2].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

		MovementComponent->WheelSetups[3].WheelClass = UCPP_McLaren_Rear::StaticClass();
		MovementComponent->WheelSetups[3].BoneName = FName("RR");
		MovementComponent->WheelSetups[3].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);
		
		
	}
}

void ACPP_AI_McLaren::BeginPlay()
{
	Super::BeginPlay();
}

void ACPP_AI_McLaren::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPP_AI_McLaren::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(Road))
	{
		RoadSpline = Road->Spline;
	}
	else
	{
		RoadSpline = nullptr;
	}
}

void ACPP_AI_McLaren::SetThrottle_Implementation(float Throttle)
{
	GetVehicleMovement()->SetThrottleInput(Throttle);
}

void ACPP_AI_McLaren::SetSteering_Implementation(float Steering)
{
	GetVehicleMovement()->SetSteeringInput(Steering);
}

void ACPP_AI_McLaren::SetBrake_Implementation(float Brake)
{
	GetVehicleMovement()->SetBrakeInput(Brake);
}

FVector ACPP_AI_McLaren::GetFrontOfCar_Implementation()
{
	return Sensor_F->GetComponentLocation();
}

float ACPP_AI_McLaren::GetCurrentSpeed_Implementation()
{
	return FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
}