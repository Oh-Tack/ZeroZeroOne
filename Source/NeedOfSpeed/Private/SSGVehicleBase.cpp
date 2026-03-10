#include "SSGVehicleBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Landscape.h"

ASSGVehicleBase::ASSGVehicleBase()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetNotifyRigidBodyCollision(true);
	MeshComp->BodyInstance.bUseCCD = true;

	MeshComp->OnComponentHit.AddDynamic(this, &ASSGVehicleBase::OnVehicleHit);
}

void ASSGVehicleBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASSGVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ASSGVehicleBase::CalculateImpactScore(const FVector& NormalImpulse, UPrimitiveComponent* OtherComp)
{
	float ImpulseStrength = NormalImpulse.Size();
	FVector MyVel = MeshComp->GetComponentVelocity();
	FVector OtherVel = OtherComp ? OtherComp->GetComponentVelocity() : FVector::ZeroVector;
	float ImpactSpeed = (MyVel - OtherVel).Size();

	float ForwardDot = FVector::DotProduct(NormalImpulse.GetSafeNormal(), GetActorForwardVector());
	float DirMul = FMath::Lerp(1.f, 1.8f, FMath::Abs(ForwardDot));

	return ((ImpulseStrength * 0.00001f) + (ImpactSpeed * 0.015f)) * DirMul;
}

void ASSGVehicleBase::OnVehicleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (bDestroyCar || bIsRespawning) return;
	if (!OtherActor || OtherActor == this) return;
	if (OtherActor->IsA(ALandscape::StaticClass())) return;

	float ImpactScore = CalculateImpactScore(NormalImpulse, OtherComp);
	if (ImpactScore < 50.f) return;

	bDestroyCar = true;

	// 🔥 파괴 이펙트
	if (DestroyEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyEffect, GetActorLocation(), GetActorRotation(), true);
	}

	// 💨 차량 날리기
	if (MeshComp)
	{
		FVector Launch = NormalImpulse.GetSafeNormal() * ImpactScore * LaunchMultiplier;
		MeshComp->AddImpulse(Launch, NAME_None, true);

		FVector Torque = FVector::UpVector * ImpactScore * 5000.f;
		MeshComp->AddTorqueInRadians(Torque, NAME_None, true);
	}

	// 델리게이트 호출
	OnVehicleDestroyed.Broadcast(this);

	// 일정 시간 후 리스폰
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		GetWorldTimerManager().SetTimer(
			Handle,
			this,
			&ASSGVehicleBase::RespawnVehicle,
			RespawnTime,
			false
		);
	});
}

void ASSGVehicleBase::RespawnVehicle()
{
	if (!bDestroyCar) return;
	bIsRespawning = true;

	// 위치/회전 리셋 (예시: Z 높이만 올림)
	FVector NewLocation = GetActorLocation();
	NewLocation.Z += 50.f;

	FRotator NewRotation = GetActorRotation();

	if (MeshComp)
	{
		MeshComp->SetWorldLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
		MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	}

	bDestroyCar = false;
	bIsRespawning = false;
}