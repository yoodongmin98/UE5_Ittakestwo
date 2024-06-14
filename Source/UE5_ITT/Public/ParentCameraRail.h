// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraRig_Rail.h"
#include "ParentCameraRail.generated.h"

/**
 * 
 */
UCLASS()
class UE5_ITT_API AParentCameraRail : public ACameraRig_Rail
{
	GENERATED_BODY()

public:
	AParentCameraRail(const FObjectInitializer& ObjectInitializer);


	virtual void Tick(float DeltaTime)override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetCameraView(float BlendTime = 0.f);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCameraViewReset(float BlendTime = 0.f);
protected:
	virtual void BeginPlay() override;
	virtual bool ShouldTickIfViewportsOnly() const override;
	virtual bool IsSupportedForNetworking() const override;
	void ResetScreenView();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastMoveRailCamera(float RailRatio);

	UPROPERTY(EditAnywhere)
	class UCameraComponent* CamComp = nullptr;
	class UFsmComponent* FsmComp;
	float CamMoveRatio = 1.f;
	bool bStart = false;

private:
	AActor* PrevViewTarget = nullptr;
};
