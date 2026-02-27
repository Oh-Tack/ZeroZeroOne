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
	CurrentSideOfRoad = 1.0f; // 현재 보간된 차선 값
	LaneChangeSpeed = 3.0f; // 기본 차선 변경 속도
	CurrentLaneChangeSpeed = LaneChangeSpeed;
}

void AAIC_Vehicle::BeginPlay()
{
	Super::BeginPlay();

	ControllerVehicle = GetPawn();

	Road = Cast<ACPP_Road>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ACPP_Road::StaticClass())
	);

	// 경기 시작 시 차량 캐싱
	TArray<AActor*> FoundVehicles;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUIn_isVehicle::StaticClass(), FoundVehicles);
    
	// 2. 리스트에서 '나 자신'은 제외 (중요!)
	FoundVehicles.Remove(ControllerVehicle);
    
	AllVehicles = FoundVehicles;
}

void AAIC_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!ControllerVehicle) return;

	// 1. 긴급 회피 및 추월 판단
	HandleEmergencyEvade(DeltaTime);
	CheckForOvertakes();

	// 2. 차선 보간 연산
	CurrentSideOfRoad = FMath::FInterpTo(CurrentSideOfRoad, TargetSideOfRoad, DeltaTime, CurrentLaneChangeSpeed);

	// 3. 조향 적용
	IUIn_isVehicle::Execute_SetSteering(ControllerVehicle, CalculateSteering());

	// 4. 속도 및 제어 적용
	float TargetSpeed = HandleTargetSpeed();
	float Throttle, Brake;
	CalculateThrottleBrake(TargetSpeed, Throttle, Brake);

	IUIn_isVehicle::Execute_SetThrottle(ControllerVehicle, Throttle);
	IUIn_isVehicle::Execute_SetBrake(ControllerVehicle, Brake);
}

// -------------------------
// Steering & Speed
// -------------------------
float AAIC_Vehicle::CalculateSteering()
{
	auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
	if (!Road || !ControllerVehicle || !AIVehicle) return 0;

	float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);

	// [수정] 급커브 상황(저속)에서 시야 거리를 더 짧게 가져가서 정확도 향상
	float ForwardVision = UKismetMathLibrary::MapRangeClamped(
		Curr, AIVehicle->Min_Speed, AIVehicle->Max_Speed, 1200.0f, 4500.f
	);

	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);

	FVector LeftLanePos, RightLanePos;
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 0.0f, LeftLanePos);
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 1.0f, RightLanePos);

	SteerTarget = FMath::Lerp(LeftLanePos, RightLanePos, CurrentSideOfRoad);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(start, SteerTarget);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());
	
	return UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -15.0, 15.0, -1, 1);
}

float AAIC_Vehicle::CalculateTopSpeed()
{
	auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
	if (!Road || !ControllerVehicle || !AIVehicle) return 0.0f;

	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);

	// 현재 보간된 차선 위치 기준의 회전각 계산
	Road->GetClosestLocationToPath_Implementation(start, 1500, CurrentSideOfRoad, SteerTarget);

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::FindLookAtRotation(start, SteerTarget),
		ControllerVehicle->GetActorRotation()
	);

	return UKismetMathLibrary::MapRangeClamped(
		abs(DeltaRot.Yaw), 0.0, AIVehicle->Angle, AIVehicle->Max_Speed, AIVehicle->Min_Speed
	);
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
		EscapeBox->GetOverlappingActors(EscapeActors);
		EscapeActors.Remove(ControllerVehicle);

		if (EscapeActors.Num() == 0) // 피할 곳이 비어있을 때
		{
			TargetSideOfRoad = EscapeSide;
			bIsOvertaking = false;
			bEmergencyBrake = false; // 탈출 가능하므로 해제
			CurrentLaneChangeSpeed = LaneChangeSpeed * 3.0f;
		}
		else // 양쪽 다 막혔을 때
		{
			bEmergencyBrake = true;
		}
	}
	else
	{
		bEmergencyBrake = false; // 장애물 없으면 해제
		CurrentLaneChangeSpeed = FMath::FInterpTo(CurrentLaneChangeSpeed, LaneChangeSpeed, DeltaTime, 2.0f);
	}
}

