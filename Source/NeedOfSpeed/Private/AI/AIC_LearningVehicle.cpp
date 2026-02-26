#include "AI/AIC_LearningVehicle.h"
#include "NeedOfSpeed/Public/AI/CPP_Road.h"
#include "NeedOfSpeed/Public/AI/UIn_isVehicle.h"
#include "NeedOfSpeed/Public/AI/CPP_AI_McLaren.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

AAIC_LearningVehicle::AAIC_LearningVehicle()
{
    bIsOvertaking = false;
    OvertakeDuration = 2.0f;
    bOverrideTopSpeed = false;
    LateralOffset = 0.f;
    TargetOffset  = 0.f;
}

void AAIC_LearningVehicle::BeginPlay()
{
    Super::BeginPlay();

    ControllerVehicle = GetPawn();

    // Road 가져오기
    Road = Cast<ACPP_Road>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ACPP_Road::StaticClass())
    );

    // 모든 AI 차량 캐싱
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_AI_McLaren::StaticClass(), AllVehicles);

    // 충돌 박스 초기화
    if (ControllerVehicle)
    {
        TArray<UBoxComponent*> Boxes;
        ControllerVehicle->GetComponents<UBoxComponent>(Boxes);

        for (UBoxComponent* Box : Boxes)
        {
            if (Box->ComponentHasTag("Front")) FrontBox = Box;
            if (Box->ComponentHasTag("Left"))  LeftBox  = Box;
            if (Box->ComponentHasTag("Right")) RightBox = Box;
        }
    }
}

void AAIC_LearningVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ControllerVehicle || !Road) return;

    // ============================
    // 1. 앞차 감지
    // ============================
    VehicleInFrontActor = GetFrontVehicle();
    bool bVehicleInFront = VehicleInFrontActor != nullptr;

    // ============================
    // 2. 추월 판단 및 회피
    // ============================
    UpdateOvertake(DeltaTime);

    // ============================
    // 3. Steering 계산 (Spline + LateralOffset)
    // ============================
    float Steering = CalculateSteering();

    // ============================
    // 4. TargetSpeed 계산 (코너 감속 + 앞차)
    // ============================
    float TargetSpeed = CalculateTopSpeed();

    if (bVehicleInFront && VehicleInFrontActor)
    {
        float FrontDist = FVector::Dist(ControllerVehicle->GetActorLocation(), VehicleInFrontActor->GetActorLocation());
        float FrontSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(VehicleInFrontActor);

        // 안전거리 기반 감속
        float SafeDistance = FMath::Max(FrontSpeed * 0.5f + 100.f, 200.f); // 안전거리 강화
        if (FrontDist < SafeDistance)
        {
            bOverrideTopSpeed = true;
            OverrideTopSpeed = FMath::Min(TargetSpeed, FrontSpeed);
        }
        else
        {
            bOverrideTopSpeed = false;
        }

        // 추월 시 목표 속도
        if (bIsOvertaking)
        {
            TargetSpeed = FMath::Max(TargetSpeed, FrontSpeed + 50.f);
        }
        else if (FrontDist < SafeDistance)
        {
            TargetSpeed = FMath::Min(TargetSpeed, FrontSpeed);
        }
    }

    // ============================
    // 5. Throttle / Brake 계산
    // ============================
    float Throttle = 0.f;
    float Brake    = 0.f;
    CalculateThrottleBrake(TargetSpeed, Throttle, Brake);

    // ============================
    // 6. 실행
    // ============================
    IUIn_isVehicle::Execute_SetSteering(ControllerVehicle, Steering);
    IUIn_isVehicle::Execute_SetThrottle(ControllerVehicle, Throttle);
    IUIn_isVehicle::Execute_SetBrake(ControllerVehicle, Brake);
    
}

// =======================================================
// Steering
// =======================================================
float AAIC_LearningVehicle::CalculateSteering()
{
    auto* AIVehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
    if (!AIVehicle || !Road) return 0.f;

    FVector Start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);
    float CurrSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);

    float LookAhead = UKismetMathLibrary::MapRangeClamped(
        CurrSpeed, AIVehicle->Min_Speed, AIVehicle->Max_Speed, 2000.f, 6000.f
    );

    Road->GetClosestLocationToPath_Implementation(Start, LookAhead, 0, SteerTarget);

    FVector TargetWithOffset = SteerTarget + ControllerVehicle->GetActorRightVector() * LateralOffset;

    FRotator LookRot  = UKismetMathLibrary::FindLookAtRotation(Start, TargetWithOffset);
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, ControllerVehicle->GetActorRotation());

    return UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -30.f, 30.f, -1.f, 1.f);
}

