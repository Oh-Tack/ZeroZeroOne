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

	// --- 추가된 초기화 ---
	TargetSideOfRoad = 1.0f;
	CurrentSideOfRoad = 1.0f; // 현재 보간된 차선 값
	LaneChangeSpeed = 2.0f; // 기본 차선 변경 속도
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
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_AI_McLaren::StaticClass(), AllVehicles);
}

void AAIC_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!ControllerVehicle) return;

	// 1. 긴급 회피 및 추월 판단
	HandleEmergencyEvade(DeltaTime);
	CheckForOvertakes();

	// 2. 차선 보간 연산 (부드러운 조향의 핵심)
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
	float ForwardVision = UKismetMathLibrary::MapRangeClamped(
		Curr, AIVehicle->Min_Speed, AIVehicle->Max_Speed, 2000.0f, 6000.f
	);

	FVector start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);

	// [개선] 보간된 CurrentSideOfRoad를 사용하여 목표 지점을 선형 보간으로 찾음
	FVector LeftLanePos, RightLanePos;
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 0.0f, LeftLanePos);
	Road->GetClosestLocationToPath_Implementation(start, ForwardVision, 1.0f, RightLanePos);

	SteerTarget = FMath::Lerp(LeftLanePos, RightLanePos, CurrentSideOfRoad);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(start, SteerTarget);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());

	return UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -10.0, 10.0, -1, 1);
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

	// 현재 가고 있는 방향(TargetSideOfRoad)에 따른 콜리전 박스 선택
	UPrimitiveComponent* TargetBox = (TargetSideOfRoad < 0.5f) ? LeftBox : RightBox;
	if (!TargetBox) return;

	TArray<AActor*> OverlappingActors;
	TargetBox->GetOverlappingActors(OverlappingActors);

	// 장애물이 감지되면 긴급 회피
	if (OverlappingActors.Num() > 0)
	{
		float EscapeSide = (TargetSideOfRoad < 0.5f) ? 1.0f : 0.0f;

		if (TargetSideOfRoad != EscapeSide)
		{
			TargetSideOfRoad = EscapeSide;
			bIsOvertaking = false; // 추월 중단

			// 회피 시에는 핸들을 훨씬 빠르게 꺾음
			CurrentLaneChangeSpeed = LaneChangeSpeed * 3.0f;
		}
	}
	else
	{
		// 상황이 종료되면 원래 보간 속도로 점진적 복구
		CurrentLaneChangeSpeed = FMath::FInterpTo(CurrentLaneChangeSpeed, LaneChangeSpeed, DeltaTime, 2.0f);
	}
}

void AAIC_Vehicle::CheckForOvertakes()
{
	if (!ControllerVehicle) return;

	// 인터페이스를 통해 Pawn으로부터 BoxComponent 주소 획득
	IUIn_isVehicle::Execute_GetCollisionBoxes(ControllerVehicle, FrontBox, LeftBox, RightBox);
	if (!FrontBox || !LeftBox || !RightBox) return;

	TArray<AActor*> FrontActors, LeftActors, RightActors;
	FrontBox->GetOverlappingActors(FrontActors);
	LeftBox->GetOverlappingActors(LeftActors);
	RightBox->GetOverlappingActors(RightActors);

	// 본인 차량은 인식에서 제외 (중요!)
	FrontActors.Remove(ControllerVehicle);
	LeftActors.Remove(ControllerVehicle);
	RightActors.Remove(ControllerVehicle);

	VehicleInFrontActor = (FrontActors.Num() > 0) ? FrontActors[0] : nullptr;
	HandleLaneChange(VehicleInFrontActor != nullptr, LeftActors, RightActors);

	if (FrontBox)
	{
		FColor BoxColor = (VehicleInFrontActor) ? FColor::Red : FColor::Green;

		// 1. 센서 박스 시각화
		DrawDebugBox(
			GetWorld(),
			FrontBox->GetComponentLocation(),
			FrontBox->GetScaledBoxExtent(),
			FrontBox->GetComponentQuat(),
			BoxColor,
			false, // Persistent (지속 여부)
			0.1f, // LifeTime
			0, // DepthPriority
			2.0f // Thickness
		);

		// 2. 만약 앞차를 발견했다면 선으로 연결
		if (VehicleInFrontActor)
		{
			DrawDebugLine(
				GetWorld(),
				ControllerVehicle->GetActorLocation(),
				VehicleInFrontActor->GetActorLocation(),
				FColor::Yellow,
				false, 0.1f, 0, 1.5f
			);
		}
	}
	// 3. 목표 지점(SteerTarget) 시각화
	DrawDebugSphere(GetWorld(), SteerTarget, 100.f, 12, FColor::Cyan, false, 0.1f);
}

