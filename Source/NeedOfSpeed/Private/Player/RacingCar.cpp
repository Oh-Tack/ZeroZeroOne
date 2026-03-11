// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RacingCar.h"

// Fill out your copyright notice in the Description page of Project Settings.


#include "EnhancedInputComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "DataWrappers/ChaosVDParticleDataWrapper.h"
#include "Engine/Engine.h"
#include "Components/PointLightComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"


ARacingCar::ARacingCar()
{
	// 변수 초기화
	PowerPlayGauge = 0.0f;
	
	ChaosMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
	
	// 1. 왼쪽 드리프트 연기 부착
	DriftSmokeLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DriftSmokeLeft"));
	DriftSmokeLeft->SetupAttachment(GetMesh()); // 나중에 블루프린트에서 위치 조정
	DriftSmokeLeft->SetAutoActivate(false);

	// 2. 오른쪽 드리프트 연기 부착
	DriftSmokeRight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DriftSmokeRight"));
	DriftSmokeRight->SetupAttachment(GetMesh());
	DriftSmokeRight->SetAutoActivate(false);

	// 3. 부스트 효과음
	BoostAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BoostAudio"));
	BoostAudio->SetupAttachment(GetMesh());
	BoostAudio->SetAutoActivate(false);
	
	// 4. 드리프트 효과음
	DriftAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("DriftAudio"));
	DriftAudio->SetupAttachment(GetMesh());
	DriftAudio->SetAutoActivate(false); // 시작할 땐 소리 끄기
	
	// 엔진 오디오 생성 및 부착 
	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(GetMesh());
    
	// 엔진은 시동이 걸려있어야 하니 true로 설정
	EngineAudio->SetAutoActivate(true);
	
	RearSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("RearSpringArm"));
	RearSpringArm->SetupAttachment(GetMesh());
	RearSpringArm->TargetArmLength = 600.0f; // 적절한 거리
	RearSpringArm->SetRelativeRotation(FRotator(-15.f, 180.f, 0.f)); // 뒤를 보도록 180도 회전
	RearSpringArm->bDoCollisionTest = true;

	// 후방 카메라 추가
	RearCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("RearCamera"));
	RearCamera->SetupAttachment(RearSpringArm);
	RearCamera->SetAutoActivate(false); // 처음엔 꺼둠
}

void ARacingCar::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<UPointLightComponent*> AllPointLights;
	GetComponents<UPointLightComponent>(AllPointLights);
	
	TArray<UCameraComponent*> CameraComps;
	GetComponents<UCameraComponent>(CameraComps);

	for (UCameraComponent* Cam : CameraComps)
	{
		// 우리가 C++에서 새로 만든 RearCamera가 아닌 것을 전방 카메라로 간주합니다.
		if (Cam != RearCamera)
		{
			FrontCamera = Cam;
			break;
		}
	}

	// Tag달아놓은 애들만 배열에 저장.
	for (UPointLightComponent* Light : AllPointLights)
	{
		if (Light->ComponentHasTag(TEXT("Brake")))
		{
			BrakeLights.Add(Light);
            
			// 시작시 평소 밝기 맞춰줌.
			Light->SetIntensity(NormalBrakeIntensity); 
		}
	}
	
	ChaosMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetComponentByClass(UChaosWheeledVehicleMovementComponent::StaticClass()));

	if (ChaosMovement)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChaosMovement 초기화 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ChaosMovement를 찾을 수 없음"));
	}
}

void ARacingCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 엑셀
		if (IA_Throttle)
		{
			EnhancedInputComponent->BindAction(IA_Throttle, ETriggerEvent::Triggered, this, &ARacingCar::Throttle);
			EnhancedInputComponent->BindAction(IA_Throttle, ETriggerEvent::Completed, this, &ARacingCar::Throttle);
		}
		// 브레이크
		if (IA_Brake)
		{
			EnhancedInputComponent->BindAction(IA_Brake, ETriggerEvent::Triggered, this, &ARacingCar::Brake);
			EnhancedInputComponent->BindAction(IA_Brake, ETriggerEvent::Completed, this, &ARacingCar::Brake);
		}

		// 조향
		if (IA_Steer)
		{
			EnhancedInputComponent->BindAction(IA_Steer, ETriggerEvent::Triggered, this, &ARacingCar::Steer);
			EnhancedInputComponent->BindAction(IA_Steer, ETriggerEvent::Completed, this, &ARacingCar::Steer);
		}
		
		// 드리프트
		if (IA_Drift)
		{
			EnhancedInputComponent->BindAction(IA_Drift, ETriggerEvent::Started, this, &ARacingCar::StartDrift);
			EnhancedInputComponent->BindAction(IA_Drift, ETriggerEvent::Completed, this, &ARacingCar::StopDrift);
		}
		
		// 뒤보는 카메라
		if (IA_LookBack)
		{
			EnhancedInputComponent->BindAction(IA_LookBack, ETriggerEvent::Started, this, &ARacingCar::StartLookBack);
			EnhancedInputComponent->BindAction(IA_LookBack, ETriggerEvent::Completed, this, &ARacingCar::StopLookBack);
		}
	}
}

void ARacingCar::Throttle(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	
    
	ChaosMovement->SetThrottleInput(AxisValue);
	ChaosMovement->SetThrottleInput(Value.Get<float>());
}

void ARacingCar::Brake(const FInputActionValue& Value)
{
	ChaosMovement->SetBrakeInput(Value.Get<float>());
	
	float BrakeInput = Value.Get<float>();
	ChaosMovement->SetBrakeInput(BrakeInput);

	// 켜질지 꺼질지 목표 밝기 결정
	float TargetIntensity = (BrakeInput > 0.0f) ? ActiveBrakeIntensity : NormalBrakeIntensity;

	// 4개의 라이트 밝기를 동시에 업데이트
	for (UPointLightComponent* Light : BrakeLights)
	{
		if (Light)
		{
			Light->SetIntensity(TargetIntensity);
		}
	}
}

void ARacingCar::Steer(const FInputActionValue& Value)
{
	float SteerValue = Value.Get<float>();
	ChaosMovement->SetSteeringInput(SteerValue);

	if (!GetMesh()) return;

	// 1. 각속도(Yaw) 제어: 팽이 현상 방지 및 묵직한 회전
	float MaxYawRate = bISDrifting ? 90.0f : 60.0f; // 드리프트 중일 때 더 많이 회전
	float TargetYawRate = SteerValue * MaxYawRate;
	FVector AngularVel = GetMesh()->GetPhysicsAngularVelocityInDegrees();
    
	// 보간 속도(InterpSpeed)를 조절하여 회전의 기민함을 튜닝
	float NewYawVel = FMath::FInterpTo(AngularVel.Z, TargetYawRate, GetWorld()->GetDeltaSeconds(), 10.0f);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector(AngularVel.X, AngularVel.Y, NewYawVel));

	// 2. 진행 방향 보정 (Velocity Alignment)
	// 차가 미끄러지지 않고 레일 위를 달리는 느낌주기위함.
	FVector CurrentVelocity = GetVelocity();
	float Speed = CurrentVelocity.Size();

	if (Speed > 1000.0f) // 고속 주행 시에만 작동
	{
		FVector ForwardVector = GetActorForwardVector();
		// 드리프트 중에는 보정치를 낮춰서 약간 미끄러지게, 평소에는 높여서 꽉 붙잡게 설정
		float AlignmentStrength = bISDrifting ? 1.5f : 5.0f; 
        
		FVector DesiredVelocity = ForwardVector * Speed;
		FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, DesiredVelocity, GetWorld()->GetDeltaSeconds(), AlignmentStrength);
        
		// Z축 속도는 유지하여 점프나 경사로 대응
		NewVelocity.Z = CurrentVelocity.Z; 
		GetMesh()->SetPhysicsLinearVelocity(NewVelocity);
	}
}



