#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIn_isPath.generated.h"

// Interface marker class
UINTERFACE(MinimalAPI)
class UUIn_isPath : public UInterface
{
	GENERATED_BODY()
};

// Actual interface
class NEEDOFSPEED_API IUIn_isPath
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Vehicle")
	void GetClosestLocationToPath(
		FVector AILocation,
		float AdditionalDista,
		float SideOfRoad,
		FVector& LocationToSteerTo
	);
};
