#include "NeedOfSpeed/Public/AI/AIC_Vehicle.h"
#include "NeedOfSpeed/Public/AI/CPP_Road.h"
#include "NeedOfSpeed/Public/AI/UIn_isVehicle.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NeedOfSpeed/Public/AI/CPP_AI_McLaren.h"
#include "NeedOfSpeed/Public/AI/CPP_AIRaceManager.h"
#include "DrawDebugHelpers.h"

AAIC_Vehicle::AAIC_Vehicle()
{
    bIsOvertaking = false;
    OvertakeDuration = 4.0f;
    LastLogTime = 0.f;

    TargetSideOfRoad = 1.0f;
    CurrentSideOfRoad = 1.0f;
    LaneChangeSpeed = 3.0f;
    CurrentLaneChangeSpeed = LaneChangeSpeed;

    bEmergencyBrake = false;
}

void AAIC_Vehicle::BeginPlay()
{
    Super::BeginPlay();

    ControllerVehicle = GetPawn();
    // 🔹 매 프레임 Cast하는 비용을 줄이기 위해 미리 캐싱
    CachedAIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);

    Road = Cast<ACPP_Road>(
       UGameplayStatics::GetActorOfClass(GetWorld(), ACPP_Road::StaticClass())
    );

    // 경기 시작 시 차량 캐싱
    TArray<AActor*> FoundVehicles;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUIn_isVehicle::StaticClass(), FoundVehicles);
    
    FoundVehicles.Remove(ControllerVehicle);
    AllVehicles = FoundVehicles;
}

void AAIC_Vehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ControllerVehicle || !CachedAIVehicle) return;

    // 2️⃣ 주변 인지 로직 (우선순위: 장애물 > 추월)
    // CheckForObstacles(DeltaTime);
    HandleEmergencyEvade(DeltaTime);
    CheckForOvertakes();

    // 3️⃣ 차선 보간
    CurrentSideOfRoad = FMath::FInterpTo(
       CurrentSideOfRoad,
       TargetSideOfRoad,
       DeltaTime,
       CurrentLaneChangeSpeed
    );

    // 4️⃣ 최종 조향 및 속도 결정
    float Steering = CalculateSteering();
    float TargetTopSpeed = CalculateTopSpeed();

    IUIn_isVehicle::Execute_SetSteering(ControllerVehicle, Steering);
    HandleTargetSpeed(TargetTopSpeed, DeltaTime, Steering);
}

// -------------------------
// Steering & Speed
// -------------------------
float AAIC_Vehicle::CalculateSteering()
{
    if (!Road || !ControllerVehicle || !CachedAIVehicle) return 0.0f;

    float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);

    float ForwardVision = UKismetMathLibrary::MapRangeClamped(
       Curr, CachedAIVehicle->Min_Speed, CachedAIVehicle->Max_Speed, 1500.f, 7500.f
    );

    FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);

    FVector LeftLanePos, RightLanePos;
    Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 0.0f, LeftLanePos);
    Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 1.0f, RightLanePos);

    // 멤버 변수 SteerTarget 업데이트
    SteerTarget = FMath::Lerp(LeftLanePos, RightLanePos, CurrentSideOfRoad);

    FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(start, SteerTarget);
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());

    return UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -25.0f, 25.0f, -1.f, 1.f);
}

float AAIC_Vehicle::CalculateTopSpeed()
{
	if (!Road || !ControllerVehicle || !CachedAIVehicle) return 0.0f;

	FVector Start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
	float CurrSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);

	// 1. 시야 거리 계산
	float LookAhead = UKismetMathLibrary::MapRangeClamped(
	   CurrSpeed, CachedAIVehicle->Min_Speed, CachedAIVehicle->Max_Speed, 2000.f, 15000.f
	);

	FVector SpeedLookAtPos;
	Road->GetClosestLocationToPath_Implementation(Start, LookAhead, CurrentSideOfRoad, SpeedLookAtPos);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(Start, SpeedLookAtPos);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());

	float Angle = FMath::Abs(DeltaRot.Yaw);

	// 2. 코너 감속 계산 (비선형)
	float AngleAlpha = FMath::Clamp(Angle / CachedAIVehicle->Angle, 0.0f, 1.0f);
	float SpeedAlpha = FMath::Pow(AngleAlpha, 1.5f);

	float TargetSpeed = FMath::Lerp(CachedAIVehicle->Max_Speed, CachedAIVehicle->Min_Speed, SpeedAlpha);

	// -------------------------
	// RubberBand (Catch-up) 시스템
	// -------------------------
	// ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
	//
	// if (Manager)
	// {
	// 	float MyDist = Manager->GetDistanceOfVehicle(ControllerVehicle);
	// 	float LeadDist = Manager->GetLeadDistance();
	//
	// 	float Gap = LeadDist - MyDist;
	//
	// 	// 4000 이상 벌어지면 보정 시작
	// 	if (Gap > 4000.f)
	// 	{
	// 		int MyRank = Manager->GetRankOfVehicle(ControllerVehicle);
	// 		int Total = FMath::Max(GetTotalRacers(), 2);
	//
	// 		float DistanceAlpha = FMath::Clamp((Gap - 4000.f) / 8000.f, 0.f, 1.f);
	//
	// 		float RankAlpha = 0.f;
	// 		if (Total > 1)
	// 		{
	// 			RankAlpha = (float)(MyRank - 1) / (float)(Total - 1);
	// 		}
	// 		
	// 		float Headroom = CachedAIVehicle->Max_Speed - TargetSpeed;
	// 		float BonusSpeed = Headroom * (DistanceAlpha * RankAlpha * 0.5f); // 최대 50%의 여유폭 활용
	//
	// 		TargetSpeed += BonusSpeed;
	// 	}
	// }

	// -------------------------
	// 급커브 안전 감속
	// -------------------------
	if (Angle > 25.f)
	{
		TargetSpeed *= 0.6f;
	}

	return TargetSpeed;
}

