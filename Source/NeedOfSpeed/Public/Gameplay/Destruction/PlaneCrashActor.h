// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "PlaneCrashActor.generated.h"

class UParticleSystem;

UENUM(BlueprintType)
enum class EPlaneCrashPhase : uint8
{
	Idle,
	Approaching,  // 하늘에서 충돌 지점으로 강하 중
	Impacted,     // 착지 충돌 후
};
UCLASS()
class NEEDOFSPEED_API APlaneCrashActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlaneCrashActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// 충돌 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TObjectPtr<UParticleSystem> ImpactParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TObjectPtr<UNiagaraSystem> ImpactNiagara;

	// 강하 중 연기 트레일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	TObjectPtr<UNiagaraSystem> TrailNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	FVector ImpactParticleOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Effects")
	FVector ImpactParticleScale = FVector(5.f, 5.f, 5.f);

	// 강하 설정 - 충돌 지점(ImpactWorldLocation) 기준 상대 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	FVector ApproachStartOffset = FVector(-3000.f, 0.f, 2000.f);

	// 강하에 걸리는 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	float ApproachDuration = 2.5f;

	// 레벨에서 설정할 충돌 착지 지점 (월드 좌표)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash|Approach")
	FVector ImpactWorldLocation = FVector::ZeroVector;
	
	// 레벨에 배치된 비행기 BP 액터를 직접 연결
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crash")
	TObjectPtr<AActor> PlaneActor;

	// 강하 시작 시 호출 - BP에서 트레일 이펙트 등 처리
	UFUNCTION(BlueprintImplementableEvent, Category = "Crash")
	void OnApproachStart();

	// 착지 충돌 시 호출 - BP에서 스켈레탈 메시 물리/애니메이션 처리
	UFUNCTION(BlueprintImplementableEvent, Category = "Crash")
	void OnImpact();

	UFUNCTION(BlueprintCallable, Category = "Crash")
	void TriggerCrash();

private:
	EPlaneCrashPhase CurrentPhase = EPlaneCrashPhase::Idle;
	float ApproachElapsed = 0.f;
	FVector ApproachStartLocation;
	FVector ApproachTargetLocation;

	void ExecuteImpact();
};
