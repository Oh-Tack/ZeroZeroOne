// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/Gameplay/TaxiExplosionActor.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

// Sets default values
ATaxiExplosionActor::ATaxiExplosionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TaxiMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaxiMesh"));
	SetRootComponent(TaxiMesh);
	
	// 물리 비활성화
	TaxiMesh->SetSimulatePhysics(false);
	TaxiMesh->SetEnableGravity(false);
}

// Called when the game starts or when spawned
void ATaxiExplosionActor::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		EnableInput(PC);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATaxiExplosionActor::TriggerExplosion);
		}
	}
}

// Called every frame
void ATaxiExplosionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATaxiExplosionActor::TriggerExplosion()
{
	if (bHasExploded) return;
	bHasExploded = true;
	
	// 물리 활성화
	TaxiMesh->SetEnableGravity(true);
	TaxiMesh->SetSimulatePhysics(true);
	
	const FVector ExplosionOrigin = GetActorLocation() + ExplosionOriginOffset;

	TaxiMesh->AddRadialImpulse(
		ExplosionOrigin,
		ExplosionRadius,
		RadialImpulseStrength,
		ERadialImpulseFalloff::RIF_Constant,
		/*bImpulseVelChange=*/false);
	
	const FVector DirectionalForce = LaunchDirection.GetSafeNormal() * LinearImpulseStrength;
	TaxiMesh->AddImpulse(DirectionalForce, NAME_None, /*bVelChange=*/false);
	
	TaxiMesh->AddAngularImpulseInDegrees(AngularImpulseDegrees, NAME_None, /*bVelChange=*/false);
	
	if (ExplosionParticle)
	{
		const FVector SpawnLocation = GetActorLocation() + ParticleSpawnOffset;
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionParticle,
			SpawnLocation,
			FRotator::ZeroRotator,
			/*bAutoDestroy=*/true);
	}
}

