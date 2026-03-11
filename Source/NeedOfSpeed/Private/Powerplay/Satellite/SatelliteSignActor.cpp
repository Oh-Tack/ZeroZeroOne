// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Satellite/SatelliteSignActor.h"
#include "NiagaraFunctionLibrary.h"

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

	SignMesh->OnComponentHit.AddDynamic(this, &ASatelliteSignActor::OnHit);
}

// Called every frame
void ASatelliteSignActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASatelliteSignActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bHasRolled || !IsValid(SparkFX)) return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), SparkFX,
		Hit.ImpactPoint,
		Hit.ImpactNormal.Rotation()
	);
}

void ASatelliteSignActor::TriggerRoll()
{
	if (bHasRolled) return;
	bHasRolled = true;

	UE_LOG(LogTemp, Warning, TEXT("[SignActor] TriggerRoll 호출됨"));

	SignMesh->SetLinearDamping(0.5f);
	SignMesh->SetAngularDamping(AngularDamping);
	SignMesh->SetEnableGravity(true);
	SignMesh->SetSimulatePhysics(true);
	SignMesh->WakeAllRigidBodies();

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		const FVector Dir = FVector(RollDirection.X, RollDirection.Y, 0.f).GetSafeNormal();
		UE_LOG(LogTemp, Warning, TEXT("[SignActor] 속도 적용: Dir=%s, Speed=%f"), *Dir.ToString(), ImpulseStrength);
		SignMesh->SetPhysicsLinearVelocity(Dir * ImpulseStrength);
	});
}
