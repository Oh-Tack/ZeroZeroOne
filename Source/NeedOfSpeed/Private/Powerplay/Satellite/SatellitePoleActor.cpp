// Fill out your copyright notice in the Description page of Project Settings.


#include "Powerplay/Satellite/SatellitePoleActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ASatellitePoleActor::ASatellitePoleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	PoleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleMesh"));
	SetRootComponent(PoleMesh);
	PoleMesh->SetSimulatePhysics(false);
	PoleMesh->SetEnableGravity(false);
	PoleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PoleMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

// Called when the game starts or when spawned
void ASatellitePoleActor::BeginPlay()
{
	Super::BeginPlay();

	// 폴 바닥을 회전 피벗으로 저장
	const FBoxSphereBounds Bounds = PoleMesh->CalcBounds(PoleMesh->GetComponentTransform());
	CollapseBasePoint = GetActorLocation();
	CollapseBasePoint.Z = Bounds.Origin.Z - Bounds.BoxExtent.Z;
}

// Called every frame
void ASatellitePoleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCollapsing) return;

	CollapseSpeed += CollapseAngularAcceleration * DeltaTime;
	const float DeltaAngle = CollapseSpeed * DeltaTime;
	CurrentCollapseAngle += DeltaAngle;

	const FQuat DeltaRot(CollapseAxis, FMath::DegreesToRadians(DeltaAngle));
	const FVector NewPos = CollapseBasePoint + DeltaRot.RotateVector(GetActorLocation() - CollapseBasePoint);
	const FQuat NewRot = DeltaRot * GetActorQuat();
	SetActorLocationAndRotation(NewPos, NewRot);

	// 80도 기울면 표지판 굴러가게
	if (CurrentCollapseAngle >= 80.f)
	{
		bIsCollapsing = false;
		SetActorTickEnabled(false);

		/*FTimerHandle SignHandle;
		GetWorld()->GetTimerManager().SetTimer(SignHandle, [this]()
		{
			if (IsValid(SignActor))
			{
				SignActor->TriggerRoll();
			}
		}, SignReleaseDelay, false);*/
	}
}

void ASatellitePoleActor::TriggerCollapse()
{
	if (bHasCollapsed) return;
	bHasCollapsed = true;

	if (CollapseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollapseSound, GetActorLocation());
	}

	if (CollapseNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CollapseNiagara, GetActorLocation());
	}

	// FallDirection 기준으로 회전축 계산 (타워와 동일 방식)
	FVector Dir = FallDirection;
	Dir.Z = 0.f;
	CollapseAxis = FVector::CrossProduct(FVector::UpVector, Dir.GetSafeNormal()).GetSafeNormal();

	bIsCollapsing = true;
	SetActorTickEnabled(true);
	
	// 키 입력 즉시 표지판 굴러가게
	FTimerHandle SignHandle;
	GetWorld()->GetTimerManager().SetTimer(SignHandle, [this]()
	{
		if (IsValid(SignActor))
		{
			SignActor->TriggerRoll();
		}
	}, SignReleaseDelay, false);
}


