// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RacingCar.h"

// Fill out your copyright notice in the Description page of Project Settings.


#include "EnhancedInputComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/Engine.h"

ARacingCar::ARacingCar()
{
	// 변수 초기화
	PowerPlayGauge = 0.0f;
	
	ChaosMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
}

void ARacingCar::BeginPlay()
{
	Super::BeginPlay();

	// 게임이 시작되자마자 진짜 컴포넌트를 찾아서 변수에 꽉 채워둡니다.
	// 이렇게 해야 블루프린트의 Print String이 실행될 때 'None'이 안 뜹니다.
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
	}
}

void ARacingCar::Throttle(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	
	if(GEngine) GEngine->AddOnScreenDebugMessage(2, 0.5f, FColor::Cyan, FString::Printf(TEXT("Throttle Input: %f"), AxisValue));
    
	ChaosMovement->SetThrottleInput(AxisValue);
	ChaosMovement->SetThrottleInput(Value.Get<float>());
}

void ARacingCar::Brake(const FInputActionValue& Value)
{
	ChaosMovement->SetBrakeInput(Value.Get<float>());
}

void ARacingCar::Steer(const FInputActionValue& Value)
{
	ChaosMovement->SetSteeringInput(Value.Get<float>());
}



void ARacingCar::StartDrift()
{
	bDriftKeyPressed = true;
	bWasDriftingLastFrame = false; // 부스터 기록 초기화
	
	ChaosMovement->SetWheelFrictionMultiplier(0, 1.1f);
	ChaosMovement->SetWheelFrictionMultiplier(1, 1.1f);
	ChaosMovement->SetWheelFrictionMultiplier(2, DriftFrictionScale);
	ChaosMovement->SetWheelFrictionMultiplier(3, DriftFrictionScale);
	ChaosMovement->SetHandbrakeInput(true);
	
}

void ARacingCar::StopDrift()
{
	bDriftKeyPressed = false;
	
	ChaosMovement->SetWheelFrictionMultiplier(2, 1.0f);
	ChaosMovement->SetWheelFrictionMultiplier(3, 1.0f);
	ChaosMovement->SetHandbrakeInput(false);
	
	if (bWasDriftingLastFrame)
	{
		ApplyExitBoost();
	}
    
	bISDrifting = false;
	bWasDriftingLastFrame = false;
}
void ARacingCar::ApplyExitBoost()
{
	FVector BoostDirection = GetActorForwardVector();
    
	// 현재 속도를 가져와서 전방 방향으로 힘을 더한다
	FVector CurrentVelocity = GetVelocity();
	FVector NewVelocity = CurrentVelocity + (BoostDirection * DriftExitBoostForce );
    
	// 차체(Mesh)에 새로운 속도를 적용
	GetMesh()->SetPhysicsLinearVelocity(NewVelocity);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Boost!"));
	}
}

void ARacingCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector velocity = GetVelocity();
	FVector Forward = GetActorForwardVector();
	
	if (ChaosMovement)
	{
		float RPM = ChaosMovement->GetEngineRotationSpeed();
		float Speed = ChaosMovement->GetForwardSpeed();
       
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Red, 
				FString::Printf(TEXT("RPM: %.1f | Speed: %.1f"), RPM, Speed));
		}
	}
	
	if (bDriftKeyPressed&&velocity.Size() > 500.0f)
	{
		float CosAngle = FVector::DotProduct(velocity.GetSafeNormal(), Forward);
		// 시작 조건: 0.94 (약 20도 이상 꺾여야 시작)
		// 유지 조건: 0.98 (거의 펴져도 Shift만 누르고 있으면 유지)
		float Threshold = bISDrifting ? 0.98f : 0.97f;

		if (CosAngle < Threshold && bDriftKeyPressed)
		{
			bISDrifting = true;
			bWasDriftingLastFrame = true;
			
			if (bISDrifting)
			{
					// 현재 속도 벡터의 반대 방향으로 약한 힘을 가함
					FVector AntiVelocity = (-GetVelocity() * 0.5f);
					GetMesh()->AddForce(AntiVelocity * GetMesh()->GetMass());
    
					// 단순 속도 감쇄
					// FVector CurrentVel = GetVelocity();
				    //매 프레임 2%씩 감속
					// GetMesh()->SetPhysicsLinearVelocity(CurrentVel * 0.98f);
				}
			// 카운터 스티어링 시작
			if (bDriftKeyPressed&&velocity.Size() > 10000.f)
			{
				// 차가 왼쪽으로 도는지 오르쪽으로 도는지 판별(외적)
				FVector CrossProduct = FVector::CrossProduct(Forward, velocity);
			
				// 미끄러지는 반대방향으로 조향값추가`
				float DriftAngle = CrossProduct.Z;
				
				ChaosMovement->SetWheelFrictionMultiplier(0, 0.3f);
				ChaosMovement->SetWheelFrictionMultiplier(1, 0.3f);
			
				if(ChaosMovement)
				{
					float CurrentSteer = ChaosMovement->GetSteeringInput();
					float CounterSteerAmount = DriftAngle * 0.5f;
					ChaosMovement->SetSteeringInput(FMath::Clamp(CurrentSteer + CounterSteerAmount, -1.0f, 1.0f));
				}
			}
			// 게이지 충전 속도도 조절
			PowerPlayGauge = FMath::Clamp(PowerPlayGauge + (DriftGaugeRate * DeltaTime), 0.0f, 3.0f);
			if(GEngine) GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Red, TEXT("DRIFTING!!!"));
		}
		else
		{
			bISDrifting = false;
		}
	}
};