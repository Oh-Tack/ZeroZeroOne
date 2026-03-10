// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Destruction/GasStationRoofActor.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Field/FieldSystemComponent.h"
#include "Particles/ParticleSystem.h"

AGasStationRoofActor::AGasStationRoofActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RoofMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofMesh"));
	SetRootComponent(RoofMesh);

	RoofGC = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("RoofGC"));
	RoofGC->SetupAttachment(RoofMesh);

	// GC는 처음엔 비활성화
	RoofGC->SetSimulatePhysics(false);
	RoofGC->SetEnableGravity(false);
	RoofGC->SetEnableDamageFromCollision(false);
	RoofGC->SetVisibility(false);
	RoofGC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGasStationRoofActor::BeginPlay()
{
	Super::BeginPlay();

	RoofMesh->SetVisibility(true);
	RoofGC->SetVisibility(false);
	RoofGC->SetSimulatePhysics(false);
	RoofGC->SetEnableDamageFromCollision(false);
}

void AGasStationRoofActor::TriggerDestruction()
{
	// SM 숨기고 GC 활성화
	RoofMesh->SetVisibility(false);
	RoofGC->SetVisibility(true);
	RoofGC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RoofGC->SetEnableGravity(true);
	RoofGC->SetSimulatePhysics(true);

	if (ExplosionParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionParticle,
			GetActorLocation() + ParticleSpawnOffset,
			FRotator::ZeroRotator,
			ParticleScale,
			true);
	}

	if (ExplosionNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionNiagara,
			GetActorLocation() + ParticleSpawnOffset,
			FRotator::ZeroRotator,
			ParticleScale,
			true);
	}

	ApplyStrain();
}

void AGasStationRoofActor::ApplyStrain()
{
	RoofGC->ApplyExternalStrain(
		INDEX_NONE,
		GetActorLocation(),
		DestructionRadius,
		1,
		1.f,
		DestructionStrain);
}
