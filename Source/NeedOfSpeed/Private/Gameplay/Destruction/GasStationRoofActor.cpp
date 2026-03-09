// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Destruction/GasStationRoofActor.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Field/FieldSystemComponent.h"
#include "Particles/ParticleSystem.h"

AGasStationRoofActor::AGasStationRoofActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RoofGC = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("RoofGC"));
	SetRootComponent(RoofGC);
	
	RoofGC->SetSimulatePhysics(false);
	RoofGC->SetEnableGravity(false);
	
	// ConstructorHelpers::FClassFinder<AActor> tempDestoyer(TEXT("/Script/Engine.Blueprint'/Game/Track/Blueprints/BP_Destroyer.BP_Destroyer_C'"));
	// if (tempDestoyer.Succeeded())
	// {
	// 	DestroyerActor = tempDestoyer.Class;
	// }
}

void AGasStationRoofActor::BeginPlay()
{
	Super::BeginPlay();
	
	RoofGC->SetSimulatePhysics(false);
}

void AGasStationRoofActor::TriggerDestruction()
{
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
	// GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Cyan, GetActorNameOrLabel());
	//
	// GetWorld()->SpawnActor<AActor>(DestroyerActor, GetActorTransform());
	//fs->GetFieldSystemComponent()->
	//
	RoofGC->ApplyExternalStrain(
		INDEX_NONE,
		GetActorLocation(),
		DestructionRadius,
		1,
		1.f,
		DestructionStrain);
}
