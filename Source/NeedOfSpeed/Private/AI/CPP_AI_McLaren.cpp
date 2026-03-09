#include "NeedOfSpeed/Public/AI/CPP_AI_McLaren.h"

#include "NeedOfSpeed/Public/AI/CPP_McLaren_Front.h"
#include "NeedOfSpeed/Public/AI/CPP_McLaren_Rear.h"
#include "NeedOfSpeed/Public/AI/CPP_Road.h"

#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "AIController.h"
#include "Landscape.h"
#include "AI/CPP_AIRaceManager.h"
#include "UObject/ConstructorHelpers.h"


ACPP_AI_McLaren::ACPP_AI_McLaren()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	Max_Speed = 200.f;
	Min_Speed = 50.f;
	Angle = 30.f;

	USkeletalMeshComponent* MeshComp = GetMesh();

	// AI Controller 설정
	AIControllerClass = LoadClass<AAIController>(nullptr, TEXT("/Script/NeedOfSpeed.AIC_Vehicle"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// -------------------------
	// 센서
	// -------------------------

	Sensor_F = CreateDefaultSubobject<USceneComponent>(TEXT("Sensor_F"));
	Sensor_F->SetupAttachment(MeshComp);
	Sensor_F->SetRelativeLocation(FVector(280.0f, 0.0f, 20.0f));

	// -------------------------
	// 전방 감지 박스
	// -------------------------

	FrontViewBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontViewBox"));
	FrontViewBox->SetupAttachment(MeshComp);
	FrontViewBox->SetRelativeLocation(FVector(1000.0f, 0.0f, 90.0f));
	FrontViewBox->SetRelativeScale3D(FVector(23.5f, 10.0f, 3.5f));

	// -------------------------
	// 좌측 감지
	// -------------------------

	LeftSideBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftSideBox"));
	LeftSideBox->SetupAttachment(FrontViewBox);
	LeftSideBox->SetRelativeLocation(FVector(-30.0f, -100.0f, 0.0f));
	LeftSideBox->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.0f));

	// -------------------------
	// 우측 감지
	// -------------------------

	RightSideBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightSideBox"));
	RightSideBox->SetupAttachment(FrontViewBox);
	RightSideBox->SetRelativeLocation(FVector(-30.0f, 100.0f, 0.0f));
	RightSideBox->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.0f));

	// -------------------------
	// Mesh 설정
	// -------------------------

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

	// -------------------------
	// Wheel 설정
	// -------------------------

	if (UChaosWheeledVehicleMovementComponent* MovementComponent =
		Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		MovementComponent->WheelSetups.SetNum(4);

		MovementComponent->WheelSetups[0].WheelClass = UCPP_McLaren_Front::StaticClass();
		MovementComponent->WheelSetups[0].BoneName = FName("FL");

		MovementComponent->WheelSetups[1].WheelClass = UCPP_McLaren_Front::StaticClass();
		MovementComponent->WheelSetups[1].BoneName = FName("FR");

		MovementComponent->WheelSetups[2].WheelClass = UCPP_McLaren_Rear::StaticClass();
		MovementComponent->WheelSetups[2].BoneName = FName("RL");

		MovementComponent->WheelSetups[3].WheelClass = UCPP_McLaren_Rear::StaticClass();
		MovementComponent->WheelSetups[3].BoneName = FName("RR");
	}

	// -------------------------
	// Collision 설정
	// -------------------------

	FrontViewBox->SetGenerateOverlapEvents(true);
	FrontViewBox->SetCollisionProfileName(TEXT("AI_Box"));

	LeftSideBox->SetGenerateOverlapEvents(true);
	LeftSideBox->SetCollisionProfileName(TEXT("AI_Box"));

	RightSideBox->SetGenerateOverlapEvents(true);
	RightSideBox->SetCollisionProfileName(TEXT("AI_Box"));

	FrontViewBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftSideBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightSideBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}


void ACPP_AI_McLaren::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		GetMesh()->SetNotifyRigidBodyCollision(true);
		GetMesh()->OnComponentHit.AddDynamic(this, &ACPP_AI_McLaren::OnVehicleHit);

		UE_LOG(LogTemp, Warning, TEXT("Hit Event Bound Successfully for %s"), *GetName());
	}
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


void ACPP_AI_McLaren::GetCollisionBoxes_Implementation(
	UBoxComponent*& FrontBox,
	UBoxComponent*& LeftBox,
	UBoxComponent*& RightBox)
{
	FrontBox = FrontViewBox;
	LeftBox = LeftSideBox;
	RightBox = RightSideBox;
}


FVector ACPP_AI_McLaren::GetFrontOfCar_Implementation()
{
	return Sensor_F->GetComponentLocation();
}


float ACPP_AI_McLaren::GetCurrentSpeed_Implementation()
{
	return FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
}

void ACPP_AI_McLaren::OnVehicleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (bIsRespawning || !OtherActor || OtherActor->IsA(ALandscape::StaticClass())) return;

	float ImpulseStrength = NormalImpulse.Size();

	if (ImpulseStrength > 100000.f || bDestroyCar)
	{
		bDestroyCar = true;
		// 충돌 감지 비활성화 (연속 Hit 방지)
		if (GetMesh())
		{
			GetMesh()->SetNotifyRigidBodyCollision(false);
		}

		// 이미 리스폰 예약되어 있지 않으면 타이머 설정
		if (!GetWorldTimerManager().IsTimerActive(RespawnTimer))
		{
			GetWorldTimerManager().SetTimer(
				RespawnTimer,
				this,
				&ACPP_AI_McLaren::RespawnVehicle,
				5.0f,
				false
			);
		}
	}
}


void ACPP_AI_McLaren::RespawnVehicle()
{
	if (bIsRespawning) return;
	bIsRespawning = true;

	ACPP_AIRaceManager* RaceManager = ACPP_AIRaceManager::GetInstance(GetWorld());
	if (!RaceManager || !RoadSpline)
	{
		bIsRespawning = false;
		return;
	}

	float CurrentDist = RaceManager->GetDistanceOfVehicle(this);
	float RespawnDistance = FMath::Max(0.f, CurrentDist - 3000.f);

	FVector NewLocation = RoadSpline->GetLocationAtDistanceAlongSpline(
		RespawnDistance,
		ESplineCoordinateSpace::World
	);

	FRotator NewRotation = RoadSpline->GetRotationAtDistanceAlongSpline(
		RespawnDistance,
		ESplineCoordinateSpace::World
	);

	auto* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	if (Movement)
	{
		Movement->StopMovementImmediately();
		Movement->ResetVehicleState();
	}

	// Mesh Teleport (Wheel 분리 방지)
	if (GetMesh())
	{
		GetMesh()->SetWorldLocationAndRotation(
			NewLocation,
			NewRotation,
			false,
			nullptr,
			ETeleportType::TeleportPhysics
		);
	}

	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		if (GetMesh())
		{
			GetMesh()->SetNotifyRigidBodyCollision(true);
		}

		bIsRespawning = false;
		bDestroyCar = false;
	});
}
