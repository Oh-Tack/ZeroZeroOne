// Fill out your copyright notice in the Description page of Project Settings.

#include "Powerplay/GasStation/GasStationRoofActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Field/FieldSystemComponent.h"
#include "Particles/ParticleSystem.h"

AGasStationRoofActor::AGasStationRoofActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);
	
	RoofMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofMesh"));
	RoofMesh->SetupAttachment(RootScene);

	RoofGC = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("RoofGC"));
	RoofGC->SetupAttachment(RootScene);

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
	UE_LOG(LogTemp, Warning, TEXT("[RoofActor] TriggerDestruction 호출됨"));
	UE_LOG(LogTemp, Warning, TEXT("[RoofActor] RoofGC 유효: %s"), IsValid(RoofGC) ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("[RoofActor] RoofGC Asset 유효: %s"), (IsValid(RoofGC) && RoofGC->GetRestCollection() != nullptr) ? TEXT("YES - GC Asset 있음") : TEXT("NO - GC Asset 없음!!"));

	RoofMesh->SetVisibility(false);
	// RoofMesh->SetHiddenInGame(true);
	RoofMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	RoofGC->SetVisibility(true);
	// RoofGC->SetHiddenInGame(false);
	RoofGC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RoofGC->SetEnableGravity(true);
	RoofGC->SetSimulatePhysics(true);
	UE_LOG(LogTemp, Warning, TEXT("[RoofActor] GC 활성화 완료, 0.1초 후 Strain 적용"));
	UE_LOG(LogTemp, Warning, TEXT("[RoofActor] RoofMesh 위치: %s"), *RoofMesh->GetComponentLocation().ToString());
	UE_LOG(LogTemp, Warning, TEXT("[RoofActor] RoofGC 위치: %s"), *RoofGC->GetComponentLocation().ToString());

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

	GetWorldTimerManager().SetTimer(StrainTimerHandle, this, &AGasStationRoofActor::ApplyStrain, 0.1f, false);
	// ApplyStrain();
}

void AGasStationRoofActor::ApplyStrain()
{
	UE_LOG(LogTemp, Warning, TEXT("[RoofActor] ApplyStrain 호출됨"));
	
	RoofGC->ApplyExternalStrain(
		INDEX_NONE,
		GetActorLocation(),
		DestructionRadius,
		1,
		1.f,
		DestructionStrain);
}
