// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/GasStation/GasStationTriggerActor.h"
#include "Powerplay/GasStation/GasStationExplosionActor.h"
#include "Player/RacingCar.h"
#include "AI/CPP_AI_McLaren.h"

// Sets default values
AGasStationTriggerActor::AGasStationTriggerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetHiddenInGame(false); // 디버깅용, 확인 후 true로
}

// Called when the game starts or when spawned
void AGasStationTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGasStationTriggerActor::OnOverlapBegin);
}

// Called every frame
void AGasStationTriggerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGasStationTriggerActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered) return;
	if (!IsValid(OtherActor)) return;

	// 플레이어 또는 AI 차량만 감지
	const bool bIsPlayer = OtherActor->IsA<ARacingCar>();
	const bool bIsAI = OtherActor->IsA<ACPP_AI_McLaren>();
	if (!bIsPlayer && !bIsAI) return;

	if (!IsValid(TargetGasStation))
	{
		UE_LOG(LogTemp, Error, TEXT("[GasStationTrigger] TargetGasStation이 설정되지 않음"));
		return;
	}

	bTriggered = true;
	UE_LOG(LogTemp, Log, TEXT("[GasStationTrigger] 차량 진입: %s → 주유소 폭발"), *OtherActor->GetName());
	TargetGasStation->TriggerExplosion();
}