void AAIC_Vehicle::CheckForOvertakes()
{
    if (!ControllerVehicle) return;

    // 1️⃣ 충돌 박스 가져오기 (인터페이스를 통해 각 차량의 BoxComponent 참조)
    IUIn_isVehicle::Execute_GetCollisionBoxes(ControllerVehicle, FrontBox, LeftBox, RightBox);
    if (!FrontBox || !LeftBox || !RightBox) return;

    TArray<AActor*> FrontActors, LeftActors, RightActors;
    FrontBox->GetOverlappingActors(FrontActors);
    LeftBox->GetOverlappingActors(LeftActors);
    RightBox->GetOverlappingActors(RightActors);
    
    // 자기 자신 제거 (필수!)
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
    bool bPlayerDetected = false;

    // 2️⃣ FrontBox 안에 들어온 차량 중 앞차 선택
    for (AActor* Actor : FrontActors)
    {
       if (!Actor) continue;

       // Spline 기준 거리 차이
       const float OtherDistance = Manager->GetDistanceOfVehicle(Actor);
       const float Gap = OtherDistance - MyDistance;

       // 뒤 차량 제외 / 너무 먼 차량 제외
       if (Gap <= 0.f || Gap > 2000.f)
          continue;

       // 월드 전방 필터 (정면 60도 이내)
       const FVector Dir = (Actor->GetActorLocation() - MyLocation).GetSafeNormal();
       const float Dot = FVector::DotProduct(MyForward, Dir);

       if (Dot < 0.5f)
          continue;

       // --- 속도 비교 ---
       const float OtherSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(Actor);
       const float SpeedAdvantage = MySpeed - OtherSpeed;

       // --- 점수 계산 (추월 대상을 결정하는 우선순위) ---
       float Score = 0.f;

       // 가까울수록 높은 점수
       Score += (1500.f - Gap) * 0.6f;

       // 내가 더 빠르면(추월 가능성 높음) 가중치
       Score += SpeedAdvantage * 2.0f;

       // 최고 점수 갱신 (가장 '방해되는' 혹은 '추월하기 좋은' 차량 선택)
       if (Score > BestScore)
       {
          BestScore = Score;
          VehicleInFrontActor = Actor;
       }
    }

    // 로그 타이머 업데이트
    if (bPlayerDetected && (GetWorld()->GetTimeSeconds() - LastLogTime >= 1.0f))
    {
       LastLogTime = GetWorld()->GetTimeSeconds();
    }

    // 3️⃣ 차선 변경 처리 (수집된 좌/우 차량 정보 전달)
    HandleLaneChange(VehicleInFrontActor != nullptr, LeftActors, RightActors);

    // ==========================
    // 🔎 Debug 시각화
    // ==========================

    // 기본 상자 색상: 아무도 없으면 Green, 앞차 있으면 Red
    FColor BoxColor = (VehicleInFrontActor) ? FColor::Red : FColor::Green;
    
    // 만약 그 앞차가 플레이어라면 상자 색상을 보라색(Magenta)으로 표시
    APawn* FrontPawn = Cast<APawn>(VehicleInFrontActor);
    if (FrontPawn && FrontPawn->IsPlayerControlled())
    {
        BoxColor = FColor::Magenta;
    }

    DrawDebugBox(
       GetWorld(),
       FrontBox->GetComponentLocation(),
       FrontBox->GetScaledBoxExtent(),
       FrontBox->GetComponentQuat(),
       BoxColor,
       false,
       0.1f,
       0,
       2.0f
    );

    if (VehicleInFrontActor)
    {
       // 플레이어를 추적 중일 때는 노란색 대신 굵은 보라색 선을 그림
       FColor LineColor = (FrontPawn && FrontPawn->IsPlayerControlled()) ? FColor::Magenta : FColor::Yellow;
       float LineThickness = (FrontPawn && FrontPawn->IsPlayerControlled()) ? 10.0f : 10.0f;

       DrawDebugLine(
          GetWorld(),
          ControllerVehicle->GetActorLocation(),
          VehicleInFrontActor->GetActorLocation(),
          LineColor,
          false,
          0.1f,
          0,
          LineThickness
       );
    }

    // 조향 목표 지점 시각화
    DrawDebugSphere(GetWorld(), SteerTarget, 100.f, 12, FColor::Cyan, false, 0.1f);
}

void AAIC_Vehicle::HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors,
                                    const TArray<AActor*>& RightActors)
{
	if (!ControllerVehicle || !Road || !Road->Spline) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 1. 현재 AI가 우측(1.0)인지 좌측(0.0)인지 파악
	bool bCurrentlyRight = (TargetSideOfRoad > 0.5f);

	// 2. 넘어갈 반대 차선이 비어있는지 확인
	// (우측에 있다면 왼쪽 확인, 좌측에 있다면 오른쪽 확인)
	bool bCanChangeLane = bCurrentlyRight ? (LeftActors.Num() == 0) : (RightActors.Num() == 0);

	// 3. 추월 시도 조건 충족 확인
	bool bCanOvertake = bVehicleInFront && bCanChangeLane && !bIsOvertaking;

	if (bCanOvertake)
	{
		// 목표 차선을 반대편으로 설정
		float IntendedSide = bCurrentlyRight ? 0.0f : 1.0f;

		FVector IntendedPosition;
		Road->GetClosestLocationToPath_Implementation(
			IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle),
			1500.f,
			IntendedSide,
			IntendedPosition
		);

		bool bSafeToChange = true;
		for (AActor* Vehicle : AllVehicles)
		{
			if (!Vehicle || Vehicle == ControllerVehicle) continue;

			// Dist 대신 DistSquared 사용 (600의 제곱 = 360000)
			float DistSq = FVector::DistSquared(IntendedPosition, Vehicle->GetActorLocation());
			if (DistSq < 360000.f)
			{
				bSafeToChange = false;
				break;
			}
		}

		if (bSafeToChange)
		{
			TargetSideOfRoad = IntendedSide; // 차선 변경
			bIsOvertaking = true;
			OvertakeStartTime = CurrentTime;
		}
	}

	// 추월 상태 해제 로직
	if (bIsOvertaking)
	{
		float DistanceToFront = 10000.f;
		if (VehicleInFrontActor)
		{
			DistanceToFront = FVector::Dist(ControllerVehicle->GetActorLocation(),
			                                VehicleInFrontActor->GetActorLocation());
		}

		bool bFrontClear = !VehicleInFrontActor;
		bool bTimeout = (CurrentTime - OvertakeStartTime) > OvertakeDuration;
		bool bSufficientGap = DistanceToFront > 2000.f;

		if (bFrontClear || bTimeout || bSufficientGap)
		{
			bIsOvertaking = false;
		}
	}
}

