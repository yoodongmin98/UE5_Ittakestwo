// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FxElectric.generated.h"

/**
 * 
 */
UCLASS()
class UE5_ITT_API AFxElectric : public AActor
{
	GENERATED_BODY()

public:
	AFxElectric(const FObjectInitializer& ObjectInitializer);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	enum class Fsm
	{
		ClientWait,
		Delay,
		Active,
		End,
		Wait,
	};

public:
	UFUNCTION(NetMulticast,Reliable)
	void NiagaraToggle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	class USoundManageComponent* SoundComp = nullptr;
	class UFsmComponent* FsmComp = nullptr;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* NiagaraComp = nullptr;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* BoxCollision = nullptr;

	class APlayerBase* ColPlayer = nullptr;

	UPROPERTY(EditAnywhere)
	bool bDelay = false;

	float ActiveTime = 0.f;

	float ClientWaitTime = 0.f;

	void SetupFsm();

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
