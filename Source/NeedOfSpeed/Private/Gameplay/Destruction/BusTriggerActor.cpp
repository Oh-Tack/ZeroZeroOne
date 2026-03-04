// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Destruction/BusTriggerActor.h"
#include "Gameplay/Destruction/BusExplosionActor.h"
#include "Player/RacingCar.h"
#include "AI/CPP_AI_McLaren.h"

ABusTriggerActor::ABusTriggerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetHiddenInGame(false); // 디버깅용, 확인 후 true로
}

void ABusTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABusTriggerActor::OnOverlapBegin);
}

void ABusTriggerActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered) return;
	if (!IsValid(OtherActor)) return;

	// 플레이어 또는 AI 차량만 감지
	const bool bIsPlayer = OtherActor->IsA<ARacingCar>();
	const bool bIsAI = OtherActor->IsA<ACPP_AI_McLaren>();
	if (!bIsPlayer && !bIsAI) return;

	if (!IsValid(TargetBus))
	{
		UE_LOG(LogTemp, Error, TEXT("[BusTrigger] TargetBus가 설정되지 않음"));
		return;
	}

	bTriggered = true;
	UE_LOG(LogTemp, Log, TEXT("[BusTrigger] 차량 진입: %s → 버스 폭발"), *OtherActor->GetName());
	TargetBus->TriggerExplosion();
}