// =======================================================
// TopSpeed (코너 감속 포함)
// =======================================================
float AAIC_LearningVehicle::CalculateTopSpeed()
{
    auto* Vehicle = Cast<ACPP_AI_McLaren>(ControllerVehicle);
    if (!Vehicle || !Road) return 0.f;

    if (bOverrideTopSpeed)
        return OverrideTopSpeed;

    FVector Start = IUIn_isVehicle::Execute_GetFrontOfCar(ControllerVehicle);

    float LookAhead = 1500.f;
    Road->GetClosestLocationToPath_Implementation(Start, LookAhead, 0, SteerTarget);

    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(
        UKismetMathLibrary::FindLookAtRotation(Start, SteerTarget),
        ControllerVehicle->GetActorRotation()
    );

    float YawDiff = FMath::Abs(DeltaRot.Yaw);

    // 코너 감속: 0~45도 기준 Min~Max 속도
    float TargetSpeed = FMath::Lerp(Vehicle->Min_Speed, Vehicle->Max_Speed, 1.f - FMath::Clamp(YawDiff / 45.f, 0.f, 1.f));

    return TargetSpeed;
}

// =======================================================
// Throttle / Brake
// =======================================================
void AAIC_LearningVehicle::CalculateThrottleBrake(float TopSpeed, float& Throttle, float& Brake)
{
    float CurrSpeed = IUIn_isVehicle::Execute_GetCurrentSpeed(ControllerVehicle);
    float SpeedDiff = CurrSpeed - TopSpeed;

    if (SpeedDiff < 0.f)
    {
        Throttle = FMath::Clamp(0.5f + (-SpeedDiff)/50.f, 0.3f, 0.8f);
        Brake    = 0.f;
    }
    else
    {
        Throttle = FMath::Clamp(1.f - SpeedDiff/50.f, 0.f, 0.5f);
        Brake    = FMath::Clamp(SpeedDiff/50.f, 0.2f, 1.f); // 코너 + 앞차 브레이크 강화
    }
}

// =======================================================
// Overtake / LateralOffset
// =======================================================
void AAIC_LearningVehicle::UpdateOvertake(float DeltaTime)
{
    if (!FrontBox) 
    {
        bIsOvertaking = false;
        TargetOffset = 0.f;
        return;
    }

    // 앞차가 없으면 추월 상태 종료
    if (!VehicleInFrontActor)
    {
        bIsOvertaking = false;
        TargetOffset = 0.f;
    }
    else
    {
        TArray<AActor*> FrontOverlaps;
        FrontBox->GetOverlappingActors(FrontOverlaps);
        bool bBlocked = FrontOverlaps.Num() > 0;

        if (bBlocked && !bIsOvertaking)
        {
            TArray<AActor*> LeftOverlaps, RightOverlaps;
            if (LeftBox)  LeftBox->GetOverlappingActors(LeftOverlaps);
            if (RightBox) RightBox->GetOverlappingActors(RightOverlaps);

            bool bLeftClear  = LeftBox  && LeftOverlaps.Num() == 0;
            bool bRightClear = RightBox && RightOverlaps.Num() == 0;

            if (bLeftClear || bRightClear)
            {
                bIsOvertaking = true;
                OvertakeStartTime = GetWorld()->GetTimeSeconds();
                TargetOffset = bLeftClear ? OffsetSize : -OffsetSize;

                UE_LOG(LogTemp, Warning, TEXT("[%s] Overtake START | Direction=%s"),
                    *ControllerVehicle->GetName(),
                    bLeftClear ? TEXT("LEFT") : TEXT("RIGHT")
                );
            }
        }
    }

    // LateralOffset 보간
    LateralOffset = FMath::FInterpTo(LateralOffset, TargetOffset, DeltaTime, 2.f);

    // 추월 종료
    if (bIsOvertaking && GetWorld()->GetTimeSeconds() - OvertakeStartTime > OvertakeDuration)
    {
        bIsOvertaking = false;
        TargetOffset = 0.f;
        UE_LOG(LogTemp, Warning, TEXT("[%s] Overtake END"), *ControllerVehicle->GetName());
    }
}

// =======================================================
// Front Vehicle
// =======================================================
AActor* AAIC_LearningVehicle::GetFrontVehicle()
{
    if (!FrontBox) return nullptr;

    TArray<AActor*> Overlaps;
    FrontBox->GetOverlappingActors(Overlaps);

    float MinDist = FLT_MAX;
    AActor* Closest = nullptr;

    for (AActor* A : Overlaps)
    {
        float Dist = FVector::Dist(A->GetActorLocation(), ControllerVehicle->GetActorLocation());
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Closest = A;
        }
    }

    return Closest;
}

// =======================================================
// Rank
// =======================================================
int AAIC_LearningVehicle::GetRaceRank()
{
    AllVehicles.Sort([](const AActor& A, const AActor& B)
    {
        return A.GetActorLocation().X > B.GetActorLocation().X;
    });

    for (int i = 0; i < AllVehicles.Num(); i++)
    {
        if (AllVehicles[i] == ControllerVehicle)
            return i + 1;
    }

    return AllVehicles.Num();
}

int AAIC_LearningVehicle::GetTotalRacers()
{
    return AllVehicles.Num();
}