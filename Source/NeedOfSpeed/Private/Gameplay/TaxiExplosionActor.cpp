// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedOfSpeed/Public/Gameplay/TaxiExplosionActor.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "AI/CPP_AI_McLaren.h"

// Sets default values
ATaxiExplosionActor::ATaxiExplosionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TaxiMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaxiMesh"));
	SetRootComponent(TaxiMesh);
	
	// 물리 비활성화
	TaxiMesh->SetSimulatePhysics(false);
	TaxiMesh->SetEnableGravity(false);
	
	// collision 설정
	TaxiMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TaxiMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

// Called when the game starts or when spawned
void ATaxiExplosionActor::BeginPlay()
{
	Super::BeginPlay();
	
	/*APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc)
	{
		EnableInput(pc);
		if (InputComponent)
		{
			FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATaxiExplosionActor::TriggerExplosion);
			Binding.bConsumeInput = false;
		}
	}*/
}

// Called every frame
void ATaxiExplosionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATaxiExplosionActor::TriggerExplosion()
{
	if (bHasExploded) return;
	bHasExploded = true;
	
	// 가장 가까운 AI 차 방향으로 발사 방향 계산
	if (bLaunchTowardNearestAI)
	{
		TArray<AActor*> AIActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_AI_McLaren::StaticClass(), AIActors);

		AActor* NearestAI = nullptr;
		float NearestDist = FLT_MAX;
		for (AActor* Actor : AIActors)
		{
			const float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Dist < NearestDist)
			{
				NearestDist = Dist;
				NearestAI = Actor;
			}
		}

		if (IsValid(NearestAI))
		{
			const FVector ToAI = (NearestAI->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			ComputedLaunchDirection = FVector(ToAI.X, ToAI.Y, 0.5f).GetSafeNormal();
		}
		else
		{
			ComputedLaunchDirection = LaunchDirection.GetSafeNormal();
		}
	}
	else
	{
		ComputedLaunchDirection = LaunchDirection.GetSafeNormal();
	}
	
	// 이펙트
	if (ExplosionParticle)
	{
		UGameplayStatics::SpawnEmitterAttached(
			ExplosionParticle,
			TaxiMesh,
			NAME_None,
			ParticleSpawnOffset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			/*bAutoDestroy=*/true);
	}
	
	if (ExplosionNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			ExplosionNiagara,
			TaxiMesh,
			NAME_None,
			ParticleSpawnOffset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			/*bAutoDestroy=*/true);
	}
	
	// 딜레이 후 날리기
	FTimerHandle LaunchDelayHandle;
	GetWorld()->GetTimerManager().SetTimer(LaunchDelayHandle, this, &ATaxiExplosionActor::ExecuteLaunch, LaunchDelay, false);
}

void ATaxiExplosionActor::ExecuteLaunch()
{
	// 물리 활성화
	TaxiMesh->SetLinearDamping(0.f);
	TaxiMesh->SetAngularDamping(0.f); 
	TaxiMesh->SetEnableGravity(true);
	TaxiMesh->SetSimulatePhysics(true);
	TaxiMesh->WakeAllRigidBodies(); 
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		TaxiMesh->SetPhysicsLinearVelocity(ComputedLaunchDirection * LinearImpulseStrength);
		TaxiMesh->SetPhysicsAngularVelocityInDegrees(AngularImpulseDegrees);
	});
}

