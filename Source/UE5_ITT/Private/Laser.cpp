// Fill out your copyright notice in the Description page of Project Settings.


#include "Laser.h"
#include "FsmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SoundManageComponent.h"

// Sets default values
ALaser::ALaser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (true == HasAuthority())
	{
		// 서버와 클라이언트 모두에서 변경사항을 적용할 도록 하는 코드입니다.
		bReplicates = true;
		SetReplicateMovement(true);
		LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
		RootComponent = LaserMesh;
		SoundComp = CreateDefaultSubobject<USoundManageComponent>(TEXT("SoundComp"));

		SetupFsm();
	}


}

// Called when the game starts or when spawned
void ALaser::BeginPlay()
{
	Super::BeginPlay();

	// 네트워크 권한을 확인하는 코드
	if (true == HasAuthority())
	{
		FsmComp->ChangeState(Fsm::Wait);

		DefaultPos = LaserMesh->GetRelativeLocation();
		AttackPos = DefaultPos;
		AttackPos.Z += AttackMoveSize;

		SoundComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void ALaser::SetupFsm()
{
	FsmComp = CreateDefaultSubobject<UFsmComponent>(TEXT("FsmComp"));
	FsmComp->CreateState(Fsm::Wait,
		[this]
		{
			SoundComp->MulticastSetAttenuationDistance(1500.f, 4000.f);
		},

		[this](float DT)
		{
			if (true == bAttackStart)
			{
				FsmComp->ChangeState(Fsm::MoveUp);
			}
		},

		[this]
		{
		});

	FsmComp->CreateState(Fsm::MoveUp,
		[this]
		{
			SoundComp->MulticastChangeSound(TEXT("PowerCoreElevatorUp_Cue"));
		},

		[this](float DT)
		{
			MovingRatio += DT;

			if (MovingRatio >= 1.f)
			{
				MovingRatio = 1.f;
				FsmComp->ChangeState(Fsm::LaserOn);
			}

			SetActorRelativeLocation(FMath::Lerp(DefaultPos, AttackPos, MovingRatio));
		},

		[this]
		{
		});

	FsmComp->CreateState(Fsm::LaserOn,
		[this]
		{
			LaserSizeRatio = 0.f;
			MulticastActiveLaser(true);
			MulticastSetLaserSize(1.f);
			SoundComp->MulticastChangeSound(TEXT("LaserSpinnerLoop_Cue"));
		},

		[this](float DT)
		{
			//레이저 길이 증가하는 코드
			LaserSizeRatio += DT/ LaserIncreaseTime;
			if (LaserIncreaseTime <= FsmComp->GetStateLiveTime())
			{
				LaserSizeRatio = 1.f;
				MulticastSetLaserSize(FMath::Lerp(1, LaserMaxSize, LaserSizeRatio));
				FsmComp->ChangeState(Fsm::Attack);
				return;
			}
			MulticastSetLaserSize(FMath::Lerp(1, LaserMaxSize, LaserSizeRatio));
		},

		[this]
		{
		});

	FsmComp->CreateState(Fsm::Attack,
		[this]
		{
		},

		[this](float DT)
		{
			if (false == bAttackStart)
			{
				FsmComp->ChangeState(Fsm::LaserOff);
			}


			AddActorLocalRotation({ 0.f,RotateSpeed * DT,0.f });
		},

		[this]
		{
		});

	FsmComp->CreateState(Fsm::LaserOff,
		[this]
		{
			LaserSizeRatio = 0.f;
		},

		[this](float DT)
		{
			//레이저 길이 감소하는 코드
			LaserSizeRatio += DT / LaserIncreaseTime;
			if (LaserIncreaseTime <= FsmComp->GetStateLiveTime())
			{
				LaserSizeRatio = 1.f;
				MulticastSetLaserSize(FMath::Lerp(1, LaserMaxSize, 1.f - LaserSizeRatio));
				FsmComp->ChangeState(Fsm::MoveDown);
				return;
			}

			MulticastSetLaserSize(FMath::Lerp(1, LaserMaxSize, 1.f - LaserSizeRatio));
		},

		[this]
		{
			MulticastActiveLaser(false);
		});

	FsmComp->CreateState(Fsm::MoveDown,
		[this]
		{
			SoundComp->MulticastChangeSound(TEXT("PowerCoreElevatorDown_Cue"));
		},

		[this](float DT)
		{
			MovingRatio -= DT;

			if (MovingRatio <= 0.f)
			{
				MovingRatio = 0.f;
				FsmComp->ChangeState(Fsm::Wait);
			}

			SetActorRelativeLocation(FMath::Lerp(DefaultPos, AttackPos, MovingRatio));
		},

		[this]
		{
		});

}

// Called every frame
void ALaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALaser::MulticastActiveLaser_Implementation(bool bValue)
{
	SetActiveLaser(bValue);
}

void ALaser::MulticastSetLaserSize_Implementation(float SizeParam)
{
	SetLaserSize(SizeParam);
}
