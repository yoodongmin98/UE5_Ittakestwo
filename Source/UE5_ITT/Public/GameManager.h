// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameManager.generated.h"

/**
 * 
 */
UCLASS()
class UE5_ITT_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	void AddCameraRigRail(const FString& KeyName, class AParentCameraRail* _CameraRigRail);
	void ChangeCameraView(const FString& KeyName, float BlendTime);
	UFUNCTION(BlueprintCallable)
	bool IsGameStart()
	{
		return bGameStart;
	}

	UFUNCTION(BlueprintCallable)
	void SetGameStart()
	{
		bGameStart = true;
	}
	UFUNCTION(BlueprintCallable)
	void LoadSound();
	class USoundCue* GetSound(const FString& KeyName);

protected:

private:
	bool bGameStart = false;
	TMap<FString,class AParentCameraRail*> MapCamera;
	class UObjectLibrary* ObjLibrary = nullptr;

	TMap<FString, class USoundCue*> SoundList;
};