// -------------------------
// Target Speed
// -------------------------
float AAIC_Vehicle::HandleTargetSpeed()
{
	if (!ControllerVehicle || !Road || !Road->Spline)
		return 0.f;

	const float BaseTopSpeed = CalculateTopSpeed();
	float TargetTopSpeed = BaseTopSpeed;

	ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
	if (!Manager)
		return BaseTopSpeed;

	const int MyRank = Manager->GetRankOfVehicle(ControllerVehicle);
	const int TotalCars = Manager->GetTotalRacers();
	const float MyDistance = Manager->GetDistanceOfVehicle(ControllerVehicle);
	const float LeadDistance = Manager->GetLeadDistance();
	const float GapToLeader = LeadDistance - MyDistance;

	// ==============================
	// Catch-up Speed Boost
	// ==============================
	if (MyRank > 1 && GapToLeader > 2000.f && TotalCars > 1)
	{
		const float GapRatio = FMath::Clamp((GapToLeader - 2000.f) / 6000.f, 0.f, 1.f);
		const float RankRatio = FMath::Clamp((float)(MyRank - 1) / (float)(TotalCars - 1), 0.f, 1.f);

		float SpeedBoost = (0.18f * GapRatio) + (0.22f * RankRatio);
		SpeedBoost = FMath::Clamp(SpeedBoost, 0.1f, 0.5f);

		TargetTopSpeed = BaseTopSpeed * (1.f + SpeedBoost);
	}

	// ==============================
	// Front Vehicle Logic
	// ==============================
	if (VehicleInFrontActor)
	{
		const float FrontDist = Manager->GetDistanceOfVehicle(VehicleInFrontActor);
		const float DistanceGap = FrontDist - MyDistance;
		const float FrontSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(VehicleInFrontActor);

		// 추월 중이면 절대 감속하지 않음
		if (bIsOvertaking)
		{
			TargetTopSpeed = FMath::Max(TargetTopSpeed, BaseTopSpeed * 1.2f);
		}
		else
		{
			if (DistanceGap > 0.f && DistanceGap < 1500.f)
			{
				TargetTopSpeed = FMath::Min(TargetTopSpeed, FrontSpeed - 20.f);
			}
		}
	}

	// ==============================
	// Safety Clamp
	// ==============================
	if (ACPP_AI_McLaren* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle))
	{
		TargetTopSpeed = FMath::Clamp(
			TargetTopSpeed,
			AIVehicle->Min_Speed,
			AIVehicle->Max_Speed * 1.35f
		);
	}

	return TargetTopSpeed;
}

// -------------------------
// Throttle & Brake
// -------------------------
void AAIC_Vehicle::CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake)
{
	if (!ControllerVehicle) return;

	// 양쪽 다 막힌 상황에서의 최우선 처리
	if (bEmergencyBrake)
	{
		Throttle = 0.0f;
		Brake = 1.0f;
		return;
	}

	float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
	float SpeedDiff = Curr - TopSpeed;

	if (SpeedDiff < 0.0f)
	{
		Throttle = FMath::Clamp(0.5f + (-SpeedDiff) / 40.0f, 0.3f, 0.9f);
		Brake = 0.0f;
	}
	else
	{
		Throttle = 0.0f;
		Brake = FMath::Clamp(SpeedDiff / 45.0f, 0.0f, 1.0f);

		// 미세한 속도 초과는 엔진 브레이크만 사용
		if (SpeedDiff < 5.0f) Brake *= 0.3f;
	}
}

// -------------------------
// Race Info
// -------------------------
int AAIC_Vehicle::GetRaceRank()
{
	ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
	return Manager ? Manager->GetRankOfVehicle(ControllerVehicle) : 1;
}

int AAIC_Vehicle::GetTotalRacers()
{
	return AllVehicles.Num();
}

void AAIC_Vehicle::LogRaceRankings() // 호출 안함
{
	// 로그는 Manager에서 중앙 집중식으로 한 번만 출력하도록 위임하는 것이 좋습니다.
	ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
	if (Manager) Manager->PrintCurrentRankings();
}
