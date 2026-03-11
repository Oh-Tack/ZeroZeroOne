// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Taxi/TaxiPowerplayZone.h"
#include "Player/RacingCar.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ATaxiPowerplayZone::ATaxiPowerplayZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	RootComponent = ZoneBox;

	ZoneBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ZoneBox->SetGenerateOverlapEvents(true);
	ZoneBox->SetHiddenInGame(false);
}

// Called when the game starts or when spawned
void ATaxiPowerplayZone::BeginPlay()
{
	Super::BeginPlay();
	
	ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &ATaxiPowerplayZone::OnOverlapBegin);
	ZoneBox->OnComponentEndOverlap.AddDynamic(this, &ATaxiPowerplayZone::OnOverlapEnd);
	
	// 바인딩은 최초 1회만
	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc)
	{
		EnableInput(pc);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATaxiPowerplayZone::OnEPressed);
		}
		DisableInput(pc);
	}
}

// Called every frame
void ATaxiPowerplayZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATaxiPowerplayZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
					bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->IsA<ARacingCar>()) return;
	
	bPlayerInZone = true;
	EnableInput(GetWorld()->GetFirstPlayerController());
	// APlayerController* pc = GetWorld()->GetFirstPlayerController();
	// if (pc)
	// {
	// 	EnableInput(pc);
	// 	if (InputComponent)
	// 	{
	// 		FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATaxiPowerplayZone::OnEPressed);
	// 		Binding.bConsumeInput = false;
	// 	}
	// }
}

void ATaxiPowerplayZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->IsA<ARacingCar>()) return;
	
	bPlayerInZone = false;
	CurrentTaxiIndex = 0;
	DisableInput(GetWorld()->GetFirstPlayerController());
}

void ATaxiPowerplayZone::OnEPressed()
{
	if (!bPlayerInZone) return;
	if (CurrentTaxiIndex >= Taxis.Num()) return;
	
	ATaxiExplosionActor* Taxi = Taxis[CurrentTaxiIndex];
	if (IsValid(Taxi))
	{
		Taxi->TriggerExplosion();
	}
	
	CurrentTaxiIndex++;
}