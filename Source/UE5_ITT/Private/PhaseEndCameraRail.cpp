// Fill out your copyright notice in the Description page of Project Settings.


#include "PhaseEndCameraRail.h"
#include "Kismet/GameplayStatics.h"
#include "FsmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "PlayerBase.h"
#include "EnemyFlyingSaucer.h"
#include "Net/UnrealNetwork.h"

APhaseEndCameraRail::APhaseEndCameraRail(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	if (true == HasAuthority())
	{
		bReplicates = true;
		SetReplicateMovement(true);

		CamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CamComp"));
		CamComp->SetupAttachment(RailCameraMount);

		FsmComp = CreateDefaultSubobject<UFsmComponent>(TEXT("FsmComp"));
		SetupFsmState();
	}
}

bool APhaseEndCameraRail::ShouldTickIfViewportsOnly() const
{
	return false;
}

void APhaseEndCameraRail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APhaseEndCameraRail::MulticastTickCameraMove_Implementation(float DeltaTime)
{
	if (true == bIsMoveCheck)
	{
		EndTime -= DeltaTime;
		if (0.0f >= EndTime)
		{
			bIsMoveEnd = true;
			return;
		}
	}

	if (false == bIsMoveCheck)
	{
		if (1.0f <= CurrentPositionOnRail)
		{
			bIsMoveCheck = true;
		}
	}
	CurrentPositionOnRail += DeltaTime * CameraMoveRatio;
	CamComp->SetWorldLocation(GetRailSplineComponent()->GetLocationAtTime(CurrentPositionOnRail, ESplineCoordinateSpace::World));
}
	
void APhaseEndCameraRail::EnableCameraMove(const float MoveRatio /*= 0.25f*/)
{
	if (0.0f >= MoveRatio)
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera movement ratio has not been set"));
		return;
	}

	CameraMoveRatio = MoveRatio;
	bIsMoveStart = true;
}

void APhaseEndCameraRail::BeginPlay()
{
	Super::BeginPlay();

	if (true == HasAuthority())
	{
		FsmComp->ChangeState(Fsm::Wait);
	}
}

void APhaseEndCameraRail::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APhaseEndCameraRail, CamComp);
	DOREPLIFETIME(APhaseEndCameraRail, CameraMoveRatio);
}


void APhaseEndCameraRail::SetupFsmState()
{
	FsmComp->CreateState(Fsm::Wait,
		[this]
		{

		},

		[this](float DT)
		{
			if (true == bIsMoveStart)
			{
				FsmComp->ChangeState(Fsm::Move);
				return;
			}
		},

		[this]
		{
			
		});

	FsmComp->CreateState(Fsm::Move,
		[this]
		{
			SetupActorsRef();
			CurrentPositionOnRail = 0.0f;
		},

		[this](float DT)
		{
			MulticastTickCameraMove(DT);
		},

		[this]
		{
		});

	FsmComp->CreateState(Fsm::End,
		[this]
		{
		},

		[this](float DT)
		{
		},

		[this]
		{
		});
}

void APhaseEndCameraRail::SetupActorsRef()
{
	AActor* BossPtr = UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyFlyingSaucer::StaticClass());
	if (nullptr != BossPtr)
	{
		EnemyFlyingSaucer = Cast<AEnemyFlyingSaucer>(BossPtr);
	}
}
