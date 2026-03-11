#include "Powerplay/PowerplayZoneBase/PowerplayZoneBase.h"
#include "Player/RacingCar.h"
#include "GameFramework/PlayerController.h"

APowerplayZoneBase::APowerplayZoneBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	RootComponent = ZoneBox;
	ZoneBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ZoneBox->SetGenerateOverlapEvents(true);
	ZoneBox->SetHiddenInGame(true);
}

void APowerplayZoneBase::BeginPlay()
{
	Super::BeginPlay();

	ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &APowerplayZoneBase::OnOverlapBegin);
	ZoneBox->OnComponentEndOverlap.AddDynamic(this, &APowerplayZoneBase::OnOverlapEnd);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		EnableInput(PC);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this, &APowerplayZoneBase::OnEPressed);
			InputComponent->BindKey(EKeys::RightControl, IE_Pressed, this, &APowerplayZoneBase::OnEPressed);
			
			// InputComponent->BindKey(EKeys::E, IE_Pressed, this, &APowerplayZoneBase::OnEPressed);
		}
		DisableInput(PC);
	}
}

void APowerplayZoneBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !OtherActor->IsA<ARacingCar>()) return;

	bPlayerInZone = true;
	EnableInput(GetWorld()->GetFirstPlayerController());
	ShowIconWidget();
}

void APowerplayZoneBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || !OtherActor->IsA<ARacingCar>()) return;

	bPlayerInZone = false;
	bTriggered = false;
	DisableInput(GetWorld()->GetFirstPlayerController());
	HideIconWidget();
}

void APowerplayZoneBase::OnEPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("[Powerplay] OnEPressed 호출됨"));
	if (!bPlayerInZone || bTriggered) return;

	bTriggered = true;
	HideIconWidget();
	ShowTextWidget();
	OnPowerplayTriggered();

	// TextDisplayDuration 후 텍스트 숨김
	FTimerHandle TextHandle;
	GetWorld()->GetTimerManager().SetTimer(TextHandle, this, &APowerplayZoneBase::HideTextWidget, TextDisplayDuration, false);
}

void APowerplayZoneBase::ShowIconWidget()
{
	if (!IconWidgetClass) return;
	if (!IconWidgetInstance)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		IconWidgetInstance = CreateWidget<UUserWidget>(PC, IconWidgetClass);
	}
	if (IconWidgetInstance) IconWidgetInstance->AddToViewport();
}

void APowerplayZoneBase::HideIconWidget()
{
	if (IconWidgetInstance) IconWidgetInstance->RemoveFromParent();
}

void APowerplayZoneBase::ShowTextWidget()
{
	if (!TextWidgetClass) return;
	if (!TextWidgetInstance)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		TextWidgetInstance = CreateWidget<UUserWidget>(PC, TextWidgetClass);
	}
	if (TextWidgetInstance) TextWidgetInstance->AddToViewport();
}

void APowerplayZoneBase::HideTextWidget()
{
	if (TextWidgetInstance) TextWidgetInstance->RemoveFromParent();
}
