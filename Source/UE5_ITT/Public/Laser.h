// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Laser.generated.h"

UCLASS()
class ALaser : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ALaser();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	enum class Fsm
	{
		Wait,
		MoveUp,
		LaserOn,
		Attack,
		LaserOff,
		MoveDown,
	};

	void SetAttack(bool bValue)
	{
		bAttackStart = bValue;
	}

	UFUNCTION(BlueprintImplementableEvent)
	void SetLaserSize(float SizeParam);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetActiveLaser(bool bValue);
	UFUNCTION(NetMulticast, Reliable)

	void MulticastActiveLaser(bool bValue);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetLaserSize(float SizeParam);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pointer")
	class UStaticMeshComponent* LaserMesh = nullptr;

	/// <summary>
	/// rotatespeed/s
	/// </summary>
	/// <param name="SpeedParam"></param>
	void SetRotateSpeed(float SpeedParam)
	{
		RotateSpeed = SpeedParam;
	}
	
	void SetLaserMaxSize(float SizeParam)
	{
		LaserMaxSize = SizeParam;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class USoundManageComponent* SoundComp = nullptr;
	bool bAttackStart = false;
	float RotateSpeed = 30.f;

	float LaserIncreaseTime = 3.f;
	float LaserSizeRatio = 0.f;
	float LaserMaxSize = 11000.f;

	FVector DefaultPos = FVector::Zero();
	FVector AttackPos = FVector::Zero();
	float AttackMoveSize = 500.f;

	float MovingRatio = 0.f;

	class UFsmComponent* FsmComp = nullptr;

	void SetupFsm();
};
