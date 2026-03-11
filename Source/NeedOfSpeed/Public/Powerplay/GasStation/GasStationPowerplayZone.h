#pragma once

#include "CoreMinimal.h"
#include "Powerplay/PowerplayZoneBase/PowerplayZoneBase.h"
#include "Powerplay/GasStation/GasStationExplosionActor.h"
#include "GasStationPowerplayZone.generated.h"

UCLASS()
class NEEDOFSPEED_API AGasStationPowerplayZone : public APowerplayZoneBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GasStation")
	TObjectPtr<AGasStationExplosionActor> TargetGasStation;

protected:
	virtual void OnPowerplayTriggered() override;
};
