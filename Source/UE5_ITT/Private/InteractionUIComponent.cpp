// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionUIComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/WidgetComponent.h"

// Sets default values for this component's properties
UInteractionUIComponent::UInteractionUIComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    DistanceThreshold = 1000.f; // Default distance threshold

    // Create widget components
    NearWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NearWidgetComponent"));
    FarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("FarWidgetComponent"));

    bOnlyMay = false;
    bOnlyCody = false;
}


// Called when the game starts
void UInteractionUIComponent::BeginPlay()
{
	Super::BeginPlay();
    InitComponent();
}

void UInteractionUIComponent::InitComponent()
{
    // Attach widget components to this scene component
    NearWidgetComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    FarWidgetComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

    // Set widget classes
    if (NearWidgetClass)
    {
        NearWidgetComponent->SetWidgetClass(NearWidgetClass);
    }
    if (FarWidgetClass)
    {
        FarWidgetComponent->SetWidgetClass(FarWidgetClass);
    }

    // Set widget space to screen
    NearWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    FarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    // Set widget size
    FVector WidgetScale = FVector(0.5f, 0.5f, 0.5f); // Example scale (adjust as needed)
    NearWidgetComponent->SetWorldScale3D(WidgetScale);
    FarWidgetComponent->SetWorldScale3D(WidgetScale);
    // Initially hide far widget
    //NearWidgetComponent->SetVisibility(false);
    //FarWidgetComponent->SetVisibility(false);


}


void UInteractionUIComponent::SetVisibilityBasedOnDistance()
{
    if (!TargetActor)
    {
        return;
    }
    FVector PlayerLocation = TargetActor->GetActorLocation();
    FVector SceneComponentLocation = GetComponentLocation();
    float Distance = FVector::Distance(PlayerLocation, SceneComponentLocation);

    // Toggle widget visibility based on distance threshold
    if (Distance < DistanceThreshold)
    {
        // Player is near, show near widget and hide far widget
        NearWidgetComponent->SetVisibility(true);
        FarWidgetComponent->SetVisibility(false);
    }
    else
    {
        // Player is far, hide near widget and show far widget
        NearWidgetComponent->SetVisibility(false);
        FarWidgetComponent->SetVisibility(true);
    }
}



// Called every frame
void UInteractionUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!TargetActor)
    {
        FindTargetActor();
        return;
    }
    // 거리에 따라 설정
    SetVisibilityBasedOnDistance();


    if(bOnlyCody)
    {
        FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
        It++;
        if (!It)
        {
            return;
        }
        It->Get();
        FVector TargetActorLocation = TargetActor->GetActorLocation();

        // 액터의 월드 위치를 스크린 좌표로 변환하여 위치 확인
        FVector2D ScreenPosition;
        if (UGameplayStatics::ProjectWorldToScreen(It->Get(), GetComponentLocation(), ScreenPosition))
        {
            // 스크린 좌표가 뷰포트의 범위 내에 있는지 확인
            int32 ViewportSizex, ViewportSizey;
            It->Get()->GetViewportSize(ViewportSizex, ViewportSizey);

            if (ScreenPosition.X >= ViewportSizex / 2 && ScreenPosition.X <= ViewportSizex &&
                ScreenPosition.Y >= 0 && ScreenPosition.Y <= ViewportSizey)
            {
                // 액터가 스크린에 있는 경우
                UE_LOG(LogTemp, Warning, TEXT("Actor %s is on screen at (%f, %f)"), *TargetActor->GetName(), ScreenPosition.X, ScreenPosition.Y);
            

                NearWidgetComponent->GetWidget()->SetPositionInViewport(ScreenPosition);
                FarWidgetComponent->GetWidget()->SetPositionInViewport(ScreenPosition);
                NearWidgetComponent->SetVisibility(true);
                FarWidgetComponent->SetVisibility(true);
            }
            else
            {
                // 액터가 스크린에 없는 경우
                NearWidgetComponent->SetVisibility(false);
                FarWidgetComponent->SetVisibility(false);
                UE_LOG(LogTemp, Warning, TEXT("Actor %s is not on screen"), *TargetActor->GetName());
            }
        }
    }
}

void UInteractionUIComponent::FindTargetActor()
{
	TArray<AActor*> AllPlayerActors;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerBase::StaticClass(), AllPlayerActors);

	APlayerBase* me = Cast<APlayerBase>(GetOwner());
	// PlayerActors를 순회하며 각 플레이어의 처리 수행
    for (AActor* Player : AllPlayerActors)
    {
        if (me != Cast<APlayerBase>(Player))
        {
            APlayerBase* NextPlayer = Cast<APlayerBase>(Player);
            const TArray<FName>& CheckTag = NextPlayer->Tags;
            for (const FName& V : CheckTag)
            {
                if (V == FName("Cody") || V == FName("May"))
                {
                    if (true == bOnlyCody && V == FName("Cody"))
                    {
                        TargetActor = NextPlayer;
                        return;

                    }
                    else if (true == bOnlyMay && V == FName("May"))
                    {
                        TargetActor = NextPlayer;
                        return;
                    }
                }
            }
        }
    }
		// Player를 사용하여 플레이어 처리
}