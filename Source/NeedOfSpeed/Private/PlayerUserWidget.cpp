// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerUserWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"

void UPlayerUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 시작하자마자 깜빡임 애니메이션 무한 반복 재생 (PingPong 모드)
	if (Text_Glow)
	{
		PlayAnimation(Text_Glow, 0.0f, 0, EUMGSequencePlayMode::PingPong);
	}

	// 2. 위젯이 키 입력을 받을 수 있도록 포커스 설정
	SetIsFocusable(true);
}

FReply UPlayerUserWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bIsTransitioning) return FReply::Handled();
	bIsTransitioning = true;

	// 1. 깜빡임 중단
	StopAnimation(Text_Glow);

	// 2. 퇴장 애니메이션 재생 (단판 재생: 1)
	if (Text_Exit)
	{
		PlayAnimation(Text_Exit, 0.0f, 1);
	}

	// 3. 핵심: 화면 서서히 어두워지기 (Fade Out)
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->PlayerCameraManager)
	{
		// StartCameraFade(FromAlpha, ToAlpha, Duration, Color, bShouldFadeAudio, bHoldWhenFinished)
		// 0.0(투명)에서 1.0(검은색)으로 1.0초 동안 페이드!
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FColor::Black, false, true);
	}

	// 4. 페이드 아웃 시간(1.0초)만큼 기다렸다가 레벨 이동 (타이머 사용)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPlayerUserWidget::TransitionToRacingMap, 1.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("Any Key Pressed! Fading Out..."));
	return FReply::Handled();
}


void UPlayerUserWidget::TransitionToRacingMap()
{
	// 현재 월드의 플레이어 컨트롤러를 가져와 입력을 다시 게임용으로 돌려줍니다.
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	// 지정된 레벨로 이동!
	UGameplayStatics::OpenLevel(GetWorld(),FName("BetaMap"));
}