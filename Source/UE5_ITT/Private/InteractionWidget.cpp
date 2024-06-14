// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionWidget.h"
#include "PlayerBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UInteractionWidget::UInteractionWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Default initialization
    WidgetInstance = nullptr;
    //NearWidgetInstance = nullptr;
    //FarWidgetInstance = nullptr;
    DistanceThreshold = 1000.f; // Default distance threshold
    Distance = 1500.0f;

    bOnlyMay = false;
    bOnlyCody = false;
    ViewportSizex = 0;
    ViewportSizey = 0;

    SpawnTime = FDateTime::Now();
    bIsViewWiget = false;
}


// 틱 함수 구현
void UInteractionWidget::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!TargetActor)
    {
        FindTargetActor();
        return;
    }

    FVector PlayerLocation = TargetActor->GetActorLocation();
    FVector SceneComponentLocation = GetComponentLocation();
    Distance = FVector::Distance(PlayerLocation, SceneComponentLocation);
    if (!WidgetInstance)
    {
        if(true == bOnlyMay)
        {
            WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
            SetWidget(WidgetInstance);
        }
        else if (true == bOnlyCody)
        {
            WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
            WidgetInstance->AddToViewport();
            WidgetInstance->SetVisibility(ESlateVisibility::Hidden);
        }


        return;
    } 

    
    if (bOnlyCody && WidgetInstance)
    {

        if (false == bIsViewWiget)
        {
            return;
        }

        if(!CodyViewController)
        {
            FDateTime CurTime = FDateTime::Now();
            if ((CurTime - SpawnTime).GetTotalSeconds() < 10)
            {
                return;
            }

            FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
            int32 NumofPlayer = GetWorld()->GetNumPlayerControllers();
            if (NumofPlayer == 2)
            {
                It++;
            }
            else if (NumofPlayer == 4)
            {
                It++;
                It++;
            }
            else
            {
                return;
            }


            if (!It)
            {
                return;
            }
            if(It->Get())
            {
                CodyViewController = It->Get();
                CodyViewController->GetViewportSize(ViewportSizex, ViewportSizey);
            }
        }
		FVector TargetActorLocation = TargetActor->GetActorLocation();

        // 액터의 월드 위치를 스크린 좌표로 변환하여 위치 확인
        FVector2D ScreenPosition;
        if (UGameplayStatics::ProjectWorldToScreen(CodyViewController, GetComponentLocation(), ScreenPosition))
        {
            ScreenPosition.X -= 50.0f;
            ScreenPosition.Y -= 50.0f;
            // 스크린 좌표가 뷰포트의 범위 내에 있는지 확인

            if (ScreenPosition.X >= ViewportSizex / 2 && ScreenPosition.X <= ViewportSizex &&
                ScreenPosition.Y >= 0 && ScreenPosition.Y <= ViewportSizey)
            {
                SetCodyWidget(ScreenPosition, true);
            }
            else
            {
                SetCodyWidget(ScreenPosition, false);
            }
        }
    }

}

void UInteractionWidget::SetCodyWidget(const FVector2D _Pos, const bool isvisible)
{
    if(true == isvisible)
    {
        WidgetInstance->SetPositionInViewport(_Pos);
        WidgetInstance->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        WidgetInstance->SetVisibility(ESlateVisibility::Hidden);
    }
}


void UInteractionWidget::FindTargetActor()
{
    TArray<AActor*> AllPlayerActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerBase::StaticClass(), AllPlayerActors);

    // PlayerActors를 순회하며 각 플레이어의 처리 수행
    for (AActor* Player : AllPlayerActors)
    {
		APlayerBase* NextPlayer = Cast<APlayerBase>(Player);
		const TArray<FName>& CheckTag = NextPlayer->Tags;
		for (const FName& V : CheckTag)
		{
			if (V == FName("Cody") || V == FName("May"))
			{
				if (true == bOnlyMay && V == FName("May"))
				{
					TargetActor = NextPlayer;
					//SetWidget(NearWidgetInstance);
					//SetWidget(FarWidgetInstance);
					return;
				}
				else if (true == bOnlyCody && V == FName("Cody"))
				{
					TargetActor = NextPlayer;
					//SetWidget(NearWidgetInstance);
					//SetWidget(FarWidgetInstance);
					return;
				}
			}
		}
    }
}