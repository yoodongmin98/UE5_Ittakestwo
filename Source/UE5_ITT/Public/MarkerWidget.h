// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PlayerBase.h"
#include "MarkerWidget.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class UE5_ITT_API UMarkerWidget : public UWidgetComponent
{
	GENERATED_BODY()
public:
    // Constructor
    UMarkerWidget(const FObjectInitializer& ObjectInitializer);

    // Æ½ ÇÔ¼ö ¼±¾ð
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // The widget to display
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> UIWidgetClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    float DistanceThreshold;



protected:

    // The widget instance 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UUserWidget* MarkerInstance;

private:


    
    // check is cody or may
    bool bChecked;
    bool bIsCody;

    // Dist between target & owner
    float Distance;

    // viewportsize
    FVector2D StartView;
    int32 ViewportSizex, ViewportSizey;

    // MayView -> Cody Target, Cody View -> MayTarget
    class APlayerBase* TargetActor;

    APlayerController* MyViewController;

    FDateTime SpawnTime;

    bool bAllSettingDone;

    void SetIsCody();

    void FindTargetActor();

    void FindMyViewController();

    void SettingWidget();

    void RenderWidgetPosition();
};
