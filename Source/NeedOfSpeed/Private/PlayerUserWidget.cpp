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

	// 1. 텍스트 깜빡임 정지 및 퇴장 애니메이션
	StopAnimation(Text_Glow);
	if (Text_Exit)
	{
		PlayAnimation(Text_Exit, 0.0f, 1);
	}
    
	// 2. 카메라 페이드 아웃 (월드 배경이 3초 동안 서서히 까맣게 변함)
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 3.0f, FColor::Black, false, true);
	}

	// 3. UI 로딩창(동그라미+텍스트) 애니메이션 재생
	// (카메라 페이드와 동시에 화면 앞쪽에 동그라미가 나타납니다)
	if (Loading_In)
	{
		PlayAnimation(Loading_In, 0.0f, 1);
	}

	// 4. 타이머: 페이드 아웃 시간(3초)에 정확히 맞춰서 BetaMap으로 이동!
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPlayerUserWidget::TransitionToRacingMap, 3.0f, false);

	UE_LOG(LogTemp, Warning, TEXT("Fading Out and Showing Loading UI..."));
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
	UGameplayStatics::OpenLevel(GetWorld(),FName("Lv_BetaMap"));
}
