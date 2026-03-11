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
#include "NiagaraFunctionLibrary.h"
#include "AI/CPP_AIRaceManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


ACPP_AI_McLaren::ACPP_AI_McLaren()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.f; // ⭐ 매 프레임

	Max_Speed = 200.f;
	Min_Speed = 60.f;
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
		GetMesh()->BodyInstance.bUseCCD = true;
		GetMesh()->OnComponentHit.AddDynamic(this, &ACPP_AI_McLaren::OnVehicleHit);

		BrakeMID = GetMesh()->CreateDynamicMaterialInstance(28);

		if (BrakeMID)
		{
			CurrentBrakeIntensity = 0.f;
			BrakeMID->SetScalarParameterValue(TEXT("Intensity"), 0.f);
		}
	}

	// ⭐ 차량마다 다른 차선 배정
	int32 LaneIndex = FMath::RandRange(-1, 1);
	RespawnLaneOffset = LaneIndex * 350.f;
	
	
}


void ACPP_AI_McLaren::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!BrakeMID) return;

	// 목표 밝기
	float TargetIntensity =
		FMath::Lerp(0.f, 10.f, CurrentBrakeAmount);

	// 부드러운 변화
	CurrentBrakeIntensity =
		FMath::FInterpTo(
			CurrentBrakeIntensity,
			TargetIntensity,
			DeltaTime,
			12.f);

	BrakeMID->SetScalarParameterValue(
		TEXT("Intensity"),
		CurrentBrakeIntensity);
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
	
	CurrentBrakeAmount = Brake;
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
	if (bIsRespawning || bDestroyCar) return;
	if (!OtherActor || OtherActor == this) return;
	if (OtherActor->IsA(ALandscape::StaticClass())) return;

	float ImpactScore = CalculateImpactScore(NormalImpulse, OtherComp);
	if (ImpactScore < 58.f) return;

	bDestroyCar = true;

	// 1️⃣ 차량에 파괴 이펙트 Attach (따라오게)
	if (DestroyEffect)
	{
		UNiagaraComponent* FX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			DestroyEffect,              // NiagaraSystem*
			GetMesh(),                  // 부모: 차량 Mesh
			NAME_None,                  // 소켓 없음
			FVector::ZeroVector,        // 위치 오프셋
			FRotator::ZeroRotator,      // 회전 오프셋
			EAttachLocation::KeepRelativeOffset, // 상대위치 유지
			true                        // 자동 소멸
		);
	}

	if (DestroySound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), DestroySound);
	}

	// 2️⃣ 차량 날라가기
	if (UPrimitiveComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(true);
		FVector Launch = NormalImpulse.GetSafeNormal() * ImpactScore * LaunchMultiplier;
		MeshComp->AddImpulse(Launch, NAME_None, true);
	}

	// 3️⃣ 충돌 무시 및 멈춤
	IgnoreVehicleCollision(true);
	GetMesh()->SetNotifyRigidBodyCollision(false);

	// 4️⃣ 일정 시간 후 리스폰
	GetWorldTimerManager().SetTimer(
		RespawnTimer,
		this,
		&ACPP_AI_McLaren::RespawnVehicle,
		5.f,
		false
	);
}


void ACPP_AI_McLaren::RespawnVehicle()
{
	if (bIsRespawning) return;
	bIsRespawning = true;

	auto* RaceManager =
		ACPP_AIRaceManager::GetInstance(GetWorld());

	if (!RaceManager || !RoadSpline)
	{
		bIsRespawning = false;
		return;
	}

	float CurrentDist =
		RaceManager->GetDistanceOfVehicle(this);

	float RespawnDistance =
		FMath::Max(0.f, CurrentDist - 3000.f);

	// ⭐ 슬롯 확보 (겹침 방지 핵심)
	int32 SlotIndex =
		RaceManager->AcquireRespawnSlot(RespawnDistance);

	// ---------------- SLOT GRID ----------------
	// ---------------- SLOT GRID (2 LANE ROAD) ----------------

	const float LaneWidth = 500.f; // 도로 끝 위치
	const float RowSpacing = 900.f; // 앞뒤 간격

	int32 Lane = SlotIndex % 2; // ⭐ 2차선
	int32 Row = SlotIndex / 2;

	// 좌(-1) / 우(+1)
	float LaneSide = (Lane == 0) ? -1.f : 1.f;

	float LaneOffset = LaneSide * LaneWidth;
	float BackOffset = Row * RowSpacing;

	// 뒤로 배치 + 안전 여유
	RespawnDistance =
		FMath::Max(0.f, RespawnDistance - BackOffset - 200.f);

	FVector BaseLocation =
		RoadSpline->GetLocationAtDistanceAlongSpline(
			RespawnDistance,
			ESplineCoordinateSpace::World);

	FVector RightVector =
		RoadSpline->GetRightVectorAtDistanceAlongSpline(
			RespawnDistance,
			ESplineCoordinateSpace::World);

	FRotator NewRotation =
		RoadSpline->GetRotationAtDistanceAlongSpline(
			RespawnDistance,
			ESplineCoordinateSpace::World);

	FVector NewLocation =
		BaseLocation + RightVector * LaneOffset;

	NewLocation.Z += 20.f;

	// reset vehicle
	if (auto* Move =
		Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		Move->StopMovementImmediately();
		Move->ResetVehicleState();
	}

	GetMesh()->SetWorldLocationAndRotation(
		NewLocation,
		NewRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	// restore
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		GetMesh()->SetNotifyRigidBodyCollision(true);
		IgnoreVehicleCollision(false);

		bIsRespawning = false;
		bDestroyCar = false;
	});
}

float ACPP_AI_McLaren::CalculateImpactScore(
	const FVector& NormalImpulse,
	UPrimitiveComponent* OtherComp)
{
	float ImpulseStrength = NormalImpulse.Size();

	FVector MyVel = GetMesh()->GetComponentVelocity();
	FVector OtherVel =
		OtherComp
			? OtherComp->GetComponentVelocity()
			: FVector::ZeroVector;

	float ImpactSpeed = (MyVel - OtherVel).Size();

	float ForwardDot =
		FVector::DotProduct(
			NormalImpulse.GetSafeNormal(),
			GetActorForwardVector());

	float DirMul =
		FMath::Lerp(1.f, 1.8f, FMath::Abs(ForwardDot));

	return ((ImpulseStrength * 0.00001f)
		+ (ImpactSpeed * 0.015f)) * DirMul;
}

void ACPP_AI_McLaren::ApplyImpactSpin(
	const FVector& NormalImpulse,
	float ImpactScore)
{
	float SideDot =
		FVector::DotProduct(
			NormalImpulse.GetSafeNormal(),
			GetActorRightVector());

	float SpinDir = (SideDot > 0) ? 1.f : -1.f;

	FVector Torque(0, 0,
	               SpinDir * ImpactScore * 50000.f);

	GetMesh()->AddTorqueInRadians(Torque, NAME_None, true);
}

void ACPP_AI_McLaren::IgnoreVehicleCollision(bool bIgnore)
{
	GetMesh()->SetCollisionResponseToChannel(
		ECC_Vehicle,
		bIgnore ? ECR_Ignore : ECR_Block);
}
