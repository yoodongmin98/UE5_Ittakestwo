// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pillar.generated.h"

UCLASS()
class UE5_ITT_API APillar : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APillar();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	enum class Fsm
	{
		Close,
		ShutterOpen,
		WaitMove,
		Wait,
		MoveUp,
		WaitBoom,
		MoveDown,
		Boom,
		ShutterClose,
		Done,
	};

	bool IsDone()
	{
		return bDone;
	}

	void ShutterOpen()
	{
		bShutterOpen = true;
	}

	void SetLongPillar()
	{
		MoveSize = 1620.f;
	}


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class USoundManageComponent* SoundComp = nullptr;
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PillarMesh = nullptr;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* ShieldMesh = nullptr;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* ButtonMesh = nullptr;

	UPROPERTY(EditAnywhere)
	class AEnergyCore* EnergyCoreActor = nullptr;


	UPROPERTY(EditAnywhere, Category = "Bool")
	bool bShutterOpen = false;
	UPROPERTY(EditAnywhere, Category = "Bool")
	bool bPlayerWait = false;
	UPROPERTY(EditAnywhere, Category = "Bool")
	bool bOnPlayer = false;

	UPROPERTY(EditAnywhere, Category = "Bool")
	bool bShieldOpen = false;
	UPROPERTY(EditAnywhere, Category = "Bool")
	bool bDone = false;

	FVector DefaultPos = FVector::Zero();
	float PlayerWaitRatio = 0.f;
	float PlayerWaitSize = 80.f;
	FVector PlayerWaitPos = FVector::Zero();

	float MoveRatio = 0.f;
	float MoveSize = 750.f;
	FVector MovePos = FVector::Zero();

	float ShieldOpenRatio = 0.f;
	float ShieldOpenSize = 400.f;
	FVector ShieldDefaultPos = FVector::Zero();
	FVector ShieldOpenPos = FVector::Zero();
	
	class UFsmComponent* FsmComp = nullptr;

	UPROPERTY(EditAnywhere)
	class AParentShutter* ParentShutter = nullptr;

	void SetupFsm();

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);




	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MulticastSpawnNiagaraEffect();


	// Explosion Effect
	
	void SpawnNiagaraEffect();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACoreExplosionEffect> ExplosionEffectClass = nullptr;

};
