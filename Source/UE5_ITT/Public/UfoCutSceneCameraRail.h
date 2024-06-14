// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParentCameraRail.h"
#include "UfoCutSceneCameraRail.generated.h"

/**
 *
 */
UCLASS()
class UE5_ITT_API AUfoCutSceneCameraRail : public AParentCameraRail
{
	GENERATED_BODY()

public:
	AUfoCutSceneCameraRail(const FObjectInitializer& ObjectInitializer);

	enum class Fsm
	{
		Wait,
		Delay0,
		Move,
		Delay1,
		Delay2,
		End,
	};
	virtual void Tick(float DeltaTime)override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* ColComp = nullptr;

	void SetupFsm();


	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	class ACody* TempCody = nullptr;
};
