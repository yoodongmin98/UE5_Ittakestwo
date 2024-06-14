// Fill out your copyright notice in the Description page of Project Settings.


#include "Smasher.h"
#include <Components/StaticMeshComponent.h>
#include <Components/SceneComponent.h>
#include "FsmComponent.h"
#include "PlayerBase.h"
#include "ITTGameModeBase.h"
#include "Components/BoxComponent.h"
#include "SoundManageComponent.h"
#include "GameManager.h"

// Sets default values
ASmasher::ASmasher()
{	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (HasAuthority() == true)
	{
		SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
		SetRootComponent(SceneComp);

		SmasherMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SmasherMesh"));
		SmasherMesh->SetupAttachment(SceneComp);

		BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCol"));
		BoxCollision->SetupAttachment(SmasherMesh);

		SoundComp = CreateDefaultSubobject<USoundManageComponent>(TEXT("SoundComp"));

		SoundComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		SetupFsm();
	}
}

// Called when the game starts or when spawned
void ASmasher::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		FsmComp->ChangeState(Fsm::SmashWait);
		ReleasePos = SmasherMesh->GetRelativeLocation();
		SmashPos = ReleasePos;
		SmashPos.X -= SmasherMesh->GetRelativeScale3D().X * SmashSize;
		BoxCollision->OnComponentBeginOverlap.AddDynamic(this,&ASmasher::OnOverlapBegin);
		BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ASmasher::OnOverlapEnd);
	}
}

void ASmasher::SetupFsm()
{
	FsmComp = CreateDefaultSubobject<UFsmComponent>(TEXT("FsmComp"));

	FsmComp->CreateState(Fsm::SmashWait,
		[this]
		{

		},

		[this](float DeltaTime)
		{
			if (true == Cast<UGameManager>(GetWorld()->GetGameInstance())->IsGameStart())
			{
				FsmComp->ChangeState(Fsm::Smash);
			}
		},

		[this]
		{
		}
	);

	FsmComp->CreateState(Fsm::Smash,
		[this]
		{
			SoundComp->MulticastSetAttenuationDistance(250.f, 300.f);
			SmashRatio = 0.f;
		},

		[this](float DeltaTime)
		{
			if (ColPlayer != nullptr)
			{
				ColPlayer->AttackPlayer(12);
			}
			
			SmashRatio += DeltaTime*5.f;
			if (SmashRatio >= 1.f)
			{
				SmashRatio = 1.f;
				SmasherMesh->SetRelativeLocation(FMath::Lerp(ReleasePos, SmashPos, SmashRatio));
				FsmComp->ChangeState(Fsm::ReleaseWait);
				return;
			}
			SmasherMesh->SetRelativeLocation(FMath::Lerp(ReleasePos, SmashPos, SmashRatio));
		},

		[this]
		{
			SoundComp->MulticastChangeSound(TEXT("Smasher_Cue"), true, 2000.f);
		}
	);

	FsmComp->CreateState(Fsm::ReleaseWait,
		[this]
		{

		},

		[this](float DeltaTime)
		{
			if (FsmComp->GetStateLiveTime() > 1.f)
			{
				FsmComp->ChangeState(Fsm::Release);
			}
		},

		[this]
		{
		}
	);

	FsmComp->CreateState(Fsm::Release,
		[this]
		{
		},

		[this](float DeltaTime)
		{
			SmashRatio -= DeltaTime;
			if (SmashRatio <= 0.f)
			{
				SmashRatio = 0.f;
				SmasherMesh->SetRelativeLocation(FMath::Lerp(ReleasePos, SmashPos, SmashRatio));
				FsmComp->ChangeState(Fsm::Smash);
				return;
			}
			SmasherMesh->SetRelativeLocation(FMath::Lerp(ReleasePos, SmashPos, SmashRatio));
		},

		[this]
		{
		}
	);
}

void ASmasher::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherActor->ActorHasTag("Player") ==true)
	{
		APlayerBase* OverlapPlayer = Cast<APlayerBase>(OtherActor);
		ColPlayer = OverlapPlayer;
	}
}

void ASmasher::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && ColPlayer && (OtherActor == ColPlayer))
	{
		ColPlayer = nullptr;
	}
}

// Called every frame
void ASmasher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