void ARacingCar::StartDrift()
{
	bDriftKeyPressed = true;
	bDriftKeyPressed = true;
	// 진입 시에만 살짝 뒤를 날려주기 위해 순간적인 토크주기.
	float EntrySide = ChaosMovement->GetSteeringInput();
	GetMesh()->AddTorqueInDegrees(FVector(0, 0, EntrySide * 50000000.0f), NAME_None, false); 
	
	// 뒷바퀴 마찰력은 0.5f 정도로 유지하여 너무 미끄러지지 않게 하기.
	ChaosMovement->SetWheelFrictionMultiplier(2, DriftFrictionScale);
	ChaosMovement->SetWheelFrictionMultiplier(3, DriftFrictionScale);
	
	for (UPointLightComponent* Light : BrakeLights)
	{
		if (Light) Light->SetIntensity(ActiveBrakeIntensity);
	}
	
}

void ARacingCar::StopDrift()
{
	bDriftKeyPressed = false;
	if (bISDrifting && PowerPlayGauge > 0.1f) // 최소 게이지 조건
	{
		bIsBoosting = true; // 부스트 시작
	}
    
	bISDrifting = false;
	
	if (ChaosMovement)
	{
		for (int32 i = 0; i < 4; i++)
		{
			ChaosMovement->SetWheelFrictionMultiplier(i, 1.0f);
		}
		ChaosMovement->SetHandbrakeInput(false);
		// 조향 입력 초기화 인풋 설정 새로 받기 위해서.
		ChaosMovement->SetSteeringInput(0.0f); 
	}
	
	if (bWasDriftingLastFrame)
	{
		ApplyExitBoost();
	}
    
	bISDrifting = false;
	bWasDriftingLastFrame = false;
	
	
	float TargetIntensity = (ChaosMovement->GetBrakeInput() > 0.0f) ? ActiveBrakeIntensity : NormalBrakeIntensity;
	for (UPointLightComponent* Light : BrakeLights)
	{
		if (Light) Light->SetIntensity(TargetIntensity);
	}
}

void ARacingCar::ApplyExitBoost()
{
	FVector BoostDirection = GetActorForwardVector();
    
	// 현재 속도를 가져와서 전방 방향으로 힘을 더한다
	FVector CurrentVelocity = GetVelocity();
	FVector NewVelocity = CurrentVelocity + (BoostDirection * DriftExitBoostForce );
    
	// 차체(Mesh)에 새로운 속도를 적용

	GetMesh()->AddImpulse(BoostDirection * DriftExitBoostForce, NAME_None, true);
	
}


