// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_FinishVFX.generated.h"

UCLASS()
class NEEDOFSPEED_API ACPP_FinishVFX : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPP_FinishVFX();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
protected:

	// ==============================
	// Trigger
	// ==============================

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> TriggerBox;

	// ==============================
	// Effect Spawn Points (7개)
	// ==============================

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<class USceneComponent>> EffectPoints;

	// ==============================
	// Niagara Effects
	// ==============================

	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> VFX_Dust;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> VFX_SmokeL1;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> VFX_SmokeL2;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> VFX_SmokeL3;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> VFX_SmokeR1;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> VFX_SmokeR2;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> VFX_SmokeR3;

	// ==============================
	// Sounds
	// ==============================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<class USoundBase> ClapSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<class USoundBase> FireworksSound;

	// ==============================
	// Trigger Control
	// ==============================

	bool bTriggered = false;

	// ==============================
	// Overlap
	// ==============================

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};