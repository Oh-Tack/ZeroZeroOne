// Fill out your copyright notice in the Description page of Project Settings.

#include "Powerplay/Bus/BusExplosionActor.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"

ABusExplosionActor::ABusExplosionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BusMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BusMesh"));
	SetRootComponent(BusMesh);

	BusMesh->SetSimulatePhysics(false);
	BusMesh->SetEnableGravity(false);
	BusMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BusMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

void ABusExplosionActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABusExplosionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABusExplosionActor::TriggerExplosion()
{
	if (bHasExploded) return;
	bHasExploded = true;

	if (ExplosionParticle)
	{
		UGameplayStatics::SpawnEmitterAttached(
			ExplosionParticle,
			BusMesh,
			NAME_None,
			ParticleSpawnOffset,
			FRotator::ZeroRotator,
			ParticleScale,
			EAttachLocation::KeepRelativeOffset,
			true);
	}

	if (ExplosionNiagara)
	{
		UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAttached(
			ExplosionNiagara,
			BusMesh,
			NAME_None,
			ParticleSpawnOffset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
		if (NC)
		{
			NC->SetRelativeScale3D(ParticleScale);
		}
	}

	FTimerHandle LaunchDelayHandle;
	GetWorld()->GetTimerManager().SetTimer(LaunchDelayHandle, this, &ABusExplosionActor::ExecuteLaunch, LaunchDelay, false);
}

void ABusExplosionActor::ExecuteLaunch()
{
	// 땅과 관통 상태에서 물리 활성화 시 Chaos가 위로 강제 탈출시키는 문제 방지
	// BusMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	
	BusMesh->SetLinearDamping(0.f);
	BusMesh->SetAngularDamping(0.f);
	BusMesh->SetEnableGravity(true);
	BusMesh->SetSimulatePhysics(true);
	BusMesh->WakeAllRigidBodies();

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		BusMesh->SetPhysicsLinearVelocity(LaunchDirection.GetSafeNormal() * LinearImpulseStrength);
		BusMesh->SetPhysicsAngularVelocityInDegrees(AngularImpulseDegrees);
		// BusMesh->AddAngularImpulseInDegrees(AngularImpulseDegrees, NAME_None, true);
		// 속도 설정 후 충돌 복원
		// BusMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	});
}
