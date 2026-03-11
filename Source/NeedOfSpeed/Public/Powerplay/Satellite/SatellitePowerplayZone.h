#pragma once

#include "CoreMinimal.h"
#include "Powerplay/PowerplayZoneBase/PowerplayZoneBase.h"
#include "Powerplay/Satellite/SatellitePoleActor.h"
#include "SatellitePowerplayZone.generated.h"

UCLASS()
class NEEDOFSPEED_API ASatellitePowerplayZone : public APowerplayZoneBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Satellite")
	TObjectPtr<ASatellitePoleActor> TargetPole;

protected:
	virtual void OnPowerplayTriggered() override;
};
