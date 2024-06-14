// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnTrigger.h"
#include "Components/ShapeComponent.h"
#include "PlayerBase.h"

ARespawnTrigger::ARespawnTrigger()
{
	PrimaryActorTick.bCanEverTick = true;

	if (true == HasAuthority())
	{
		bReplicates = true;
		SetReplicateMovement(true);

		RespawnPivotComp = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnPivotComp"));
		RespawnPivotComp->SetupAttachment(RootComponent);
	}
}

void ARespawnTrigger::BeginPlay()
{
	if (true == HasAuthority())
	{
		GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ARespawnTrigger::OnOverlapBegin);
	}
}

void ARespawnTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor!=this&& true == OtherActor->ActorHasTag("Player"))
	{
		APlayerBase* Player = Cast<APlayerBase>(OtherActor);
		Player->SetTriggerActors(this);
	}
}

