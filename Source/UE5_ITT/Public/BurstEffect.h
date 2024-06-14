// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BurstEffect.generated.h"

UCLASS()
class UE5_ITT_API ABurstEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABurstEffect();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	class USoundManageComponent* SoundComp = nullptr;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	

	void SetupComponent();
	void SetupDestroyTimerEvent();
	void EffectDestroy();
	void TickIncreaseScale(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USceneComponent* SceneComp = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class UStaticMeshComponent* StaticMeshComp = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float MaxScale = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float BurstSpeed = 50.0f;

	UPROPERTY(EditAnywhere)
	bool bDestroyValue = false;

	UPROPERTY(EditAnywhere)
	bool bIsPlayerOverlap = false;

	UPROPERTY(EditDefaultsOnly)
	class APlayerBase* OverlapPlayer = nullptr;

	UPROPERTY(EditAnywhere)
	int32 DamageToPlayer = 1;;
	

};
