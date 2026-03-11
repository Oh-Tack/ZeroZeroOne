// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class NEEDOFSPEED_API UPlayerUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
	// 위젯이 생성될 때 실행 (초기화)
	virtual void NativeConstruct() override;

	// 키 입력을 가로채는 함수
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// --- 에디터와 연결되는 변수들 ---
	
	// 깜빡이는 애니메이션 (이름이 같아야 함)
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Text_Glow;

	// 사라지는 애니메이션 (방금 만드신 것)
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Text_Exit;

	// "PRESS ANY KEY" 텍스트 블록
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PressAnyKeyText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	FName RacingMapName = TEXT("Map_RacingTrack");

	// 애니메이션이 끝난 후 호출할 함수
	UFUNCTION()
	void TransitionToRacingMap();

private:
	bool bIsTransitioning = false; // 중복 입력 방지용
};

