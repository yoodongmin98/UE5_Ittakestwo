// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "SoundManageComponent.generated.h"

/**
 * 
 */
UCLASS()
class UE5_ITT_API USoundManageComponent : public UAudioComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Reliable,NetMulticast)
	void MulticastChangeSound(const FString& KeyName,bool bPlay = true,float StartTime = 0.f);

	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void MulticastPlaySoundDirect(const FString& KeyName);
	
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void MulticastPlaySoundLocation(const FString& KeyName,FVector Location,float Attenuation = 0.f);

	bool IsSupportedForNetworking() const override;

protected:

private:

};
