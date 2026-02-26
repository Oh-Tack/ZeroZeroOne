// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "NiagaraSystem.h"
#include "TowerDestructionActor.generated.h"

UCLASS()
class NEEDOFSPEED_API ATowerDestructionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATowerDestructionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGeometryCollectionComponent> GCComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|FX")
	TObjectPtr<UNiagaraSystem> ExplosionFX;
	
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void StartCollapse();
	
	bool bHasCollapsed = false;
	
private:
	void TriggerGroundFracture();

	bool bIsCollapsing = false;
	float CurrentCollapseAngle = 0.f;
	float CollapseSpeed = 0.f;   // 현재 각속도 (degrees/sec)
	FVector CollapseBasePoint;   // 회전 피벗 = 타워 바닥 중심
	FVector CollapseAxis;        // 회전 축 = Actor 오른쪽 벡터 (앞으로 쓰러지는 방향)

	// 각가속도 (degrees/sec²). 클수록 빨리 넘어짐
	UPROPERTY(EditAnywhere, Category = "Tower|Physics", meta = (ClampMin = "0.0"))
	float CollapseAngularAcceleration = 40.f;

	// 이 각도에 도달하면 땅에 닿은 것으로 판단 → fracture 발동
	UPROPERTY(EditAnywhere, Category = "Tower|Physics", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float FractureTriggerAngle = 85.f;

	// fracture 때 파편을 날리는 radial impulse 강도
	UPROPERTY(EditAnywhere, Category = "Tower|Physics", meta = (ClampMin = "0.0"))
	float DestructionRadialStrength = 800.f;
};
