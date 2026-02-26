// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/Gameplay/Destruction/ExplodeLaunchComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

// Sets default values for this component's properties
UExplodeLaunchComponent::UExplodeLaunchComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UExplodeLaunchComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UExplodeLaunchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UExplodeLaunchComponent::TriggerExplode()
{
	if (!IsValid(TargetMesh)) return;
	
	if (IsValid(ExplosionFX))
	{
		const FVector SpawnLocation = TargetMesh->GetComponentLocation() + FXOffset;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionFX,
			SpawnLocation,
			FRotator::ZeroRotator
		);
	}
}

