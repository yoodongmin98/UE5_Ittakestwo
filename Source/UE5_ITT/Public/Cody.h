// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"
#include "EnemyFlyingSaucer.h"
#include "Cody.generated.h"







UCLASS()
class UE5_ITT_API ACody : public APlayerBase
{
public:
	GENERATED_BODY()
	ACody();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	//CodySize Enum을 변경합니다
	void ChangeCodySizeEnum(CodySize _Enum)
	{
		NextCodySize = _Enum;
	}
	//Cody의 Size를 Enum으로 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline CodySize GetCodySize() const
	{
		return CurCodySize;
	}
	//변환하고자하는 Size의 TargetScale을 반환합니다
	UFUNCTION(BlueprintCallable)
	inline FVector GetTargetScale() const
	{
		return TargetScale;
	}

	UFUNCTION(Client, Reliable)
	void ChangeBigSize();

	UFUNCTION(Client, Reliable)
	void ChangeSmallSize();



	UFUNCTION(Server, Reliable, WithValidation)
	void ChangeServerBigSize();

	UFUNCTION(Server, Reliable, WithValidation)
	void ChangeServerSmallSize();

	UFUNCTION(BlueprintCallable)
	inline bool GetCodyHoldEnemy() const
	{
		return CodyHoldEnemy;
	}



	UFUNCTION(BlueprintCallable)
	inline bool GetCutScenceTrigger() const
	{
		return CutsceneTrigger;
	}


	


	virtual void SprintInput() override;
	UFUNCTION(Client, Reliable)
	void ClientSprintInput();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSprintInput();

	virtual void DashEnd() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(Replicated)
	bool CutsceneTrigger;

	UFUNCTION(BlueprintCallable)
	inline void CutScenceStart();

	UFUNCTION(Client, Reliable)
	void CustomClientCutScene();
	UFUNCTION(Server, Reliable, WithValidation)
	void CustomServerCutScene();

	UFUNCTION(BlueprintCallable)
	void SetCodyMoveable();

	UFUNCTION(Client, Reliable)
	void CustomClientMoveable();
	UFUNCTION(Server, Reliable, WithValidation)
	void CustomServerMoveable();

	void Sit();
	UFUNCTION(Client, Reliable)
	void ClientSitStart();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSitStart();
	void SitEnd();
	UFUNCTION(Client, Reliable)
	void ClientSitEnd();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSitEnd();

	//UFUNCTION(Client, Reliable)
	//void ClientCameraLengthChange(float _Length, float _DeltaTime, float _CameraSpeed);
	//UFUNCTION(Server, Reliable, WithValidation)
	//void ServerCameraLengthChange(float _Length, float _DeltaTime, float _CameraSpeed);
private:
	//Size
	FVector BigSize;
	FVector NormalSize;
	FVector SmallSize;
	FVector TargetScale;

	//Transform
	FTransform CodyCapsuleComponent;
	FTransform CodyTransform;
	FTransform CodyLocalTransform;

	float CodyDefaultSpeed;

	float SpringArmLerpTime;
	float CurSprintArmLength;
	UPROPERTY(Replicated)
	float SpringArmLength;
	UPROPERTY(Replicated)
	float CameraSpeed;

	UPROPERTY(EditAnywhere)
	AEnemyFlyingSaucer* EnemyBoss = nullptr;
};
