#include "CPP_AI_McLaren.h"

#include "CPP_McLaren_Front.h"
#include "CPP_McLaren_Rear.h"
#include "Components/SkeletalMeshComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"

ACPP_AI_McLaren::ACPP_AI_McLaren()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetSimulatePhysics(true);
		
		static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(TEXT("/Game/Resources/McLaren/ABP_McLaren_Wheel"));
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