// -------------------------
// Overtake & Evade Logic
// -------------------------
void AAIC_Vehicle::HandleEmergencyEvade(float DeltaTime)
{
    if (!ControllerVehicle) return;

    UPrimitiveComponent* TargetBox = (TargetSideOfRoad < 0.5f) ? LeftBox : RightBox;
    if (!TargetBox) return;

    TArray<AActor*> OverlappingActors;
    TargetBox->GetOverlappingActors(OverlappingActors);
    OverlappingActors.Remove(ControllerVehicle);

    if (OverlappingActors.Num() > 0)
    {
       float EscapeSide = (TargetSideOfRoad < 0.5f) ? 1.0f : 0.0f;
       UPrimitiveComponent* EscapeBox = (EscapeSide < 0.5f) ? LeftBox : RightBox;
       
       TArray<AActor*> EscapeActors;
       if(EscapeBox)
       {
           EscapeBox->GetOverlappingActors(EscapeActors);
           EscapeActors.Remove(ControllerVehicle);
       }

       if (EscapeActors.Num() == 0) 
       {
          TargetSideOfRoad = EscapeSide;
          bIsOvertaking = false;
          bEmergencyBrake = false;
          CurrentLaneChangeSpeed = LaneChangeSpeed * 3.0f;
       }
       else 
       {
          bEmergencyBrake = true;
       }
    }
    else
    {
       bEmergencyBrake = false;
       CurrentLaneChangeSpeed = FMath::FInterpTo(CurrentLaneChangeSpeed, LaneChangeSpeed, DeltaTime, 2.0f);
    }
}

void AAIC_Vehicle::CheckForOvertakes()
{
    if (!ControllerVehicle) return;

    IUIn_isVehicle::Execute_GetCollisionBoxes(ControllerVehicle, FrontBox, LeftBox, RightBox);
    if (!FrontBox || !LeftBox || !RightBox) return;

    TArray<AActor*> FrontActors, LeftActors, RightActors;
    FrontBox->GetOverlappingActors(FrontActors);
    LeftBox->GetOverlappingActors(LeftActors);
    RightBox->GetOverlappingActors(RightActors);
    
    FrontActors.Remove(ControllerVehicle);
    LeftActors.Remove(ControllerVehicle);
    RightActors.Remove(ControllerVehicle);

    VehicleInFrontActor = nullptr;

    ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
    if (!Manager) return;

    const float MyDistance = Manager->GetDistanceOfVehicle(ControllerVehicle);
    const FVector MyLocation = ControllerVehicle->GetActorLocation();
    const FVector MyForward = ControllerVehicle->GetActorForwardVector();
    const float MySpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);

    float BestScore = -FLT_MAX;

    for (AActor* Actor : FrontActors)
    {
       // 🔹 인터페이스 체크 추가 (크래시 방지 핵심)
       if (!Actor || !Actor->GetClass()->ImplementsInterface(UUIn_isVehicle::StaticClass())) continue;

       const float OtherDistance = Manager->GetDistanceOfVehicle(Actor);
       const float Gap = OtherDistance - MyDistance;

       if (Gap <= 0.f || Gap > 2000.f) continue;

       const FVector Dir = (Actor->GetActorLocation() - MyLocation).GetSafeNormal();
       const float Dot = FVector::DotProduct(MyForward, Dir);
       if (Dot < 0.5f) continue;

       const float OtherSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(Actor);
       const float SpeedAdvantage = MySpeed - OtherSpeed;

       float Score = (1500.f - Gap) * 0.6f + (SpeedAdvantage * 2.0f);

       if (Score > BestScore)
       {
          BestScore = Score;
          VehicleInFrontActor = Actor;
       }
    }

    HandleLaneChange(VehicleInFrontActor != nullptr, LeftActors, RightActors);

    // Debug Visualization
    FColor BoxColor = (VehicleInFrontActor) ? FColor::Red : FColor::Green;
    APawn* FrontPawn = Cast<APawn>(VehicleInFrontActor);
    if (FrontPawn && FrontPawn->IsPlayerControlled()) BoxColor = FColor::Magenta;

    DrawDebugBox(GetWorld(), FrontBox->GetComponentLocation(), FrontBox->GetScaledBoxExtent(), FrontBox->GetComponentQuat(), BoxColor, false, 0.1f, 0, 2.0f);
    DrawDebugSphere(GetWorld(), SteerTarget, 100.f, 12, FColor::Cyan, false, 0.1f);
}

