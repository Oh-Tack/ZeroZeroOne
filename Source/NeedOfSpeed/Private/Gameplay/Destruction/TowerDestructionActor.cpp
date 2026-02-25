// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/Gameplay/Destruction/TowerDestructionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "TimerManager.h"

// Sets default values
ATowerDestructionActor::ATowerDestructionActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GCComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("TowerMesh"));
	SetRootComponent(GCComp);
	
	// 게임 시작 시 정지 상태 유지 — StartPowerPlay() 호출 전까지 움직이지 않음
	GCComp->SetSimulatePhysics(false);
}

// Called when the game starts or when spawned
void ATowerDestructionActor::BeginPlay()
{
	Super::BeginPlay();
	
	GCComp->SetSimulatePhysics(false);
	GCComp->SetEnableGravity(false);
}

// Called every frame
void ATowerDestructionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATowerDestructionActor::StartPowerPlay()
{
	GCComp->SetSimulatePhysics(true);
	GCComp->SetEnableGravity(true);
	GCComp->WakeAllRigidBodies();         
	GCComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); 
	
	FVector TopLocation = GetActorLocation() + FVector(0, 0, 1500); // 타워 높이에 맞춰 조절
	GCComp->AddImpulseAtLocation(FVector(1000, 0, -500) * 100, TopLocation);

	// 0.5초 뒤에 body 붕괴
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ATowerDestructionActor::FallBody, 0.5f, false);
}

void ATowerDestructionActor::FallBody()
{
	FVector ForwardForce = GetActorForwardVector() * 5000000.0f; 
	FVector ImpulsePos = GetActorLocation() + FVector(0, 0, 800); 
    
	GCComp->AddImpulseAtLocation(ForwardForce, ImpulsePos);
}
