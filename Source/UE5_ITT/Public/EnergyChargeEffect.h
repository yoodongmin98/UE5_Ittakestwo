// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnergyChargeEffect.generated.h"

UCLASS()
class UE5_ITT_API AEnergyChargeEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnergyChargeEffect();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	const float GetEffectLifeTime() const { return EffectLifeTime; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void EffectDestroy();
	void SetupDestroyTimerEvent();

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USceneComponent* SceneComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class UNiagaraComponent* EnergyChargeEffectComp = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float EffectLifeTime = 8.0f;
};
