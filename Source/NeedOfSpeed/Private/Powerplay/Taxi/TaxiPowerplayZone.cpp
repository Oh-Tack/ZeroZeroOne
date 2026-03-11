#include "Powerplay/Taxi/TaxiPowerplayZone.h"

void ATaxiPowerplayZone::OnPowerplayTriggered()
{
	if (CurrentTaxiIndex >= Taxis.Num()) return;

	ATaxiExplosionActor* Taxi = Taxis[CurrentTaxiIndex];
	if (IsValid(Taxi))
	{
		Taxi->TriggerExplosion();
	}

	CurrentTaxiIndex++;

	// 아직 남은 택시 있으면 bTriggered 리셋 → 다음 E 누를 수 있게
	if (CurrentTaxiIndex < Taxis.Num())
	{
		bTriggered = false;
		FTimerHandle IconReshowHandle;
		GetWorld()->GetTimerManager().SetTimer(IconReshowHandle, this, &ATaxiPowerplayZone::ShowIconWidget, 0.5f, false);
	}
}
