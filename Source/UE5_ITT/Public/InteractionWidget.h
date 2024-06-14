// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class UE5_ITT_API UInteractionWidget : public UWidgetComponent
{
	GENERATED_BODY()
public:

    // Constructor
    UInteractionWidget(const FObjectInitializer& ObjectInitializer);

    // Æ½ ÇÔ¼ö ¼±¾ð
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bOnlyMay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bOnlyCody;

    UFUNCTION(BlueprintCallable)
    inline float GetDistanceFromTarget() const
    {
        return Distance;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    // The widget instance 
    UUserWidget* WidgetInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bIsViewWiget;

protected:


    // The widget to display
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction Widget")
    TSubclassOf<UUserWidget> UIWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    float DistanceThreshold;

    

private:

    void FindTargetActor();
    class APlayerBase* TargetActor;

    float Distance;

    void SetCodyWidget(const FVector2D _Pos, const bool isvisible);

    APlayerController* CodyViewController;

    int32 ViewportSizex, ViewportSizey;

    FDateTime SpawnTime;


};
