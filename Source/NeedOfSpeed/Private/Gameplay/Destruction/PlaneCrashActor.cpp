// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Destruction/PlaneCrashActor.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
APlaneCrashActor::APlaneCrashActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlaneCrashActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlaneCrashActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPhase != EPlaneCrashPhase::Approaching) return;

	ApproachElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(ApproachElapsed / ApproachDuration, 0.f, 1.f);

	// EaseIn 커브 - 점점 빠르게 강하
	const float EasedAlpha = FMath::InterpEaseIn(0.f, 1.f, Alpha, 2.f);
	const FVector NewLocation = FMath::Lerp(ApproachStartLocation, ApproachTargetLocation, EasedAlpha);
	SetActorLocation(NewLocation);

	// 강하 방향으로 기체 회전 정렬
	const FVector Dir = (ApproachTargetLocation - ApproachStartLocation).GetSafeNormal();
	if (!Dir.IsNearlyZero())
	{
		SetActorRotation(Dir.Rotation());
	}

	if (Alpha >= 1.f)
	{
		ExecuteImpact();
	}
}

void APlaneCrashActor::TriggerCrash()
{
	if (CurrentPhase != EPlaneCrashPhase::Idle) return;

	ApproachTargetLocation = ImpactWorldLocation.IsNearlyZero()
		? GetActorLocation()
		: ImpactWorldLocation;

	ApproachStartLocation = ApproachTargetLocation + ApproachStartOffset;
	SetActorLocation(ApproachStartLocation);

	CurrentPhase = EPlaneCrashPhase::Approaching;
	ApproachElapsed = 0.f;
	SetActorTickEnabled(true);

	// BP에서 트레일 이펙트 등 처리
	OnApproachStart();

	UE_LOG(LogTemp, Log, TEXT("[PlaneCrash] 비행기 강하 시작: %s → %s"),
		*ApproachStartLocation.ToString(), *ApproachTargetLocation.ToString());
}

void APlaneCrashActor::ExecuteImpact()
{
	CurrentPhase = EPlaneCrashPhase::Impacted;
	SetActorTickEnabled(false);

	// 폭발 이펙트 스폰
	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), ImpactParticle,
			GetActorLocation() + ImpactParticleOffset,
			FRotator::ZeroRotator, ImpactParticleScale);
	}

	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ImpactNiagara,
			GetActorLocation() + ImpactParticleOffset,
			FRotator::ZeroRotator, ImpactParticleScale);
	}

	// BP에서 스켈레탈 메시 물리/애니메이션 처리
	OnImpact();

	UE_LOG(LogTemp, Log, TEXT("[PlaneCrash] 비행기 충돌!"));
}
