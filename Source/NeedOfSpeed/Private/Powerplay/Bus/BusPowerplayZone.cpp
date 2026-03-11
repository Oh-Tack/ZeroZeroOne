// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Bus/BusPowerplayZone.h"
#include "Player/RacingCar.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ABusPowerplayZone::ABusPowerplayZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	RootComponent = ZoneBox;

	ZoneBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ZoneBox->SetGenerateOverlapEvents(true);
	ZoneBox->SetHiddenInGame(false); // 베타 전 true로 변경
}

// Called when the game starts or when spawned
void ABusPowerplayZone::BeginPlay()
{
	Super::BeginPlay();
	
	ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &ABusPowerplayZone::OnOverlapBegin);
	ZoneBox->OnComponentEndOverlap.AddDynamic(this, &ABusPowerplayZone::OnOverlapEnd);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		EnableInput(PC);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ABusPowerplayZone::OnEPressed);
		}
		DisableInput(PC);
	}
}

// Called every frame
void ABusPowerplayZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABusPowerplayZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !OtherActor->IsA<ARacingCar>()) return;

	bPlayerInZone = true;
	EnableInput(GetWorld()->GetFirstPlayerController());
}

void ABusPowerplayZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || !OtherActor->IsA<ARacingCar>()) return;

	bPlayerInZone = false;
	DisableInput(GetWorld()->GetFirstPlayerController());
}

void ABusPowerplayZone::OnEPressed()
{
	if (!bPlayerInZone || bTriggered) return;
	if (!IsValid(TargetBus)) return;

	bTriggered = true;
	TargetBus->TriggerExplosion();
}

