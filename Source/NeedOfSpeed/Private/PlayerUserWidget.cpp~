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
	// 1. 중복 실행 방지
	if (bIsTransitioning) return FReply::Handled();
	bIsTransitioning = true;

	// 2. 루프 중인 깜빡임 애니메이션 강제 중지
	if (Text_Glow)
	{
		StopAnimation(Text_Glow);
	}

	// 3. 퇴장 애니메이션 재생 및 종료 이벤트 연결
	if (Text_Exit)
	{
		// 퇴장 애니메이션은 절대 Loop 되면 안 됩니다! (단판 재생: 1)
		PlayAnimation(Text_Exit, 0.0f, 1, EUMGSequencePlayMode::Forward);

		// 애니메이션 종료 시 실행될 함수 연결
		FWidgetAnimationDynamicEvent EndEvent;
		EndEvent.BindDynamic(this, &UPlayerUserWidget::TransitionToRacingMap);
		BindToAnimationFinished(Text_Exit, EndEvent);
        
		UE_LOG(LogTemp, Warning, TEXT("Exit Animation Started... Waiting for Finish."));
	}
	else
	{
		// 애니메이션 에셋이 없다면 즉시 이동 (보험 코드)
		TransitionToRacingMap();
	}

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
	UGameplayStatics::OpenLevel(GetWorld(),FName(""));
}