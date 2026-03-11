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

	// 🔹 1️⃣ AIRaceManager에서 모든 AI 가져오기
	TArray<AAIC_Vehicle*> AllAIVehicles;
	if (ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld()))
	{
		for (auto& Info : Manager->RacerTable)
		{
			AAIC_Vehicle* AI = Cast<AAIC_Vehicle>(Info.Vehicle);
			if (AI && AI != this)
				AllAIVehicles.Add(AI);
		}
	}

	// 🔹 2️⃣ 장애물 회피
	CheckForStaticObstacles(DeltaTime, AllAIVehicles);

	// 🔹 3️⃣ 주변 인지 / 비상 회피
	HandleEmergencyEvade(DeltaTime);
	CheckForOvertakes();

	// 🔹 4️⃣ 차선 보간
	CurrentSideOfRoad = FMath::FInterpTo(
	   CurrentSideOfRoad,
	   TargetSideOfRoad,
	   DeltaTime,
	   CurrentLaneChangeSpeed
	);

	// 🔹 5️⃣ 최종 조향 및 속도 결정
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
	float ForwardVision = UKismetMathLibrary::MapRangeClamped(Curr, CachedAIVehicle->Min_Speed, CachedAIVehicle->Max_Speed, 1500.f, 7500.f);
	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);

	FVector LeftLanePos, RightLanePos;
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 0.0f, LeftLanePos);
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 1.0f, RightLanePos);

	// 1️⃣ 기본 타겟: 현재 차선 위치
	SteerTarget = FMath::Lerp(LeftLanePos, RightLanePos, CurrentSideOfRoad);

	// 2️⃣ 회피 오프셋 적용: 회피 반경을 1500~1800으로 확장하여 더 확실하게 피하게 함
	FVector RightVector = ControllerVehicle->GetActorRightVector();
	float AvoidanceWidth = 2000.0f; 
	SteerTarget += RightVector * (AvoidanceForceValue * AvoidanceWidth); 

	// 3️⃣ 최종 조향각 계산
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(start, SteerTarget);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());

	// 디버그용 (최종 타겟 구체)
	DrawDebugSphere(GetWorld(), SteerTarget, 100.f, 12, FColor::Yellow, false, 0.1f);

	return UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -25.0f, 25.0f, -1.f, 1.f);
}

