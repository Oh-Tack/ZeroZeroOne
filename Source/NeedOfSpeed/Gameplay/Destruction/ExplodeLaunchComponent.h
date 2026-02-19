// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExplodeLaunchComponent.generated.h"

class NiagaraSystem;
class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEEDOFSPEED_API UExplodeLaunchComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UExplodeLaunchComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	UFUNCTION( BlueprintCallable, Category = "Explosion" )
	void TriggerExplode();
	
	// mesh
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Explosion" )
	TObjectPtr<UStaticMeshComponent> TargetMesh = nullptr;
	
	// explosion effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion" )
	TObjectPtr<UNiagaraSystem> ExplosionFX = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	FVector FXOffset = FVector(0.01f, 0.01f, 0.01f);
	
	
};
