// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Destruction/PlaneCrashActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"

APlaneCrashActor::APlaneCrashActor()
{
	PrimaryActorTick.bCanEverTick = true;

	FlightSpline = CreateDefaultSubobject<USplineComponent>(TEXT("FlightSpline"));
	SetRootComponent(FlightSpline);
}

void APlaneCrashActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(PlaneActor))
	{
		const FVector StartPos = FlightSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

		PlaneActor->SetActorLocation(StartPos);
	}

	SetActorTickEnabled(false);
}

void APlaneCrashActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPhase == EPlaneCrashPhase::Sliding)
	{
		ExecuteSliding(DeltaTime);
		return;
	}

	if (CurrentPhase != EPlaneCrashPhase::Approaching) return;

	ApproachElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(ApproachElapsed / ApproachDuration, 0.f, 1.f);
	const float EasedAlpha = Alpha;

	const float SplineLength = FlightSpline->GetSplineLength();
	const float Distance = EasedAlpha * SplineLength;

	if (IsValid(PlaneActor))
	{
		const FVector NewLocation = FlightSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		// const FVector Direction = FlightSpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		const FVector SplineDir = FlightSpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		
		PlaneActor->SetActorLocation(NewLocation);
		if (!SplineDir.IsNearlyZero())
		{
			const FVector FlatSplineDir = FVector(SplineDir.X, SplineDir.Y, 0.f).GetSafeNormal();

			if (!FlatSplineDir.IsNearlyZero())
			{
				FRotator FinalRot = FlatSplineDir.Rotation() + MeshRotationOffset;

				const float PitchOffset = FMath::InterpEaseOut(NoseDownPitch, 0.f, Alpha, 2.f);
				FinalRot.Pitch += PitchOffset;
				FinalRot.Roll = 0.f;

				PlaneActor->SetActorRotation(FinalRot);
			}
		}
	}

	if (Alpha >= 1.f)
	{
		ExecuteImpact();
	}
}

void APlaneCrashActor::TriggerCrash()
{
	if (CurrentPhase != EPlaneCrashPhase::Idle) return;

	if (IsValid(PlaneActor))
	{
		const FVector StartPos = FlightSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		const FVector StartDir = FlightSpline->GetDirectionAtSplinePoint(0, ESplineCoordinateSpace::World);
		const FVector FlatDir = FVector(StartDir.X, StartDir.Y, 0.f).GetSafeNormal();

		PlaneActor->SetActorLocation(StartPos);

		if (!FlatDir.IsNearlyZero())
		{
			FRotator Rot = FlatDir.Rotation() + MeshRotationOffset;
			Rot.Pitch += NoseDownPitch;
			Rot.Roll = 0.f;

			PlaneActor->SetActorRotation(Rot);
		}
	}

	CurrentPhase = EPlaneCrashPhase::Approaching;
	ApproachElapsed = 0.f;
	SetActorTickEnabled(true);
}

void APlaneCrashActor::ExecuteImpact()
{
	if (!IsValid(PlaneActor)) return;

	LandingLocation = PlaneActor->GetActorLocation();
	LandingRotation = PlaneActor->GetActorRotation();

	// 너무 박힌 상태로 시작하지 않도록 완화
	LandingRotation.Pitch = FMath::Clamp(LandingRotation.Pitch, -5.f, 5.f);
	LandingRotation.Roll = 0.f;

	PlaneActor->SetActorRotation(LandingRotation);
	
	// 스플라인 끝 방향의 수평 성분 → 슬라이딩 방향
	const FVector EndDir = FlightSpline->GetDirectionAtSplinePoint(
		FlightSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
	SlideDirection = FVector(EndDir.X, EndDir.Y, 0.f).GetSafeNormal();

	// 콜리전 끄기
	PlaneActor->SetActorEnableCollision(false);

	// 스파크
	if (SlidingSparkNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			SlidingSparkNiagara,
			PlaneActor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
	}

	// 이펙트
	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), ImpactParticle,
			LandingLocation + ImpactParticleOffset,
			FRotator::ZeroRotator, ImpactParticleScale);
	}

	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ImpactNiagara,
			LandingLocation + ImpactParticleOffset,
			FRotator::ZeroRotator, ImpactParticleScale);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, LandingLocation);
	}

	if (ImpactCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), ImpactCameraShake, LandingLocation, 0.f, 5000.f);
	}

	OnImpact();

	CurrentPhase = EPlaneCrashPhase::Sliding;
	SlidingElapsed = 0.f;

	UE_LOG(LogTemp, Log, TEXT("[PlaneCrash] 비행기 충돌!"));
}

void APlaneCrashActor::ExecuteSliding(float DeltaTime)
{
	SlidingElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(SlidingElapsed / SlidingDuration, 0.f, 1.f);

	if (IsValid(PlaneActor))
	{
		// 감속
		const float CurrentSpeed = SlidingSpeed * FMath::InterpEaseOut(1.f, 0.f, Alpha, 2.f);
		FVector NewLoc = PlaneActor->GetActorLocation() + SlideDirection * CurrentSpeed * DeltaTime;
		NewLoc.Z = LandingLocation.Z; // Z 고정
		PlaneActor->SetActorLocation(NewLoc);

		// 기수 수평 복귀
		const FRotator LevelRotation = FRotator(0.f, LandingRotation.Yaw, 0.f);
		// const FRotator LevelRotation = FRotator(0.f, LandingRotation.Yaw, LandingRotation.Roll);
		PlaneActor->SetActorRotation(FMath::Lerp(LandingRotation, LevelRotation, Alpha));
	}

	if (Alpha >= 1.f)
	{
		CurrentPhase = EPlaneCrashPhase::Impacted;
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Log, TEXT("[PlaneCrash] 슬라이딩 완료"));
	}
}
