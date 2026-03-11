#pragma once

#include "CoreMinimal.h"
#include "Powerplay/PowerplayZoneBase/PowerplayZoneBase.h"
#include "Powerplay/Bus/BusExplosionActor.h"
#include "BusPowerplayZone.generated.h"

UCLASS()
class NEEDOFSPEED_API ABusPowerplayZone : public APowerplayZoneBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bus")
	TObjectPtr<ABusExplosionActor> TargetBus;

protected:
	virtual void OnPowerplayTriggered() override;
};
