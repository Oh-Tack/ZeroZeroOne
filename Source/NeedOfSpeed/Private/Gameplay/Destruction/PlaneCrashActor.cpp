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

	// 비행기를 스플라인 시작 지점에 배치
	if (IsValid(PlaneActor))
	{
		const FVector StartPos = FlightSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		const FVector StartDir = FlightSpline->GetDirectionAtSplinePoint(0, ESplineCoordinateSpace::World);
		PlaneActor->SetActorLocation(StartPos);
		if (!StartDir.IsNearlyZero())
		{
			PlaneActor->SetActorRotation(StartDir.Rotation() + MeshRotationOffset);
		}
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
		const FVector EndPos = FlightSpline->GetLocationAtSplinePoint(FlightSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
		
		PlaneActor->SetActorLocation(NewLocation);
		if (!SplineDir.IsNearlyZero())
		{
			// Pitch는 스플라인 접선에서, Yaw는 항상 착지 지점 방향으로 고정
			const FVector ToEnd = (EndPos - NewLocation).GetSafeNormal();
			FRotator Rot = SplineDir.Rotation();
			Rot.Yaw = ToEnd.Rotation().Yaw;
			// 착지 직전 기수 수평 복귀 (Alpha 0→1 갈수록 틸트 감소)
			const float TiltScale = FMath::InterpEaseOut(1.f, 0.f, Alpha, 2.f);
			PlaneActor->SetActorRotation(Rot + MeshRotationOffset + ApproachTiltOffset * TiltScale);
			
			/*FQuat BaseQuat(Rot + MeshRotationOffset);
			// 메시 로컬 Right 축 기준으로 기수 하향 적용
			FQuat NoseDownQuat(BaseQuat.GetAxisX(), FMath::DegreesToRadians(NoseDownPitch));
			PlaneActor->SetActorRotation((NoseDownQuat * BaseQuat).Rotator());*/
			
			/*Rot.Pitch += NoseDownPitch; // 강하 중 기수 하향
			PlaneActor->SetActorRotation(Rot + MeshRotationOffset);*/
			
			// PlaneActor->SetActorRotation(Direction.Rotation() + MeshRotationOffset);
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

	CurrentPhase = EPlaneCrashPhase::Approaching;
	ApproachElapsed = 0.f;
	SetActorTickEnabled(true);

	// 연기 트레일
	if (TrailNiagara && IsValid(PlaneActor))
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailNiagara,
			PlaneActor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
	}

	if (ApproachSound && IsValid(PlaneActor))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ApproachSound, PlaneActor->GetActorLocation());
	}

	OnApproachStart();
}

void APlaneCrashActor::ExecuteImpact()
{
	if (!IsValid(PlaneActor)) return;

	LandingLocation = PlaneActor->GetActorLocation();
	LandingRotation = PlaneActor->GetActorRotation();

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
		const FRotator LevelRotation = FRotator(0.f, LandingRotation.Yaw, LandingRotation.Roll);
		PlaneActor->SetActorRotation(FMath::Lerp(LandingRotation, LevelRotation, Alpha));
	}

	if (Alpha >= 1.f)
	{
		CurrentPhase = EPlaneCrashPhase::Impacted;
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Log, TEXT("[PlaneCrash] 슬라이딩 완료"));
	}
}
