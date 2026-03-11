#include "Powerplay/GasStation/GasStationPowerplayZone.h"

void AGasStationPowerplayZone::OnPowerplayTriggered()
{
	if (IsValid(TargetGasStation))
	{
		TargetGasStation->TriggerExplosion();
	}
}
