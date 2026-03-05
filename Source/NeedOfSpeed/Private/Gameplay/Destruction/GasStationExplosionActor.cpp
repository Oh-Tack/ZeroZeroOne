// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Destruction/GasStationExplosionActor.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"

// Sets default values
AGasStationExplosionActor::AGasStationExplosionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GasStationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GasStationMesh"));
	SetRootComponent(GasStationMesh);

	GasStationMesh->SetSimulatePhysics(false);
	GasStationMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GasStationMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	RoofGC = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("RoofGC"));
	RoofGC->SetupAttachment(GasStationMesh);
	RoofGC->SetSimulatePhysics(false);
}

// Called when the game starts or when spawned
void AGasStationExplosionActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGasStationExplosionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGasStationExplosionActor::TriggerExplosion()
{
	if (bHasExploded) return;
	bHasExploded = true;

	// 파티클 이펙트
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

	// 나이아가라 이펙트
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

	// 천장 Chaos 파괴 (천장 높이 기준으로 Strain 적용)
	if (IsValid(RoofGC))
	{
		RoofGC->SetSimulatePhysics(true);

		const FVector RoofLocation = GetActorLocation() + FVector(0.f, 0.f, RoofHeightOffset);
		RoofGC->ApplyExternalStrain(
			INDEX_NONE,
			RoofLocation,
			DestructionRadius,
			1,
			1.f,
			DestructionStrain);
	}
}