void AAIC_Vehicle::HandleLaneChange(bool bVehicleInFront, const TArray<AActor*>& LeftActors,
                                    const TArray<AActor*>& RightActors)
{
	if (!ControllerVehicle || !Road || !Road->Spline) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	// 현재 목표 차선에 따라 반대 차선 상태 확인
	bool bOppositeLaneClear = (TargetSideOfRoad > 0.5f) ? (LeftActors.Num() == 0) : (RightActors.Num() == 0);
	bool bSideVehiclePresent = (TargetSideOfRoad > 0.5f) ? (RightActors.Num() > 0) : (LeftActors.Num() > 0);

	bool bCanOvertake = bVehicleInFront && bOppositeLaneClear && !bSideVehiclePresent && !bIsOvertaking;

	if (bCanOvertake)
	{
		float IntendedSide = (TargetSideOfRoad > 0.5f) ? 0.0f : 1.0f;
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
			float Dist = FVector::Dist(IntendedPosition, Vehicle->GetActorLocation());
			if (Dist < 600.f)
			{
				bSafeToChange = false;
				break;
			}
		}

		if (bSafeToChange)
		{
			TargetSideOfRoad = IntendedSide; // 즉시 변경이 아닌 '목표'만 변경
			bIsOvertaking = true;
			OvertakeStartTime = CurrentTime;
		}
	}

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
		bool bSufficientGap = DistanceToFront > 1500.f;

		if (bFrontClear || bTimeout || bSufficientGap)
		{
			bIsOvertaking = false;
			// TargetSideOfRoad = 1.0f; // 추월 종료 시 우측 차선 복귀 지시
		}
	}
}

// -------------------------
// Target Speed (RaceManager 데이터 활용 버전)
// -------------------------
float AAIC_Vehicle::HandleTargetSpeed()
{
	if (!ControllerVehicle || !Road || !Road->Spline) return 0.f;

	const float BaseTopSpeed = CalculateTopSpeed();
	float TargetTopSpeed = BaseTopSpeed;

	// 직접 순위를 계산하지 않고 Manager에게 요청 (성능 최적화)
	ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
	if (!Manager) return BaseTopSpeed;

	const int MyRank = Manager->GetRankOfVehicle(ControllerVehicle);
	const int TotalCars = Manager->GetTotalRacers();
	const float MyDistance = Manager->GetDistanceOfVehicle(ControllerVehicle);
	const float LeadDistance = Manager->GetLeadDistance();
	const float GapToLeader = LeadDistance - MyDistance;

	// Catch-up Speed Boost
	if (MyRank > 1 && GapToLeader > 2000.f)
	{
		const float GapRatio = FMath::Clamp((GapToLeader - 2000.f) / 6000.f, 0.f, 1.f);
		const float RankRatio = FMath::Clamp((float)(MyRank - 1) / (float)(TotalCars - 1), 0.f, 1.f);
		float SpeedBoost = (0.18f * GapRatio) + (0.22f * RankRatio);
		SpeedBoost = FMath::Clamp(SpeedBoost, 0.1f, 0.5f);

		TargetTopSpeed = BaseTopSpeed * (1.f + SpeedBoost);
	}

	// Front Vehicle Constraint
	if (VehicleInFrontActor)
	{
		float FrontDist = Manager->GetDistanceOfVehicle(VehicleInFrontActor);
		const float DistanceGap = FrontDist - MyDistance;
		const float FrontSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(VehicleInFrontActor);

		if (bIsOvertaking)
		{
			TargetTopSpeed = FMath::Max(TargetTopSpeed, FrontSpeed + 100.f);
		}
		else if (DistanceGap < 1000.f)
		{
			TargetTopSpeed = FMath::Min(TargetTopSpeed, FrontSpeed - 20.f);
		}
	}

	// Safety Clamp
	auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
	if (AIVehicle)
	{
		TargetTopSpeed = FMath::Clamp(TargetTopSpeed, AIVehicle->Min_Speed, AIVehicle->Max_Speed * 1.35f);
	}

	return TargetTopSpeed;
}

// -------------------------
// Throttle & Brake
// -------------------------
void AAIC_Vehicle::CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake)
{
	if (!ControllerVehicle) return;

	float Curr = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
	float SpeedDiff = Curr - TopSpeed;

	if (SpeedDiff < 0.0f)
	{
		Throttle = FMath::Clamp(0.5f + (-SpeedDiff) / 50.0f, 0.3f, 0.9f);
		Brake = 0.0f;
	}
	else
	{
		Throttle = FMath::Clamp(0.2f - SpeedDiff / 50.0f, 0.0f, 0.2f);
		Brake = FMath::Clamp(SpeedDiff / 30.0f, 0.0f, 1.0f); // 감속 시 반응성 강화
	}
}

// -------------------------
// Race Info (Manager 참조 방식으로 변경)
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

void AAIC_Vehicle::LogRaceRankings()
{
	// 로그는 Manager에서 중앙 집중식으로 한 번만 출력하도록 위임하는 것이 좋습니다.
	ACPP_AIRaceManager* Manager = ACPP_AIRaceManager::GetInstance(GetWorld());
	if (Manager) Manager->PrintCurrentRankings();
}