void AAIC_Vehicle::HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors, const TArray<AActor*>& RightActors)
{
    if (!ControllerVehicle || !Road || !Road->Spline) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    bool bCurrentlyRight = (TargetSideOfRoad > 0.5f);
    bool bCanChangeLane = bCurrentlyRight ? (LeftActors.Num() == 0) : (RightActors.Num() == 0);
    bool bCanOvertake = bVehicleInFront && bCanChangeLane && !bIsOvertaking;

    if (bCanOvertake)
    {
       float IntendedSide = bCurrentlyRight ? 0.0f : 1.0f;
       FVector IntendedPosition;
       Road->GetClosestLocationToPath_Implementation(IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle), 1500.f, IntendedSide, IntendedPosition);

       bool bSafeToChange = true;
       for (AActor* Vehicle : AllVehicles)
       {
          if (!Vehicle || Vehicle == ControllerVehicle) continue;
          // 🔹 성능을 위해 DistSquared 사용 (600^2 = 360,000)
          if (FVector::DistSquared(IntendedPosition, Vehicle->GetActorLocation()) < 360000.f)
          {
             bSafeToChange = false;
             break;
          }
       }

       if (bSafeToChange)
       {
          TargetSideOfRoad = IntendedSide;
          bIsOvertaking = true;
          OvertakeStartTime = CurrentTime;
       }
    }

    if (bIsOvertaking)
    {
       float DistanceToFront = 10000.f;
       if (VehicleInFrontActor) DistanceToFront = FVector::Dist(ControllerVehicle->GetActorLocation(), VehicleInFrontActor->GetActorLocation());

       if (!VehicleInFrontActor || (CurrentTime - OvertakeStartTime) > OvertakeDuration || DistanceToFront > 2000.f)
       {
          bIsOvertaking = false;
       }
    }
}

void AAIC_Vehicle::HandleTargetSpeed(float TargetTopSpeed, float DeltaTime, float Steering)
{
	if (!ControllerVehicle || !CachedAIVehicle) return;

	float CurrentSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
	float SpeedDiff = CurrentSpeed - TargetTopSpeed;

	float Throttle = 0.0f;
	float Brake = 0.0f;

	if (SpeedDiff > 0)
	{
		// 🔹 브레이크 강도 대폭 강화
		// 차이가 클수록 즉시 최대 제동(1.0)을 수행하도록 분모 값을 낮춤
		Brake = FMath::Clamp(SpeedDiff / 4.0f, 0.0f, 1.0f);
        
		// 아주 급격한 감속이 필요한 경우 스로틀을 완전히 차단
		Throttle = 0.0f;
	}
	else
	{
		Throttle = 1.0f;
		Brake = 0.0f;
	}

	// 코너링 중 타이어 접지력을 위해 스로틀 미세 조절 (Optional)
	if (FMath::Abs(Steering) > 0.6f && CurrentSpeed > CachedAIVehicle->Min_Speed)
	{
		Throttle *= 0.5f; 
	}

	IUIn_isVehicle::Execute_SetThrottle(ControllerVehicle, Throttle);
	IUIn_isVehicle::Execute_SetBrake(ControllerVehicle, Brake);
}

// -------------------------
// Race Info (Helper functions)
// -------------------------
int AAIC_Vehicle::GetRaceRank()
{
    ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
    return Manager ? Manager->GetRankOfVehicle(ControllerVehicle) : 1;
}

int AAIC_Vehicle::GetTotalRacers()
{
    return AllVehicles.Num() + 1; // 자기 자신 포함
}