void ARacingCar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    FVector velocity = GetVelocity();
    FVector Forward = GetActorForwardVector();
    float CurrSpeed = velocity.Size();
    
    // 카메라 컨트롤러 가져오기
    APlayerController* PC = Cast<APlayerController>(GetController());
	
	if (EngineAudio)
	{
		// 1. 현재 차의 속도를 구합니다.
		float CurrentSpeed = GetVelocity().Size(); 

		// 2. 속도에 맞춰 소리 높낮이(Pitch)를 계산합니다.
		// 속도가 0일 때 Pitch는 0.8 (낮은 구르릉 소리)
		// 속도가 3000(예상 최고속도)일 때 Pitch는 2.0 (높은 우아앙 소리)
		FVector2D SpeedRange(0.0f, 3000.0f); // 맵핑할 속도 구간
		FVector2D PitchRange(0.8f, 2.0f);    // 맵핑할 음정 구간
        
		// 언리얼 수학 함수: 현재 속도를 위 구간에 맞춰 변환해줌!
		float TargetPitch = FMath::GetMappedRangeValueClamped(SpeedRange, PitchRange, CurrentSpeed);

		// 3. 계산된 Pitch를 오디오에 적용
		EngineAudio->SetPitchMultiplier(TargetPitch);
	}

 
    // 고속 주행 카메라 쉐이크
    
    if (CurrSpeed > 2500.0f) // 고속 기준 (필요에 따라 조절)
    {
        if (HighSpeedShakeClass && !ActiveHighSpeedShake && PC && PC->PlayerCameraManager)
        {
            // 강도 0.5 정도로 은은하게 흔들림
            ActiveHighSpeedShake = PC->PlayerCameraManager->StartCameraShake(HighSpeedShakeClass, 0.1f);
        }
    }
    else
    {
        if (ActiveHighSpeedShake && PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StopCameraShake(ActiveHighSpeedShake);
            ActiveHighSpeedShake = nullptr;
        }
    }

	if (bIsBoosting)
	{
		// 부스트 소리 켜기
		if (BoostAudio && !BoostAudio->IsPlaying()) BoostAudio->Play();

		// (나머지 부스트 가속 및 카메라 쉐이크 로직 유지)
	}
	else
	{
		// 부스트 끝나면 소리 끄기
		if (BoostAudio && BoostAudio->IsPlaying()) BoostAudio->Stop();
	}
	
	float CosAngle = FVector::DotProduct(velocity.GetSafeNormal(), Forward);
	if (bISDrifting)
	{
		// 연기 뿜기 시작!
		if (DriftSmokeLeft && !DriftSmokeLeft->IsActive()) DriftSmokeLeft->Activate();
		if (DriftSmokeRight && !DriftSmokeRight->IsActive()) DriftSmokeRight->Activate();
		if (DriftAudio)
		{
			if (!DriftAudio->IsPlaying()) 
			{
				DriftAudio->Play();
			}

			// [핵심] 미끄러짐 강도 계산
			// 단순히 켜고 끄는 게 아니라, 현재 속도와 드리프트 각도(CosAngle)를 이용해 강도를 계산합니다.
			// CosAngle이 작을수록(옆으로 더 많이 꺾일수록) 소리가 커지게 설정합니다.
			float SlipIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.7f, 0.95f), FVector2D(1.2f, 0.0f), CosAngle);
			float NormalizedSpeed = FMath::Clamp(CurrSpeed / 3000.0f, 0.5f, 1.5f);
            
			// 최종 강도 (속도와 미끄러짐의 조합)
			float FinalSkidPower = SlipIntensity * NormalizedSpeed;

			// 방법 A: 메타사운드를 사용 중일 때 (파라미터 전달)
			DriftAudio->SetFloatParameter(SkidIntensityParam, FinalSkidPower);

			// 방법 B: 사운드 큐나 일반 웨이브를 사용할 때 (C++에서 직접 제어)
			// 반복 재생이 티 나지 않도록 피치를 계속 흔들어줍니다.
			float DynamicPitch = 0.9f + (FinalSkidPower * 0.4f); // 강도에 따라 피치 변화
			DriftAudio->SetPitchMultiplier(DynamicPitch);
			DriftAudio->SetVolumeMultiplier(FMath::Min(FinalSkidPower, 1.0f));
		}

		// (나머지 감속 및 카운터 스티어링 로직 유지)
	}
	else
	{
		// 드리프트가 끝나거나 직진 중이면 연기 끄기
		if (DriftSmokeLeft && DriftSmokeLeft->IsActive()) DriftSmokeLeft->Deactivate();
		if (DriftSmokeRight && DriftSmokeRight->IsActive()) DriftSmokeRight->Deactivate();
		if (DriftAudio && DriftAudio->IsPlaying())
		{
			DriftAudio->FadeOut(0.2f, 0.0f); // 0.2초 동안 서서히 꺼짐
		}
	}
	
	
    // 1. 게이지 소모 및 부스트 로직
    
    if (bIsBoosting)
    {

        // 부스트 쉐이크 켜기
        if (BoostShakeClass && !ActiveBoostShake && PC && PC->PlayerCameraManager)
        {
            
            ActiveBoostShake = PC->PlayerCameraManager->StartCameraShake(BoostShakeClass, 0.3f);
        }

        if (PowerPlayGauge > 0.0f)
        {
            PowerPlayGauge = FMath::Max(0.0f, PowerPlayGauge - (BoostConsumptionRate * DeltaTime));

            FVector BoostForce = Forward * BoostAccelerationForce;
            GetMesh()->AddForce(BoostForce * GetMesh()->GetMass());
        	
        }
        else
        {
            PowerPlayGauge = 0.0f;
            bIsBoosting = false;
        }
    }
    else
    {
        // 부스트가 끝났으면 쉐이크 끄기
        if (ActiveBoostShake && PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StopCameraShake(ActiveBoostShake);
            ActiveBoostShake = nullptr;
        }
    }

    
    // 2. 드리프트 및 게이지 충전 로직 
    
    if (bDriftKeyPressed && CurrSpeed > 500.0f)
    {
       
        float Threshold = bISDrifting ? 0.98f : 0.94f;
        
        FVector AngularVel = GetMesh()->GetPhysicsAngularVelocityInDegrees();
        if (FMath::Abs(AngularVel.Z) > 120.0f)
        {
            GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector(0, 0, AngularVel.Z * 0.9f));
        }

        if (CosAngle < Threshold)
        {
            bISDrifting = true;
            
            if (!bIsBoosting)
            {
                PowerPlayGauge = FMath::Clamp(PowerPlayGauge + (DriftGaugeRate * DeltaTime), 0.0f, 3.0f);
            }

            FVector AntiVelocity = (-velocity * 0.05f);
            GetMesh()->AddForce(AntiVelocity * GetMesh()->GetMass());

            if (ChaosMovement && CurrSpeed > 2000.f)
            {
                FVector CrossProduct = FVector::CrossProduct(Forward, velocity.GetSafeNormal());
                float DriftDirection = CrossProduct.Z;
                
                ChaosMovement->SetWheelFrictionMultiplier(0, 0.3f);
                ChaosMovement->SetWheelFrictionMultiplier(1, 0.3f);
                
                float CurrentSteer = ChaosMovement->GetSteeringInput();
                float CounterSteerAmount = DriftDirection * 0.5f;
                ChaosMovement->SetSteeringInput(FMath::Clamp(CurrentSteer + CounterSteerAmount, -1.0f, 1.0f));
            }
        	
        }
        else
        {
            bISDrifting = false;
        }
    }
    else
    {
        bISDrifting = false;
    }

	UCameraComponent* MyCamera = FindComponentByClass<UCameraComponent>();
    
	if (MyCamera)
	{
		float TargetFOV = bIsBoosting ? BoostFOV : NormalFOV;
		float CurrentFOV = MyCamera->FieldOfView;
        
		// FMath::FInterpTo를 사용해 현재 시야각에서 목표 시야각으로 부드럽게 줌 인/아웃
		MyCamera->SetFieldOfView(FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 3.0f));
	}
}

void ARacingCar::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	
	float ImpactSpeed = GetVelocity().Size();

	
	if (ImpactSpeed > 500.0f && ImpactShakeClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC && PC->PlayerCameraManager)
		{
		
			float ShakeScale = FMath::Clamp(ImpactSpeed / 1000.0f, 0.5f, 3.0f);
            
			PC->PlayerCameraManager->StartCameraShake(ImpactShakeClass, ShakeScale);
            
		}
	}
}


void ARacingCar::StartLookBack()
{
	if (FrontCamera && RearCamera)
	{
		FrontCamera->SetActive(false);
		RearCamera->SetActive(true);
	}
}

void ARacingCar::StopLookBack()
{
	if (FrontCamera && RearCamera)
	{
		RearCamera->SetActive(false);
		FrontCamera->SetActive(true);
	}
}


// 결승선 통과
void ARacingCar::PassFinishLine()
{
	if (bCanLap && CurrentLap <= TotalLaps)
	{
		CurrentLap++;
		bCanLap = false;
	}
		
}



