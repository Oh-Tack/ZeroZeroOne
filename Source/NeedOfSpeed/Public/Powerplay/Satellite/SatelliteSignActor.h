// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "SatelliteSignActor.generated.h"

UCLASS()
class NEEDOFSPEED_API ASatelliteSignActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASatelliteSignActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SignMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll")
	FVector RollDirection = FVector(0.f, 1.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll")
	float ImpulseStrength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll")
	FVector AngularVelocity = FVector(-400.f, 0.f, 0.f);

	// 2~3바퀴 후 멈추는 감속값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll")
	float AngularDamping = 1.5f;
	
	// 굴러가면서 땅에 닿을 때 스파크 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll|FX")
	TObjectPtr<UNiagaraSystem> SparkFX;

	UFUNCTION(BlueprintCallable, Category = "Roll")
	void TriggerRoll();

private:
	bool bHasRolled = false;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			   FVector NormalImpulse, const FHitResult& Hit);
};
