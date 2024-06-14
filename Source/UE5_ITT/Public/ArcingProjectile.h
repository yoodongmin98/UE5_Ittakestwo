// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcingProjectile.generated.h"

UCLASS()
class UE5_ITT_API AArcingProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArcingProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SetupProjectileMovementComponent();

	UFUNCTION()
	void SetupTargetLocation(FVector Location) { TargetLocation = Location; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// overlap event
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditDefaultsOnly)
	bool bIsOverlapEvent = false;

	UFUNCTION()
	void SetupOverlapEvent();

	UFUNCTION()
	void SetupComponent();

	UPROPERTY(EditDefaultsOnly)
	FVector TargetLocation = FVector::ZeroVector;

	// TSubclass
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ABurstEffect> BurstEffectClass;

	// Component
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USceneComponent* SceneComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class UStaticMeshComponent* ProjectileMeshComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class UProjectileMovementComponent* ProjectileMovementComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class UNiagaraComponent* TrailEffectComp = nullptr;
};
