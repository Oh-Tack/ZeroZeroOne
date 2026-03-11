// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Powerplay/PowerplayZoneBase/PowerplayZoneBase.h"
#include "Powerplay/Plane/PlaneCrashActor.h"
#include "GameFramework/Actor.h"
#include "PlanePowerplayZone.generated.h"

UCLASS()
class NEEDOFSPEED_API APlanePowerplayZone : public APowerplayZoneBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plane")
	TObjectPtr<APlaneCrashActor> TargetPlane;

protected:
	virtual void OnPowerplayTriggered() override;
};
