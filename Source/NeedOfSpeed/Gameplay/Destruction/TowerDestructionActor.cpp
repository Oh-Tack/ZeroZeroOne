// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Gameplay/Destruction/TowerDestructionActor.h"

#include "Field/FieldSystemObjects.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "TimerManager.h"

// Sets default values
ATowerDestructionActor::ATowerDestructionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GCComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GCComp"));
	SetRootComponent(GCComp);
	
	GCComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GCComp->SetSimulatePhysics(true);
	
	GCComp->SetNotifyRigidBodyCollision(true);
	
	GCComp->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	
	
}

// Called when the game starts or when spawned
void ATowerDestructionActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 충돌 이벤트 바인딩
	GCComp->OnChaosPhysicsCollision.AddDynamic(this, &ATowerDestructionActor::onChaosHit);
	
	FTimerHandle TH;
	GetWorldTimerManager().SetTimer(TH, [this]()
	{
		const FVector Axis = FVector::ForwardVector;
		GCComp->AddAngularImpulseInDegrees(Axis * InitialImpulse, NAME_None, true);
	}, 0.15f, false);
}

// Called every frame
void ATowerDestructionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATowerDestructionActor::onChaosHit(const FChaosPhysicsCollisionInfo& Info)
{
	if (bBrokenOnHit) return;
	
	const float Strength = Info.AccumulatedImpulse.Size();
	if (Strength < BreakImpulseThreshold) return;
	
	bBrokenOnHit = true;
	
	ApplyStrainAt(Info.Location);
}

void ATowerDestructionActor::ApplyStrainAt(const FVector& WorldPos)
{
	URadialFalloff* StrainField = NewObject<URadialFalloff>(this);
	StrainField->Magnitude = StrainMagnitude;
	StrainField->MinRange = 0.f;
	StrainField->MaxRange = StrainRadius;
	StrainField->Default = 0.f;
	StrainField->Radius = StrainRadius;
	StrainField->Position = WorldPos;
	StrainField->Falloff = EFieldFalloffType::Field_FallOff_None;
	
	GCComp->ApplyPhysicsField(
		true,
		EGeometryCollectionPhysicsTypeEnum::Chaos_ExternalClusterStrain,
		nullptr,
		StrainField
		);
}

