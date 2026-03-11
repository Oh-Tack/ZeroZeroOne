// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Satellite/SatelliteSignActor.h"

// Sets default values
ASatelliteSignActor::ASatelliteSignActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SignMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignMesh"));
	SetRootComponent(SignMesh);
	SignMesh->SetSimulatePhysics(false);
	SignMesh->SetEnableGravity(false);
	SignMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SignMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

// Called when the game starts or when spawned
void ASatelliteSignActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASatelliteSignActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASatelliteSignActor::TriggerRoll()
{
	if (bHasRolled) return;
	bHasRolled = true;

	SignMesh->SetLinearDamping(0.5f);
	SignMesh->SetAngularDamping(AngularDamping);
	SignMesh->SetEnableGravity(true);
	SignMesh->SetSimulatePhysics(true);
	SignMesh->WakeAllRigidBodies();

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		const FVector Dir = FVector(RollDirection.X, RollDirection.Y, 0.f).GetSafeNormal();
		SignMesh->SetPhysicsLinearVelocity(Dir * ImpulseStrength);
		SignMesh->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
	});
}
