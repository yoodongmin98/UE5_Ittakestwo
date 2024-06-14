// Fill out your copyright notice in the Description page of Project Settings.


#include "BurstEffect.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerBase.h"
#include "Kismet/GameplayStatics.h"
#include "SoundManageComponent.h"

// Sets default values
ABurstEffect::ABurstEffect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (true == HasAuthority())
	{
		Tags.Add(FName("BurstEffect"));

		bReplicates = true;
		SetReplicateMovement(true);

		SetupComponent();
	}
	
}

// Called when the game starts or when spawned
void ABurstEffect::BeginPlay()
{
	Super::BeginPlay();
	
	if (true == HasAuthority())
	{
		SoundComp->AttachToComponent(StaticMeshComp, FAttachmentTransformRules::KeepRelativeTransform);
		SoundComp->MulticastPlaySoundLocation(TEXT("LaserBomb_Explosion_Cue"), GetActorLocation());

		StaticMeshComp->OnComponentBeginOverlap.AddDynamic(this, &ABurstEffect::OnOverlapBegin);
		StaticMeshComp->OnComponentEndOverlap.AddDynamic(this, &ABurstEffect::OnOverlapEnd);
	}
}

void ABurstEffect::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (nullptr != OtherActor)
	{
		if (true == OtherActor->ActorHasTag(TEXT("Player")))
		{
			bIsPlayerOverlap = true;
			OverlapPlayer = Cast<APlayerBase>(OtherActor);
	 	}
	}
}

void ABurstEffect::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (nullptr != OtherActor)
	{
		if (true == OtherActor->ActorHasTag(TEXT("Player")))
		{
			bIsPlayerOverlap = false;
			OverlapPlayer = nullptr;
		}
	}
}

void ABurstEffect::SetupComponent()
{
	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComp);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BurstEffectMesh"));
	StaticMeshComp->SetupAttachment(SceneComp);

	SoundComp = CreateDefaultSubobject<USoundManageComponent>(TEXT("SoundManageComponent"));
}

void ABurstEffect::TickIncreaseScale(float DeltaTime)
{
	FVector NewScale = GetActorScale3D() + FVector::OneVector * 3.0f * DeltaTime;
	GetRootComponent()->SetWorldScale3D(NewScale);
	if (NewScale.X >= MaxScale)
	{
		SetupDestroyTimerEvent();
		bDestroyValue = true;
	}
}

void ABurstEffect::SetupDestroyTimerEvent()
{
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABurstEffect::EffectDestroy, 1.0f, false);
}

// Called every frame
void ABurstEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 네트워크 권한을 확인하는 코드
	if (true == HasAuthority())
	{
		if (true == bDestroyValue)
		{
			return;
		}

		TickIncreaseScale(DeltaTime);

		if (true == bIsPlayerOverlap)
		{
			// OverlapPlayer->AttackPlayer(DamageToPlayer);
		}
	}
}

void ABurstEffect::EffectDestroy()
{
	Destroy();
}
