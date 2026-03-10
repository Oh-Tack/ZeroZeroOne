#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SSGVehicleBase.generated.h"

// 파괴 이벤트 델리게이트 (누가 파괴됐는지 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleDestroyed, AActor*, DestroyedVehicle);

UCLASS()
class NEEDOFSPEED_API ASSGVehicleBase : public APawn
{
	GENERATED_BODY()

public:
	ASSGVehicleBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	FTimerHandle Handle;

	// 차량 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* MeshComp;

	// 파괴 이펙트
	UPROPERTY(EditDefaultsOnly, Category="Effects")
	UParticleSystem* DestroyEffect;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	float LaunchMultiplier = 1000.f;

	// 리스폰
	UPROPERTY(EditDefaultsOnly, Category="Respawn")
	float RespawnTime = 5.f;

	// 파괴 플래그
	UPROPERTY(BlueprintReadOnly)
	bool bDestroyCar = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsRespawning = false;

	// 충돌 델리게이트
	UPROPERTY(BlueprintAssignable, Category="Vehicle|Events")
	FOnVehicleDestroyed OnVehicleDestroyed;

	// -------------------------
	// 충돌 처리
	UFUNCTION()
	virtual void OnVehicleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	// -------------------------
	// 리스폰 처리
	virtual void RespawnVehicle();

	// -------------------------
	// 유틸
	virtual float CalculateImpactScore(const FVector& NormalImpulse, UPrimitiveComponent* OtherComp);
};