// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "PowerplayZoneBase.generated.h"

UCLASS()
class NEEDOFSPEED_API APowerplayZoneBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APowerplayZoneBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> ZoneBox;

	// 존 진입 시 뜨는 E키 아이콘 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> IconWidgetClass;

	// E 누른 후 뜨는 POWERPLAY 텍스트 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> TextWidgetClass;

	// POWERPLAY 텍스트 표시 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float TextDisplayDuration = 2.f;

protected:
	bool bPlayerInZone = false;
	bool bTriggered = false;

	// 자식 클래스에서 실제 이벤트 발동
	virtual void OnPowerplayTriggered() {}

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> IconWidgetInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> TextWidgetInstance;

	void ShowIconWidget();
	void HideIconWidget();
	void ShowTextWidget();
	void HideTextWidget();

	void OnEPressed();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
