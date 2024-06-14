// Fill out your copyright notice in the Description page of Project Settings.


#include "EnergyChargeEffect.h"
#include "NiagaraComponent.h"

// Sets default values
AEnergyChargeEffect::AEnergyChargeEffect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (true == HasAuthority())
	{
		bReplicates = true;
		SetReplicateMovement(true);
		
		SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
		SetRootComponent(SceneComp);

		EnergyChargeEffectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EnergyChargeEffectComponent"));
		EnergyChargeEffectComp->SetupAttachment(SceneComp);
	}
	
}

// Called when the game starts or when spawned
void AEnergyChargeEffect::BeginPlay()
{
	Super::BeginPlay();
	
	if (true == HasAuthority())
	{
		SetupDestroyTimerEvent();
	}
}

void AEnergyChargeEffect::EffectDestroy()
{
	Destroy();
}

void AEnergyChargeEffect::SetupDestroyTimerEvent()
{
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnergyChargeEffect::EffectDestroy, EffectLifeTime, false);
}

// Called every frame
void AEnergyChargeEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (true == HasAuthority())
	{
	}
}