float AAIC_Vehicle::CalculateTopSpeed()
{
	if (!Road || !ControllerVehicle || !CachedAIVehicle) return 0.0f;

	float BaseTopSpeed = CachedAIVehicle->Max_Speed;

	// --- 기존 커브 감속 로직 ---
	FVector Start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
	float CurrSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);

	float LookAhead = UKismetMathLibrary::MapRangeClamped(
		CurrSpeed,
		CachedAIVehicle->Min_Speed,
		CachedAIVehicle->Max_Speed,
		2000.f,
		15000.f
	);

	FVector SpeedLookAtPos;
	Road->GetClosestLocationToPath_Implementation(Start, LookAhead, CurrentSideOfRoad, SpeedLookAtPos);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(Start, SpeedLookAtPos);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());

	float Angle = FMath::Abs(DeltaRot.Yaw);
	float AngleAlpha = FMath::Clamp(Angle / CachedAIVehicle->Angle, 0.f, 1.f);
	float SpeedAlpha = FMath::Pow(AngleAlpha, 1.5f);

	float CurvedTopSpeed = FMath::Lerp(BaseTopSpeed, CachedAIVehicle->Min_Speed, SpeedAlpha);

	// 급커브 안전 감속
	if (Angle > 25.f)
		CurvedTopSpeed *= 0.6f;

	return CurvedTopSpeed;
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
    float BaseTopSpeed = CachedAIVehicle->Max_Speed;

    // -------------------------
    // 1️⃣ 기본 커브 감속
    // -------------------------
    float CurvedTopSpeed = TargetTopSpeed;

    // -------------------------
    // 2️⃣ 캐치업(Catch-up) 적용
    // -------------------------
    if (ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld()))
    {
        int MyRank = Manager->GetRankOfVehicle(ControllerVehicle);
        int TotalCars = FMath::Max(GetTotalRacers(), 2);

        if (MyRank > 1 && TotalCars > 1)
        {
            float GapToLeader = Manager->LeadDistance - Manager->GetDistanceOfVehicle(ControllerVehicle);

            const float GapRatio = FMath::Clamp(GapToLeader / 8000.f, 0.f, 1.f); // 최대 8000cm 보정
            const float RankRatio = FMath::Clamp((float)(MyRank - 1) / (float)(TotalCars - 1), 0.f, 1.f);

            float SpeedBoost = (0.18f * GapRatio) + (0.22f * RankRatio);
            SpeedBoost = FMath::Clamp(SpeedBoost, 0.1f, 0.5f);

            CurvedTopSpeed = FMath::Min(CurvedTopSpeed, BaseTopSpeed * (1.f + SpeedBoost));
        }
    }

    // -------------------------
    // 3️⃣ 직선 구간 속도 강화
    // -------------------------
    float ForwardVision = UKismetMathLibrary::MapRangeClamped(CurrentSpeed, CachedAIVehicle->Min_Speed, CachedAIVehicle->Max_Speed, 1500.f, 7500.f);
    if (ForwardVision > 5000.f) // 충분히 직선 구간이면
    {
        CurvedTopSpeed *= 1.1f; // 10% 추가 가속
        CurvedTopSpeed = FMath::Min(CurvedTopSpeed, BaseTopSpeed * 1.5f); // 상한 제한
    }

    // -------------------------
    // 4️⃣ 스로틀/브레이크 결정
    // -------------------------
    float Throttle = 1.f;
    float Brake = 0.f;

    float SpeedDiff = CurrentSpeed - CurvedTopSpeed;
    if (SpeedDiff > 0.f)
    {
        Brake = FMath::Clamp(SpeedDiff / 4.f, 0.f, 1.f);
        Throttle = 0.f;
    }

    if (FMath::Abs(Steering) > 0.6f && CurrentSpeed > CachedAIVehicle->Min_Speed)
        Throttle *= 0.5f;

    // -------------------------
    // 5️⃣ Emergency Brake 적용
    // -------------------------
    if (bEmergencyBrake)
    {
        Throttle = 0.f;
        Brake = FMath::Max(Brake, 1.f);
    }

    // -------------------------
    // 6️⃣ 최종 적용
    // -------------------------
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

