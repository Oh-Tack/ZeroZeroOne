// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/Gameplay/Destruction/TowerDestructionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

// Sets default values
ATowerDestructionActor::ATowerDestructionActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	TowerMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	TowerMeshComp->SetupAttachment(RootComponent);
	TowerMeshComp->SetSimulatePhysics(false);
	TowerMeshComp->SetEnableGravity(false);
	TowerMeshComp->SetCollisionProfileName(TEXT("BlockAll"));
	TowerMeshComp->SetNotifyRigidBodyCollision(true); 
	
	GCComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GCComp"));
	SetRootComponent(GCComp);
	
	GCComp->SetSimulatePhysics(false);
	GCComp->SetEnableGravity(false);
	GCComp->SetEnableDamageFromCollision(false);
	GCComp->SetNotifyRigidBodyCollision(true);
	// GCComp->SetVisibility(false);
	// GCComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// GCComp->SetEnableDamageFromCollision(false);
}

// Called when the game starts or when spawned
void ATowerDestructionActor::BeginPlay()
{
	Super::BeginPlay();
	
	TowerMeshComp->OnComponentHit.AddDynamic(this, &ATowerDestructionActor::OnTowerMeshHit);
}

// Called every frame
void ATowerDestructionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATowerDestructionActor::StartCollapse()
{
	if (bHasCollapsed) return;
	bHasCollapsed = true;
	
	TowerMeshComp->SetSimulatePhysics(true);
	TowerMeshComp->SetEnableGravity(true);
	GCComp->WakeAllRigidBodies();
	
	const FVector RotationAxis  = GetActorRightVector();
	const FVector AngularImpulse = RotationAxis * CollapseAngularImpulse;
	/*const FVector ImpulseLocation = GetActorLocation() + GetActorUpVector() * CollapseApplyHeight;
	const FVector ForwardImpulse  = GetActorForwardVector() * CollapseImpulseStrength;
	GCComp->AddImpulseAtLocation(ForwardImpulse, ImpulseLocation);
	
	GetWorldTimerManager().SetTimer(
		FallbackFractureTimer,
		this,
		&ATowerDestructionActor::FallbackFracture,
		MaxFallTime,
		false
	);*/
}

void ATowerDestructionActor::OnTowerMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bHasCollapsed || bFractured) return;
	
	if (NormalImpulse.Size() >= FractureImpulseThreshold)
	{
		bFractured = true;
		ActivateFracture(Hit.ImpactPoint);
	}
}

void ATowerDestructionActor::ActivateFracture(const FVector& ImpactPoint)
{
	TowerMeshComp->SetSimulatePhysics(false);
	TowerMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TowerMeshComp->SetVisibility(false);
	
	GCComp->SetWorldTransform(TowerMeshComp->GetComponentTransform());
	
	GCComp->SetVisibility(true);
	GCComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GCComp->SetEnableDamageFromCollision(true);
	GCComp->SetSimulatePhysics(true);
	
	GCComp->AddRadialImpulse(
		ImpactPoint,
		FractureRadialRadius,    
		FractureRadialStrength, 
		ERadialImpulseFalloff::RIF_Linear,
		false                    
	);
}
