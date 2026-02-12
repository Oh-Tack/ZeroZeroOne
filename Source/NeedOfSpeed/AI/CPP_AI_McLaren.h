#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "CPP_AI_McLaren.generated.h"

UCLASS()
class NEEDOFSPEED_API ACPP_AI_McLaren : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ACPP_AI_McLaren();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};