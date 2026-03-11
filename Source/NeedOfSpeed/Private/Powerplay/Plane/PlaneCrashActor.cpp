#include "Powerplay/Plane/PlaneCrashActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"

APlaneCrashActor::APlaneCrashActor()
{
	PrimaryActorTick.bCanEverTick = true;

	FlightSpline = CreateDefaultSubobject<USplineComponent>(TEXT("FlightSpline"));
	SetRootComponent(FlightSpline);
}

void APlaneCrashActor::BeginPlay()
{
	Super::BeginPlay();

	// 시작하자마자 스플라인 시작점에 정확히 배치
	AlignPlaneToSplineStart(true);

	/*// 대기 상태에서는 Tick 끔
	SetActorTickEnabled(false);*/
}

void APlaneCrashActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(PlaneActor) || !IsValid(FlightSpline))
	{
		return;
	}

	if (CurrentPhase == EPlaneCrashPhase::Sliding)
	{
		ExecuteSliding(DeltaTime);
		return;
	}

	if (CurrentPhase == EPlaneCrashPhase::Idle)
	{
		AlignPlaneToSplineStart(true);
		return;
	}
	
	if (CurrentPhase != EPlaneCrashPhase::Approaching)
	{
		return;
	}

	ApproachElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(ApproachElapsed / ApproachDuration, 0.f, 1.f);

	const float SplineLength = FlightSpline->GetSplineLength();
	const float Distance = Alpha * SplineLength;

	const FVector NewLocation =
		FlightSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	const FVector SplineDir =
		FlightSpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	PlaneActor->SetActorLocation(NewLocation);

	if (!SplineDir.IsNearlyZero())
	{
		const FRotator FinalRot = MakeApproachRotation(SplineDir, Alpha);
		PlaneActor->SetActorRotation(FinalRot);
	}

	// 지정 포인트 도달 시 이펙트만 스폰 (스플라인 이동 계속)
	if (!bEffectsSpawned && ImpactSplinePointIndex >= 0)
	{
		const float ImpactDist = FlightSpline->GetDistanceAlongSplineAtSplinePoint(ImpactSplinePointIndex);
		if (Distance >= ImpactDist)
		{
			SpawnImpactEffects();
			bEffectsSpawned = true;
		}
	}

	// 스플라인 끝 도달 시 슬라이딩 시작
	if (Alpha >= 1.f)
	{
		ExecuteImpact();
	}
}

void APlaneCrashActor::TriggerCrash()
{
	if (CurrentPhase != EPlaneCrashPhase::Idle)
	{
		return;
	}

	// 트리거 순간 시작점으로 다시 정확히 맞춰서
	// 순간이동/각도 꼬임 느낌 최소화
	AlignPlaneToSplineStart(true);

	CurrentPhase = EPlaneCrashPhase::Approaching;
	ApproachElapsed = 0.f;
	bEffectsSpawned = false;
	SetActorTickEnabled(true);

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
		FTimerHandle SoundDelayHandle;
        		GetWorld()->GetTimerManager().SetTimer(SoundDelayHandle, [this]()
        		{
        			if (IsValid(PlaneActor) && ApproachSound)
        			{
        				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ApproachSound, PlaneActor->GetActorLocation());
        			}
        		}, ApproachSoundDelay, false);
	}

	OnApproachStart();
}

void APlaneCrashActor::SpawnImpactEffects()
{
	if (!IsValid(PlaneActor))
	{
		return;
	}

	// 뒤쪽 스파크/불 이펙트: PlaneActor에 붙여서 계속 따라가게
	if (ImpactNiagara && !ActiveImpactNiagaraComp)
	{
		ActiveImpactNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			ImpactNiagara,
			PlaneActor->GetRootComponent(),
			NAME_None,
			SlidingSparkOffset,   // 뒤쪽 위치 오프셋
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (ImpactParticle && !ActiveImpactParticleComp)
	{
		ActiveImpactParticleComp = UGameplayStatics::SpawnEmitterAttached(
			ImpactParticle,
			PlaneActor->GetRootComponent(),
			NAME_None,
			SlidingSparkOffset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// 사운드/카메라쉐이크는 한 번만
	const FVector EffectLocation = PlaneActor->GetActorLocation();

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ImpactSound,
			EffectLocation);
	}

	if (ImpactCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			ImpactCameraShake,
			EffectLocation,
			0.f,
			5000.f);
	}

	OnImpact();
}

