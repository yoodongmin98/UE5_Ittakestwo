// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "InteractionUIComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_ITT_API UInteractionUIComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionUIComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, Category = "UI")
    bool bOnlyMay;

    UPROPERTY(EditAnywhere, Category = "UI")
    bool bOnlyCody;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> NearWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> FarWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    float DistanceThreshold;


private:



    UPROPERTY()
    UWidgetComponent* NearWidgetComponent;

    UPROPERTY()
    UWidgetComponent* FarWidgetComponent;

    void InitComponent();


    void SetVisibilityBasedOnDistance();


    void FindTargetActor();
    class APlayerBase* TargetActor;

};
