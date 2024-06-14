// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParentCameraRail.h"
#include "UfoCameraRail.generated.h"

/**
 * 
 */
UCLASS()
class UE5_ITT_API AUfoCameraRail : public AParentCameraRail
{
	GENERATED_BODY()

public:
	AUfoCameraRail(const FObjectInitializer& ObjectInitializer);

	enum class Fsm
	{
		Wait,
		Move,
	};
	virtual void Tick(float DeltaTime)override;

protected:
	virtual void BeginPlay() override;

private:
	void SetupFsm();
	class ACody* Cody = nullptr;
};
