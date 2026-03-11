// Fill out your copyright notice in the Description page of Project Settings.

#include "Powerplay/GasStation/GasStationExplosionActor.h"
#include "Powerplay/GasStation/GasStationRoofActor.h"
#include "Components/StaticMeshComponent.h"

AGasStationExplosionActor::AGasStationExplosionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GasStationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GasStationMesh"));
	SetRootComponent(GasStationMesh);

	GasStationMesh->SetSimulatePhysics(false);
	GasStationMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GasStationMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
}

void AGasStationExplosionActor::BeginPlay()
{
	Super::BeginPlay();
}

void AGasStationExplosionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGasStationExplosionActor::TriggerExplosion()
{
	if (bHasExploded) return;
	bHasExploded = true;
	
	if (RoofActor != nullptr)
	{
		RoofActor->TriggerDestruction();
	}
}
