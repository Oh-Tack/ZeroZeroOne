// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TowerMeshComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGeometryCollectionComponent> GCComp;
	
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void StartCollapse();
	
	bool bHasCollapsed = false;

private:
	// TowerMeshComp의 물리 충돌 감지 콜백 
	UFUNCTION()
	void OnTowerMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, FVector NormalImpulse,
						const FHitResult& Hit);
	
	void ActivateFracture(const FVector& ImpactPoint);
	
	bool bFractured = false;
	
	UPROPERTY(EditAnywhere, Category = "Tower|Physics", meta = (ClampMin = "0.0"))
	float CollapseAngularImpulse = 800000.0f;
	
	UPROPERTY(EditAnywhere, Category = "Tower|Physics", meta = (ClampMin = "0.0"))
	float FractureImpulseThreshold = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Tower|Physics", meta = (ClampMin = "0.0"))
	float FractureRadialStrength = 1500000.0f;
	
	UPROPERTY(EditAnywhere, Category = "Tower|Physics", meta = (ClampMin = "0.0"))
	float FractureRadialRadius = 500.0f;
};
