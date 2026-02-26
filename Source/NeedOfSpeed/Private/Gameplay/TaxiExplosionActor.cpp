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
	PrimaryActorTick.bCanEverTick = false;

	TaxiMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaxiMesh"));
	SetRootComponent(TaxiMesh);
	
	// 물리 비활성화
	TaxiMesh->SetSimulatePhysics(false);
	TaxiMesh->SetEnableGravity(false);
	
	// collision 설정
	TaxiMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TaxiMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
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
	TaxiMesh->SetLinearDamping(0.f);
	TaxiMesh->SetAngularDamping(0.f); 
	TaxiMesh->SetEnableGravity(true);
	TaxiMesh->SetSimulatePhysics(true);
	TaxiMesh->WakeAllRigidBodies(); 
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		TaxiMesh->SetPhysicsLinearVelocity(LaunchDirection.GetSafeNormal() * LinearImpulseStrength);
		TaxiMesh->SetPhysicsAngularVelocityInDegrees(AngularImpulseDegrees);
	});
	
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

