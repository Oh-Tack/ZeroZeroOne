// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/Gameplay/Destruction/TowerTriggerActor.h"
#include "NeedOfSpeed/Public/Gameplay/Destruction/TowerDestructionActor.h"


// Sets default values
ATowerTriggerActor::ATowerTriggerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	// 이벤트 바인딩
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ATowerTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 생성자에서 바인딩하면 World context 없이 실패하므로 BeginPlay에서 바인딩
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
	if (OtherActor && OtherActor->ActorHasTag("PlayerCar") && TargetTower)
    {
        TargetTower->StartPowerPlay(); 
        Destroy(); 
    }
}


