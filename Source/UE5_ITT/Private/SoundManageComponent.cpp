// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundManageComponent.h"
#include "Sound/SoundCue.h"
#include "GameManager.h"
#include "Kismet/GameplayStatics.h"

USoundManageComponent::USoundManageComponent()
{

}

void USoundManageComponent::MulticastPlaySoundLocation_Implementation(const FString& KeyName, FVector Location, float Min, float Max)
{
	USoundCue* TempCue = Cast<UGameManager>(GetWorld()->GetGameInstance())->GetSound(KeyName);
	if (TempCue != nullptr)
	{
		USoundAttenuation* AttenuationSet = NewObject<USoundAttenuation>();
		AttenuationSet->Attenuation.bAttenuate = true;
		AttenuationSet->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
		AttenuationOverrides.AttenuationShapeExtents = FVector(Min, 0.f, 0.f);
		AttenuationSet->Attenuation.FalloffDistance = Max-Min;

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), TempCue, Location,1.f,1.f,0.f, AttenuationSet);
	}

}

void USoundManageComponent::MulticastSoundStop_Implementation()
{
	Stop();
}

void USoundManageComponent::MulticastChangeSound_Implementation(const FString& KeyName, bool bPlay, float StartTime)
{	
	//매니저에게 사운드 불러오기
	USoundCue* TempCue = Cast<UGameManager>(GetWorld()->GetGameInstance())->GetSound(KeyName);

	//현재 사운드 변경
	if (TempCue !=nullptr)
	{
		SetSound(TempCue);

		if (bPlay)
		{
			Play(StartTime);
		}
	}

}

void USoundManageComponent::MulticastPlaySoundDirect_Implementation(const FString& KeyName)
{
	USoundCue* TempCue = Cast<UGameManager>(GetWorld()->GetGameInstance())->GetSound(KeyName);
	if (TempCue!=nullptr)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), TempCue);
	}
}

bool USoundManageComponent::IsSupportedForNetworking() const
{
	return true;
}

void USoundManageComponent::MulticastSetAttenuationDistance_Implementation(float Min, float Max)
{
	bOverrideAttenuation = true;
	bAllowSpatialization = true;
	AttenuationOverrides.bAttenuate = true;
	AttenuationOverrides.bSpatialize = true;
	AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
	AttenuationOverrides.AttenuationShapeExtents = FVector(Min, 0.f, 0.f);
	AttenuationOverrides.FalloffDistance = Max- Min;	
}
