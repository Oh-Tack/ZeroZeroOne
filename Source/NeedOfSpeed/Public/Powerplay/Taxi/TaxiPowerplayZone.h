#pragma once

#include "CoreMinimal.h"
#include "Powerplay/PowerplayZoneBase/PowerplayZoneBase.h"
#include "Powerplay/Taxi/TaxiExplosionActor.h"
#include "TaxiPowerplayZone.generated.h"

UCLASS()
class NEEDOFSPEED_API ATaxiPowerplayZone : public APowerplayZoneBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Taxis")
	TArray<TObjectPtr<ATaxiExplosionActor>> Taxis;

protected:
	// 택시는 E 누를 때마다 순차 발동 (bTriggered 안 씀)
	virtual void OnPowerplayTriggered() override;

private:
	int32 CurrentTaxiIndex = 0;
};
