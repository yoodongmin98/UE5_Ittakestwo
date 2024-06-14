// Fill out your copyright notice in the Description page of Project Settings.


#include "Floor.h"
#include <Engine/StaticMeshActor.h>
#include "Pillar.h"
#include "FsmComponent.h"
#include "Laser.h"
#include "SoundManageComponent.h"
#include "GameManager.h"

// Sets default values
AFloor::AFloor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(FName("Floor"));

	// 네트워크 권한을 확인하는 코드
	if (true == HasAuthority())
	{
		// 서버와 클라이언트 모두에서 변경사항을 적용할 도록 하는 코드입니다.
		bReplicates = true;
		SetReplicateMovement(true);
		SetupFsm();

		SoundComp = CreateDefaultSubobject<USoundManageComponent>(TEXT("SoundComp"));
		RootComponent = SoundComp;
	}
}

// Called when the game starts or when spawned
void AFloor::BeginPlay()
{
	Super::BeginPlay();

	// 네트워크 권한을 확인하는 코드
	if (true == HasAuthority())
	{
		FsmComp->ChangeState(Fsm::PlayerWait);

		MainLaser->SetLaserMaxSize(6000.f);
		ArraySubLaser.Push(SubLaser0);
		SubLaser0->SetRotateSpeed(12.f);
		ArraySubLaser.Push(SubLaser1);
		SubLaser1->SetRotateSpeed(9.f);
		ArraySubLaser.Push(SubLaser2);
		SubLaser2->SetRotateSpeed(6.f);
		for (size_t i = 0; i < ArraySubLaser.Num(); i++)
		{
			ArraySubLaser[i]->SetActorHiddenInGame(true);
		}

	}
}

// Called every frame
void AFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFloor::SetPhase(Fsm Phase)
{	
	FsmComp->ChangeState(Phase);	
}

int32 AFloor::GetCurrentPhase() const
{
	if (nullptr == FsmComp)
	{
		return -1;
	}

	return FsmComp->GetCurrentState();
}

