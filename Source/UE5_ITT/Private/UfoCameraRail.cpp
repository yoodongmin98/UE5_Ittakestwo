// Fill out your copyright notice in the Description page of Project Settings.


#include "UfoCameraRail.h"
#include "FsmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "GameManager.h"
#include "Cody.h"

AUfoCameraRail::AUfoCameraRail(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (true == HasAuthority())
	{
		bReplicates = true;
		SetReplicateMovement(true);

		SetupFsm();
	}
}

void AUfoCameraRail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUfoCameraRail::BeginPlay()
{
	Super::BeginPlay();

	if (true == HasAuthority())
	{
		FsmComp->ChangeState(Fsm::Wait);
		CamComp->AttachToComponent(RailCameraMount, FAttachmentTransformRules::KeepRelativeTransform);
		Cast<UGameManager>(GetGameInstance())->AddCameraRigRail(TEXT("UfoRoad"), this);
	}
}

void AUfoCameraRail::SetupFsm()
{
	FsmComp->CreateState(Fsm::Wait,
		[this]
		{
			//ÇöÀç´Â ¾È¾¸
			Destroy();
		},

		[this](float DT)
		{
			if (bStart)
			{
				FsmComp->ChangeState(Fsm::Move);
			}
		},

		[this]
		{
		});

	FsmComp->CreateState(Fsm::Move,
		[this]
		{
			//Cody = Cast<UGameManager>(GetGameInstance())->GetCody();
		},

		[this](float DT)
		{
			if (nullptr == Cody)
			{
				UE_LOG(LogTemp, Display, TEXT("null"));
				return;
			}
			double Dist = FVector::Distance(Cody->GetActorLocation(), CamComp->GetComponentLocation());
			float Ratio = 50.f - Dist; 
			Ratio *= 0.001f;
			
			MulticastMoveRailCamera(DT *-Ratio);		
		},

		[this]
		{
		});
}
