// Fill out your copyright notice in the Description page of Project Settings.


#include "ParentCameraRail.h"
#include "FsmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "Misc/OutputDeviceNull.h"

AParentCameraRail::AParentCameraRail(const FObjectInitializer& ObjectInitializer)
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
	}
}

bool AParentCameraRail::ShouldTickIfViewportsOnly() const
{
	return false;
}

void AParentCameraRail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AParentCameraRail::MulticastMoveRailCamera_Implementation(float RailRatio)
{
	CurrentPositionOnRail += RailRatio;
	CurrentPositionOnRail = FMath::Clamp(CurrentPositionOnRail, 0.f, 1.f);
	CamComp->SetWorldLocation(GetRailSplineComponent()->GetLocationAtTime(CurrentPositionOnRail, ESplineCoordinateSpace::World));
}

void AParentCameraRail::MulticastSetCameraView_Implementation(float BlendTime)
{
	FConstPlayerControllerIterator PlayerControllerIter = GetWorld()->GetPlayerControllerIterator();
	if (HasAuthority() == true)
	{
		++++PlayerControllerIter;
	}
	else
	{
		++PlayerControllerIter;
	}

	PrevViewTarget = PlayerControllerIter->Get()->GetViewTarget();
	PlayerControllerIter->Get()->SetViewTargetWithBlend(this, BlendTime);
	bStart = true;
}

void AParentCameraRail::ResetScreenView()
{
	
	FOutputDeviceNull OutputDevice;
	FString FunctionString = TEXT("ResetScreenMode");

	CallFunctionByNameWithArguments(*FunctionString, OutputDevice, nullptr, true);
}

bool AParentCameraRail::IsSupportedForNetworking() const
{
	return true;
}

void AParentCameraRail::MulticastCameraViewReset_Implementation(float BlendTime)
{
	FConstPlayerControllerIterator PlayerControllerIter = GetWorld()->GetPlayerControllerIterator();
	if (HasAuthority() == true)
	{
		++++PlayerControllerIter;
	}
	else
	{
		++PlayerControllerIter;
	}

	PlayerControllerIter->Get()->SetViewTargetWithBlend(PrevViewTarget, BlendTime);
	PrevViewTarget = nullptr;
}

void AParentCameraRail::BeginPlay()
{
	Super::BeginPlay();
}
