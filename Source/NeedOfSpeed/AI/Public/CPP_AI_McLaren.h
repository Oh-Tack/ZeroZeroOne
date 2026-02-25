#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "UIn_isVehicle.h"
#include "Components/SplineComponent.h"
#include "CPP_AI_McLaren.generated.h"

class ACPP_Road;
class USplineComponent;
class USceneComponent;
class UBoxComponent;

UCLASS()
class NEEDOFSPEED_API ACPP_AI_McLaren : public AWheeledVehiclePawn, public IUIn_isVehicle
{
	GENERATED_BODY()

public:
	ACPP_AI_McLaren();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetThrottle_Implementation(float Throttle) override;
	virtual void SetSteering_Implementation(float Steering) override;
	virtual void SetBrake_Implementation(float Brake) override;
	virtual void GetCollisionBoxes_Implementation(UBoxComponent*& FrontBox, UBoxComponent*& LeftBox, UBoxComponent*& RightBox) override;
	virtual FVector GetFrontOfCar_Implementation() override;
	virtual float GetCurrentSpeed_Implementation() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	ACPP_Road* Road;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	USplineComponent* RoadSpline;

	// UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	// USceneComponent* Sensor_L;
	//
	// UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	// USceneComponent* Sensor_R;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	USceneComponent* Sensor_F;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	UBoxComponent* FrontViewBox;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	UBoxComponent* LeftSideBox;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	UBoxComponent* RightSideBox;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
	float Max_Speed = 150.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
	float Min_Speed = 30.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
	float Angle = 15.0f;
};
