// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Satellite/SatellitePoleActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ASatellitePoleActor::ASatellitePoleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PoleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleMesh"));
	SetRootComponent(PoleMesh);
	PoleMesh->SetSimulatePhysics(false);
	PoleMesh->SetEnableGravity(false);
	PoleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PoleMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

// Called when the game starts or when spawned
void ASatellitePoleActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASatellitePoleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASatellitePoleActor::TriggerCollapse()
{
	if (bHasCollapsed) return;
	bHasCollapsed = true;

	if (CollapseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollapseSound, GetActorLocation());
	}

	if (CollapseNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CollapseNiagara, GetActorLocation());
	}

	// 기둥 쓰러짐
	PoleMesh->SetEnableGravity(true);
	PoleMesh->SetSimulatePhysics(true);
	PoleMesh->WakeAllRigidBodies();

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			// 기둥 꼭대기에 뒤에서 미는 힘 → 자연스럽게 쓰러짐
			const FVector Dir = FVector(FallDirection.X, FallDirection.Y, 0.f).GetSafeNormal();
			const FBoxSphereBounds Bounds = PoleMesh->CalcBounds(PoleMesh->GetComponentTransform());
			const FVector TopLocation = FVector(
				Bounds.Origin.X,
				Bounds.Origin.Y,
				Bounds.Origin.Z + Bounds.BoxExtent.Z);
			PoleMesh->AddImpulseAtLocation(Dir * ImpulseStrength, TopLocation);
		});

	// 딜레이 후 표지판 굴러감
	FTimerHandle SignHandle;
	GetWorld()->GetTimerManager().SetTimer(SignHandle, [this]()
	{
		if (IsValid(SignActor))
		{
			SignActor->TriggerRoll();
		}
	}, SignReleaseDelay, false);
}