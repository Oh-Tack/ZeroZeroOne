// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Destruction/PlaneTriggerActor.h"
#include "Gameplay/Destruction/PlaneCrashActor.h"
#include "Player/RacingCar.h"
#include "AI/CPP_AI_McLaren.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlaneTriggerActor::APlaneTriggerActor()
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
void APlaneTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlaneTriggerActor::OnOverlapBegin);
}

// Called every frame
void APlaneTriggerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlaneTriggerActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered) return;
	if (!IsValid(OtherActor)) return;

	// 플레이어 또는 AI 차량만 감지
	const bool bIsPlayer = OtherActor->IsA<ARacingCar>();
	const bool bIsAI = OtherActor->IsA<ACPP_AI_McLaren>();
	if (!bIsPlayer && !bIsAI) return;

	if (!IsValid(TargetPlane))
	{
		UE_LOG(LogTemp, Error, TEXT("[PlaneTrigger] TargetPlane이 설정되지 않음"));
		return;
	}

	bTriggered = true;
	UE_LOG(LogTemp, Log, TEXT("[PlaneTrigger] 차량 진입: %s → 비행기 충돌 시작"), *OtherActor->GetName());
	APlaneCrashActor* CrashActor = Cast<APlaneCrashActor>(TargetPlane);
	if (IsValid(CrashActor))
	{
		CrashActor->TriggerCrash();
	}
	// TargetPlane->TriggerCrash();
}
