#include "NeedOfSpeed/Public/AI/CPP_FinishVFX.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// Sets default values
ACPP_FinishVFX::ACPP_FinishVFX()
{
	PrimaryActorTick.bCanEverTick = false;

	// =========================
	// Trigger Box
	// =========================
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	TriggerBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACPP_FinishVFX::OnOverlapBegin);

	// =========================
	// Effect Points (7개 생성)
	// =========================
	for (int i = 0; i < 7; ++i)
	{
		FName Name = *FString::Printf(TEXT("EffectPoint_%d"), i);

		USceneComponent* Point =
			CreateDefaultSubobject<USceneComponent>(Name);

		Point->SetupAttachment(RootComponent);

		EffectPoints.Add(Point);
	}
}

// Called when the game starts
void ACPP_FinishVFX::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACPP_FinishVFX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


// ======================================================
// Overlap
// ======================================================

void ACPP_FinishVFX::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 이미 실행됐으면 종료
	if (bTriggered) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (OtherActor != PlayerPawn) return;

	bTriggered = true;

	if (EffectPoints.Num() < 7) return;

	// =========================
	// Dust (0)
	// =========================
	if (VFX_Dust)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFX_Dust,
			EffectPoints[0]->GetComponentLocation());
	}

	// =========================
	// Left Smoke (1~3)
	// =========================
	UNiagaraSystem* LeftFX[3] =
	{
		VFX_SmokeL1,
		VFX_SmokeL2,
		VFX_SmokeL3
	};

	for (int i = 0; i < 3; ++i)
	{
		if (LeftFX[i])
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				LeftFX[i],
				EffectPoints[i + 1]->GetComponentLocation());
		}
	}

	// =========================
	// Right Smoke (4~6)
	// =========================
	UNiagaraSystem* RightFX[3] =
	{
		VFX_SmokeR1,
		VFX_SmokeR2,
		VFX_SmokeR3
	};

	for (int i = 0; i < 3; ++i)
	{
		if (RightFX[i])
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				RightFX[i],
				EffectPoints[i + 4]->GetComponentLocation());
		}
	}

	// =========================
	// Sounds
	// =========================
	if (ClapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ClapSound,
			EffectPoints[0]->GetComponentLocation());
	}

	if (FireworksSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireworksSound,
			GetActorLocation());
	}
}