void APlaneCrashActor::ExecuteImpact()
{
	if (!IsValid(PlaneActor)) return;

	// 이펙트가 아직 안 터졌으면 여기서도 스폰
	if (!bEffectsSpawned)
	{
		SpawnImpactEffects();
		bEffectsSpawned = true;
	}
	
	LandingLocation = PlaneActor->GetActorLocation();
	LandingRotation = PlaneActor->GetActorRotation();

	// 착지 직후 너무 박히지 않게 정리
	LandingRotation.Pitch = FMath::Clamp(LandingRotation.Pitch, -5.f, 5.f);
	LandingRotation.Roll = 0.f;
	PlaneActor->SetActorRotation(LandingRotation);

	const FVector EndDir = FlightSpline->GetDirectionAtSplinePoint(
		FlightSpline->GetNumberOfSplinePoints() - 1,
		ESplineCoordinateSpace::World);

	SlideDirection = FVector(EndDir.X, EndDir.Y, 0.f).GetSafeNormal();

	PlaneActor->SetActorEnableCollision(true);
	
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
		const float CurrentSpeed = SlidingSpeed * FMath::InterpEaseOut(1.f, 0.f, Alpha, 2.f);

		FVector NewLoc = PlaneActor->GetActorLocation() + SlideDirection * CurrentSpeed * DeltaTime;
		NewLoc.Z = LandingLocation.Z;
		PlaneActor->SetActorLocation(NewLoc);

		// 슬라이딩하면서 기수만 수평 복귀
		FRotator LevelRotation = LandingRotation;
		LevelRotation.Pitch = 0.f;
		LevelRotation.Roll = 0.f;

		PlaneActor->SetActorRotation(FMath::Lerp(LandingRotation, LevelRotation, Alpha));
	}

	if (Alpha >= 1.f)
	{
		CurrentPhase = EPlaneCrashPhase::Impacted;
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Log, TEXT("[PlaneCrash] 슬라이딩 완료"));
	}
}

void APlaneCrashActor::AlignPlaneToSplineStart(bool bApplyNoseDown)
{
	if (!IsValid(PlaneActor) || !IsValid(FlightSpline))
	{
		return;
	}

	const FVector StartPos =
		FlightSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

	const FVector StartDir =
		FlightSpline->GetDirectionAtSplinePoint(0, ESplineCoordinateSpace::World);

	PlaneActor->SetActorLocation(StartPos);

	if (!StartDir.IsNearlyZero())
	{
		const float StartAlpha = bApplyNoseDown ? 0.f : 1.f;
		const FRotator StartRot = MakeApproachRotation(StartDir, StartAlpha);
		PlaneActor->SetActorRotation(StartRot);
	}
}

FRotator APlaneCrashActor::MakeApproachRotation(const FVector& Direction, float Alpha) const
{
	const FVector FlatDir(Direction.X, Direction.Y, 0.f);

	if (FlatDir.IsNearlyZero())
	{
		return FRotator::ZeroRotator;
	}

	// 수평 방향 기반 기본 회전 (Yaw만 결정, Roll/Pitch 없음)
	FRotator BaseRot = FlatDir.GetSafeNormal().Rotation() + MeshRotationOffset;
	BaseRot.Pitch = 0.f;
	BaseRot.Roll = 0.f;

	const FQuat BaseQuat = BaseRot.Quaternion();

	// 시작할 때는 nose down, 끝으로 갈수록 0으로 복귀
	const float PitchOffset = FMath::InterpEaseOut(NoseDownPitch, 0.f, Alpha, 2.f);

	// MeshRotationOffset.Yaw=-90° 때문에 코(nose) 방향이 actor local +Y(Right)에 해당함.
	// 따라서 날개 축은 actor local +X(Forward)를 사용해야 올바른 Pitch 회전이 됨.
	const FVector WingAxis = BaseQuat.GetForwardVector();
	const FQuat PitchQuat(WingAxis, FMath::DegreesToRadians(PitchOffset));

	return (PitchQuat * BaseQuat).Rotator();
}