void AAIC_Vehicle::CheckForStaticObstacles(float DeltaTime, const TArray<AAIC_Vehicle*>& AllAIVehicles)
{
    if (!ControllerVehicle || !CachedAIVehicle) return;

    // 🔹 속도 기반 감지 거리
    float CurrentSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
    float TraceLength = FMath::Clamp(CurrentSpeed * 0.8f, 2000.f, 8000.f);

    FVector CarFront = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
    FVector Start = CarFront + FVector(0.f, 0.f, 150.f);

    FVector Forward = ControllerVehicle->GetActorForwardVector();
    FVector Right = ControllerVehicle->GetActorRightVector();

    // =====================================================
    // ✅ 커브 대응 Ray 방향 (도로 진행 방향 사용)
    // =====================================================
    FVector RoadForward = Forward;

    if (Road && Road->Spline)
    {
        FVector AheadPos;
        Road->GetClosestLocationToPath_Implementation(
            CarFront,
            1200.f,                 // 약간 앞 위치
            CurrentSideOfRoad,
            AheadPos
        );

        RoadForward = (AheadPos - CarFront).GetSafeNormal();
    }

    // 차량 방향 + 도로 방향 혼합
    float CurveAssist = 0.7f; // (0.6~0.8 추천)
    FVector BlendedForward =
        FMath::Lerp(Forward, RoadForward, CurveAssist).GetSafeNormal();
    // =====================================================

    float SphereRadius = 50.f;
    const int32 TraceCount = 5;
    const float SpreadAngle = 40.f;

    bool bRayHit = false;
    float MinHitDistance = TraceLength;
    AActor* CurrentHitActor = nullptr;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(ControllerVehicle);

    for (AAIC_Vehicle* OtherAI : AllAIVehicles)
        if (OtherAI && OtherAI->ControllerVehicle)
            Params.AddIgnoredActor(OtherAI->ControllerVehicle);

    float AccumulatedAvoidance = 0.f;
    int32 HitCount = 0;

    // =====================================================
    // 🔹 커브 방향 기반 Sphere Trace
    // =====================================================
    for (int32 i = 0; i < TraceCount; i++)
    {
        float Angle = -SpreadAngle / 2.0f +
            (SpreadAngle / (TraceCount - 1)) * i;

        // ⭐ Forward 대신 BlendedForward 사용
        FVector TraceDir =
            BlendedForward.RotateAngleAxis(Angle, FVector::UpVector);

        FVector End = Start + TraceDir * TraceLength;

        FHitResult Hit;
        bool bHit = GetWorld()->SweepSingleByChannel(
            Hit,
            Start,
            End,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeSphere(SphereRadius),
            Params
        );

        if (bHit && Hit.GetActor()->ActorHasTag(FName("Powerplay")))
        {
            bRayHit = true;
            MinHitDistance = FMath::Min(MinHitDistance, Hit.Distance);
            CurrentHitActor = Hit.GetActor();

            float SideDot =
                FVector::DotProduct(Hit.ImpactPoint - Start, Right);

            // 오른쪽 맞으면 왼쪽 회피
            // 왼쪽 맞으면 오른쪽 회피
            AccumulatedAvoidance += (SideDot >= 0.f) ? -1.f : 1.f;
            HitCount++;
        }

#if WITH_EDITOR
        DrawDebugLine(GetWorld(), Start, End,
            bHit ? FColor::Red : FColor::Green,
            false, 0.05f, 0, 1.5f);
#endif
    }

    // =====================================================
    // 🔹 평균 회피 방향
    // =====================================================
    float TargetAvoidance =
        (HitCount > 0) ? (AccumulatedAvoidance / HitCount) : 0.f;

    // =====================================================
    // ✅ 차선 기반 중앙 복귀 Bias (좌우 모두)
    // =====================================================
    if (HitCount > 0)
    {
        const float LaneBiasStrength = 0.35f;

        if (CurrentSideOfRoad > 0.5f)
        {
            // 오른차선 → 중앙(왼쪽)
            TargetAvoidance -= LaneBiasStrength;
        }
        else
        {
            // 왼쪽차선 → 중앙(오른쪽)
            TargetAvoidance += LaneBiasStrength;
        }
    }

    TargetAvoidance = FMath::Clamp(TargetAvoidance, -1.0f, 1.0f);

    // =====================================================
    // 🔹 회피값 스무딩
    // =====================================================
    float InterpSpeed = bRayHit ? 10.f : 5.f;

    AvoidanceForceValue = FMath::FInterpTo(
        AvoidanceForceValue,
        TargetAvoidance,
        DeltaTime,
        InterpSpeed
    );

    // =====================================================
    // 🔹 쿨다운 유지
    // =====================================================
    if (bRayHit)
    {
        LastDetectedObstacle = CurrentHitActor;
        AvoidanceCooldown = 0.5f;
    }
    else if (AvoidanceCooldown > 0.f && IsValid(LastDetectedObstacle))
    {
        AvoidanceCooldown -= DeltaTime;
        bRayHit = true;
    }

    float SafeDistance =
        FMath::Max(1500.f, CurrentSpeed * 0.6f);

    bEmergencyBrake =
        bRayHit && (MinHitDistance < SafeDistance);

    if (bRayHit)
    {
        DrawDebugSphere(
            GetWorld(),
            Start + BlendedForward * MinHitDistance,
            SphereRadius,
            8,
            FColor::Red,
            false,
            0.2f
        );
    }
}