#include "Powerplay/Satellite/SatellitePowerplayZone.h"

void ASatellitePowerplayZone::OnPowerplayTriggered()
{
	if (IsValid(TargetPole))
	{
		TargetPole->TriggerCollapse();
	}
}
