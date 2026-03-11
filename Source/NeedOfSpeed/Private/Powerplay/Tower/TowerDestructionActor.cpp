// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Tower/TowerDestructionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATowerDestructionActor::ATowerDestructionActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	GCComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GCComp"));
	RootComponent = GCComp;
	GCComp->SetSimulatePhysics(false);
	GCComp->SetEnableGravity(false);
	GCComp->SetEnableDamageFromCollision(false);
}

// Called when the game starts or when spawned
void ATowerDestructionActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 타워 바닥 중심을 회전 피벗으로 저장
	const FBoxSphereBounds Bounds = GCComp->CalcBounds(GCComp->GetComponentTransform());
	CollapseBasePoint   = GetActorLocation();
	CollapseBasePoint.Z = Bounds.Origin.Z - Bounds.BoxExtent.Z;
	
	// CollapseAxis = GetActorForwardVector();
}

void ATowerDestructionActor::StartCollapse(FVector FallDirection)
{
	if (bHasCollapsed) return;
	bHasCollapsed = true;

	// 회전축 (넘어질방향)
	FallDirection.Z = 0.f;
	CollapseAxis = FVector::CrossProduct(FVector::UpVector, FallDirection.GetSafeNormal()).GetSafeNormal();
	
	if (IsValid(ExplosionFX))
	{
		const FBoxSphereBounds Bounds = GCComp->CalcBounds(GCComp->GetComponentTransform());
		const float ForwardExtent = FMath::Abs(FVector::DotProduct(Bounds.BoxExtent, GetActorForwardVector()));
		const FVector FrontLocation = GCComp->GetComponentLocation() - GetActorForwardVector() * ForwardExtent - GetActorRightVector() * ExplosionLeftOffset;
		// const FVector FrontLocation = GCComp->GetComponentLocation() - GetActorForwardVector() * ForwardExtent;
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ExplosionFX,
			FrontLocation
			//GCComp->GetComponentLocation()
		);
	}
	
	if (CollapseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollapseSound, GetActorLocation());
	}

	bIsCollapsing = true;
	SetActorTickEnabled(true);
}

void ATowerDestructionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCollapsing) return;

	// 중력처럼 각속도를 점점 가속
	CollapseSpeed += CollapseAngularAcceleration * DeltaTime;
	const float DeltaAngle = CollapseSpeed * DeltaTime;
	CurrentCollapseAngle += DeltaAngle;

	// 피벗(타워 바닥) 기준으로 회전 적용
	const FQuat  DeltaRot(CollapseAxis, FMath::DegreesToRadians(DeltaAngle));
	const FVector NewPos = CollapseBasePoint + DeltaRot.RotateVector(GetActorLocation() - CollapseBasePoint);
	const FQuat  NewRot  = DeltaRot * GetActorQuat();
	SetActorLocationAndRotation(NewPos, NewRot);

	// 땅에 닿는 각도 도달 → fracture
	if (CurrentCollapseAngle >= FractureTriggerAngle)
	{
		bIsCollapsing = false;
		SetActorTickEnabled(false);
		TriggerGroundFracture();
	}
}

void ATowerDestructionActor::TriggerGroundFracture()
{
	// 바닥 충돌 이펙트
	if (IsValid(CollapseImpactFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), CollapseImpactFX,
			GCComp->GetComponentLocation()
		);
	}

	// 돌멩이 파편 이펙트
	if (IsValid(DebrisFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), DebrisFX,
			GCComp->GetComponentLocation()
		);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, GetActorLocation());
	}
	
	// 부서질 때 재질 변경
	if (IsValid(FracturedMaterial))
	{
		GCComp->SetMaterial(0, FracturedMaterial);
	}
	
	// 물리 + 파괴 활성화
	GCComp->SetEnableGravity(true);
	GCComp->SetEnableDamageFromCollision(true);
	GCComp->SetSimulatePhysics(true);

	// 파편 날리는 radial impulse (bVelChange=true: 질량 무관하게 속도 직접 적용)
	GCComp->AddRadialImpulse(
		GCComp->GetComponentLocation(),
		DestructionRadialRadius,
		DestructionRadialStrength,
		ERadialImpulseFalloff::RIF_Linear,
		true
	);
}
