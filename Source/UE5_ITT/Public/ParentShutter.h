// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParentShutter.generated.h"

UCLASS()
class UE5_ITT_API AParentShutter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AParentShutter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACoreShutter> CoreShutterClass;

	TArray<class ACoreShutter*> ArrayCoreShutter;

	float MovingSize = 170.f;

	bool bOpen = false;
};