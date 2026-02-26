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
	GCComp->SetupAttachment(RootComponent);
	
	GCComp->SetSimulatePhysics(false);
	GCComp->SetEnableGravity(false);
	GCComp->SetEnableDamageFromCollision(false);
	// GCComp->SetNotifyRigidBodyCollision(true);
	GCComp->SetVisibility(false);
	GCComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	UE_LOG(LogTemp, Warning, TEXT("[Tower] StartCollapse 호출됨"));
	if (bHasCollapsed) return;
	bHasCollapsed = true;
	
	TowerMeshComp->SetSimulatePhysics(true);
	TowerMeshComp->SetEnableGravity(true);
	GCComp->WakeAllRigidBodies();
	
	const FVector RotationAxis  = GetActorRightVector();
	const FVector AngularImpulse = RotationAxis * CollapseAngularImpulse;
	TowerMeshComp->AddAngularImpulseInDegrees(AngularImpulse, NAME_None, true);
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
	
	const FTransform& MeshTransform = TowerMeshComp->GetComponentTransform();
	GCComp->SetWorldLocationAndRotation(MeshTransform.GetLocation(),MeshTransform.GetRotation());
	
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