void AFloor::SetupFsm()
{
	FsmComp = CreateDefaultSubobject<UFsmComponent>(TEXT("FsmComp"));

	FsmComp->CreateState(Fsm::PlayerWait,
		[this]
		{
		},

		[this](float DT)
		{

			if (true == Cast<UGameManager>(GetWorld()->GetGameInstance())->IsGameStart())
			{
				FsmComp->ChangeState(Fsm::Phase1_1);
			}
		},

		[this]
		{
		}
	);

	FsmComp->CreateState(Fsm::Phase1_1,
		[this]
		{
			Pillar1->ShutterOpen();
		},

		[this](float DT)
		{

			if (true == Pillar1->IsDone())
			{
				FsmComp->ChangeState(Fsm::Phase1_1Attack);
			}
		},

		[this]
		{
		}
	);
	FsmComp->CreateState(Fsm::Phase1_1Attack,
		[this]
		{
			SoundComp->MulticastPlaySoundDirect(TEXT("LiftStart_Cue"));
			SoundComp->MulticastChangeSound(TEXT("Liftloop_Cue"));
			MoveRatio = 0.f;
			CurPos = GetActorLocation();
			NextPos = CurPos;
			NextPos.Z += MoveSize;
			MainLaser->SetAttack(true);
		},

		[this](float DT)
		{

			MoveRatio += DT / MoveTime;
			if (MoveRatio >= 1.f)
			{
				MoveRatio = 1.f;
				MainLaser->SetAttack(false);
				FsmComp->ChangeState(Fsm::Phase1_2);
			}

			SetActorLocation(FMath::Lerp(CurPos, NextPos, MoveRatio));
		},

		[this]
		{
			SoundComp->MulticastChangeSound(TEXT("Liftstop_Cue"));
		}
	);

	FsmComp->CreateState(Fsm::Phase1_2,
		[this]
		{
			Pillar2->ShutterOpen();
		},

		[this](float DT)
		{
			if (true == Pillar2->IsDone())
			{
				FsmComp->ChangeState(Fsm::Phase1_2Attack);
			}
		},

		[this]
		{
		}
	);

	FsmComp->CreateState(Fsm::Phase1_2Attack,
		[this]
		{
			SoundComp->MulticastPlaySoundDirect(TEXT("LiftStart_Cue"));
			SoundComp->MulticastChangeSound(TEXT("Liftloop_Cue"));
			MoveRatio = 0.f;
			CurPos = GetActorLocation();
			NextPos = CurPos;
			NextPos.Z += MoveSize;
			MainLaser->SetAttack(true);
		},

		[this](float DT)
		{	

			MoveRatio += DT / MoveTime;
			if (MoveRatio >= 1.f)
			{
				MoveRatio = 1.f;
				MainLaser->SetAttack(false);
				FsmComp->ChangeState(Fsm::Phase1_3);
			}

			SetActorLocation(FMath::Lerp(CurPos, NextPos, MoveRatio));
		},

		[this]
		{
			SoundComp->MulticastChangeSound(TEXT("Liftstop_Cue"));
		}
	);

	FsmComp->CreateState(Fsm::Phase1_3,
		[this]
		{
			Pillar0->ShutterOpen();
			Pillar0->SetLongPillar();
			MoveRatio = 0.f;
			CurPos = GetActorLocation();
			NextPos = CurPos;
			NextPos.Z += MoveSize;
		},

		[this](float DT)
		{
			if (true == Pillar0->IsDone())
			{
				MoveRatio += DT / MoveTime;
				if (MoveRatio >= 1.f)
				{
					MoveRatio = 1.f;
					FsmComp->ChangeState(Fsm::Phase1End);
				}
			}
		},

		[this]
		{
		}
	);
		

	FsmComp->CreateState(Fsm::Phase1End,
		[this]
		{
		},

		[this](float DT)
		{
			if (true == bPhase2)
			{
				FsmComp->ChangeState(Fsm::Phase2);
			}
		},

		[this]
		{
		}
	);

	FsmComp->CreateState(Fsm::Phase2,
		[this]
		{
			CloseDoor();
		},

		[this](float DT)
		{
			if (true == bPhase3)
			{
				FsmComp->ChangeState(Fsm::Phase3);
			}
		},

		[this]
		{
		}
	);

	FsmComp->CreateState(Fsm::Phase3,
		[this]
		{
			SoundComp->MulticastPlaySoundDirect(TEXT("LiftStart_Cue"));
			SoundComp->MulticastChangeSound(TEXT("Liftloop_Cue"));
			MoveRatio = 0.f;
			CurPos = GetActorLocation();
			NextPos = CurPos;
			NextPos.Z += MoveSize;
			MainLaser->SetAttack(true);
		},

		[this](float DT)
		{
			MoveRatio += DT / MoveTimeHalf;
			if (MoveRatio >= 1.f)
			{
				MoveRatio = 1.f;
				FsmComp->ChangeState(Fsm::KeepPhase);
			}

			SetActorLocation(FMath::Lerp(CurPos, NextPos, MoveRatio));
		},

		[this]
		{
			SoundComp->MulticastChangeSound(TEXT("Liftstop_Cue"));
		}
	);

	FsmComp->CreateState(Fsm::KeepPhase,
		[this]
		{
			CheckTime = SubLaserUpTime;
		},

		[this](float DT)
		{
			if (SubLaserIndex == ArraySubLaser.Num())
			{
				FsmComp->ChangeState(Fsm::End);
				return;
			}
			CheckTime -= DT;
			if (CheckTime <=0.f)
			{
				ArraySubLaser[SubLaserIndex]->SetActorHiddenInGame(false);
				ArraySubLaser[SubLaserIndex]->SetAttack(true);
				CheckTime = SubLaserUpTime;
				++SubLaserIndex;
			}
		},

		[this]
		{
		}
	);

	FsmComp->CreateState(Fsm::End,
		[this]
		{
		},

		[this](float DT)
		{
		},

		[this]
		{
		}
	);
}

void AFloor::CloseDoor()
{
	FVector TempPos = FVector::ZeroVector;
	TempPos.Y += DoorCloseSize;
	FTransform LeftTransform;
	LeftTransform.SetLocation(TempPos);

	FTransform RightTransform;
	RightTransform.SetLocation(-TempPos);

	LeftDoor->AddActorLocalTransform(LeftTransform);

	RightDoor->AddActorLocalTransform(RightTransform);
}
