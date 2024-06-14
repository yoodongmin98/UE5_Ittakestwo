// Fill out your copyright notice in the Description page of Project Settings.


#include "MarkerWidget.h"
#include "Kismet/GameplayStatics.h"

UMarkerWidget::UMarkerWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Default initialization

	

    bChecked = false;
    bAllSettingDone = false;

    DistanceThreshold = 4000.0f; // Default distance threshold
    Distance = 0.0f;

    ViewportSizex = 0;
    ViewportSizey = 0;

    SpawnTime = FDateTime::Now();
}

void UMarkerWidget::SetIsCody()
{
    if(GetOwner())
    {
        const TArray<FName>& CheckTag = GetOwner()->Tags;
        for (const FName& V : CheckTag)
        {
            if (V == FName("Cody") || V == FName("May"))
            {
                if (V == FName("Cody"))
                {
                    bIsCody = true;
                    bChecked = true;
                }
                else if(V == FName("May"))
                {
                    bIsCody = false;
                    bChecked = true;
                }
                return;
            }
        }
    }
}

void UMarkerWidget::FindMyViewController()
{
    FDateTime CurTime = FDateTime::Now();
    if ((CurTime - SpawnTime).GetTotalSeconds() < 3.0f)
    {
        return;
    }
    FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
    if (true == bIsCody)
    {
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
    }

    if (!It)
    {
        return;
    }
    if (It->Get())
    {
        // projection 계산을 할 controller찾기
        MyViewController = It->Get();
        MyViewController->GetViewportSize(ViewportSizex, ViewportSizey);
        ViewportSizex /= 2;
        if (true == bIsCody)
        {
            StartView.X = ViewportSizex;
        }
        else
        {
            StartView.X = 0.0f;
        }
		StartView.Y = 0.0f;
    }
}


void UMarkerWidget::SettingWidget()
{
    if (false == bChecked)
    {
        SetIsCody();
        return;
    }
    if (!TargetActor)
    {
        FindTargetActor();
        return;
    }
    if (!MyViewController)
    {
        FindMyViewController();
        return;
    }
    if (!MarkerInstance)
    {
        MarkerInstance = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		MarkerInstance->AddToViewport();
        MarkerInstance->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    bAllSettingDone = true;
}

void UMarkerWidget::RenderWidgetPosition()
{

    FVector2D ActorScreenPosition;
    FVector TargetLocation = TargetActor->GetActorLocation();
    float NextRotation = 0.0f;

    if (!MyViewController->ProjectWorldLocationToScreen(TargetLocation, ActorScreenPosition))
    {
        return; // Failed to project actor's location to screen space
    }
    if ((ActorScreenPosition.X >= StartView.X && ActorScreenPosition.X <= StartView.X + ViewportSizex &&
        ActorScreenPosition.Y >= StartView.Y && ActorScreenPosition.Y <= StartView.Y + ViewportSizey)) // 화면 내에 존재
    {
        Distance = FVector::Distance(GetOwner()->GetActorLocation(), TargetLocation);

        if (DistanceThreshold < Distance) // 일정 거리 이상인 경우
        {
            MarkerInstance->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            MarkerInstance->SetVisibility(ESlateVisibility::Hidden);
            return;
        }
        NextRotation = 0.0f;
    }
    else // 화면 밖에 존재
    {
        MarkerInstance->SetVisibility(ESlateVisibility::Visible);
        if (ActorScreenPosition.X < StartView.X)
        {
            ActorScreenPosition.X = StartView.X + 30.0f;
            NextRotation = 90.0f;
        }
        else if (ActorScreenPosition.X > StartView.X + ViewportSizex)
        {
            ActorScreenPosition.X = StartView.X + ViewportSizex - 30.0f;
            NextRotation = -90.0f;
        }

        if (ActorScreenPosition.Y < StartView.Y)
        {
            ActorScreenPosition.Y = StartView.Y + 60.0f;
            if (NextRotation > 0.0f)
            {
                NextRotation = 135.0f;
            }
            else if (NextRotation < 0.0f)
            {
                NextRotation = -135.0f;
            }
            else
            {
                NextRotation = 180.0f;
            }
        }
        else if (ActorScreenPosition.Y > StartView.Y + ViewportSizey)
        {
            ActorScreenPosition.Y = StartView.Y + ViewportSizey - 10.0;
            if (NextRotation > 0.0f)
            {
                NextRotation = 45.0f;
            }
            else if (NextRotation < 0.0f)
            {
                NextRotation = -45.0f;
            }
        }

    }
    // 위치 보정
    ActorScreenPosition.X -= 23.0f;
    ActorScreenPosition.Y -= 60.0f;

    // 새로운 변환행렬
    const FWidgetTransform NextTransf = FWidgetTransform(FVector2D::ZeroVector, FVector2D::UnitVector, FVector2D::ZeroVector, NextRotation);
    MarkerInstance->SetRenderTransform(NextTransf);
    MarkerInstance->SetPositionInViewport(ActorScreenPosition);
}

void UMarkerWidget::FindTargetActor()
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
                if (false == bIsCody && V == FName("Cody")) // 본인이 May라면 target은 Cody
                {
                    TargetActor = NextPlayer;
                    return;
                }
                else if (true == bIsCody && V == FName("May")) // 본인이 Cody라면 Target은 May
                {
                    TargetActor = NextPlayer;
                    return;
                }
            }
        }
    }
}

// 틱 함수 구현
void UMarkerWidget::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (false == bAllSettingDone)
    {
        SettingWidget();
        return;
    }
    RenderWidgetPosition();
}
