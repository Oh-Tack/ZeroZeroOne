// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Tower/TowerTriggerActor.h"
#include "Powerplay/Tower/TowerDestructionActor.h"
#include "NeedOfSpeed/Public/Player/RacingCar.h"

// Sets default values
ATowerTriggerActor::ATowerTriggerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void ATowerTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATowerTriggerActor::OnOverlapBegin);
}

// Called every frame
void ATowerTriggerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATowerTriggerActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[TowerTrigger] 오버랩 감지: %s"), *OtherActor->GetName());
	
	if (!Cast<ARacingCar>(OtherActor)) return;
	
	if (!IsValid(TargetTower))
	{
		UE_LOG(LogTemp, Error, TEXT("[TowerTrigger] TargetTower가 설정되지 않음"));
		return; 
	}
	
	bTriggered = true;
	// const FVector FallDirection = -OtherActor->GetVelocity().GetSafeNormal();
	const FVector FallDirection = -TargetTower->GetActorForwardVector();
	TargetTower->StartCollapse(FallDirection);

	UE_LOG(LogTemp, Log, TEXT("[TowerTrigger] 차량 진입 감지 → %s 붕괴 시작"),
		   *TargetTower->GetName());
}
