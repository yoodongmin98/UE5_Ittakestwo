// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Floor.generated.h"

UCLASS()
class UE5_ITT_API AFloor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's propertiesS
	AFloor();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	enum class Fsm
	{
		PlayerWait,
		Phase1_1,
		Phase1_1Attack,
		Phase1_2,
		Phase1_2Attack,
		Phase1_3,
		Phase1End,
		Phase2,
		Phase3,
		KeepPhase,
		End,
	};
	void SetPhase(Fsm Phase);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentPhase() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class USoundManageComponent* SoundComp = nullptr;

	float MoveTime = 20.f;
	float MoveSize = 4995.f;
	float MoveRatio = 0.f;
	float PillarSecond = 0.f;
	float MoveTimeHalf = 10.f;
	float DoorCloseSize = 1100.f;

	FVector CurPos = FVector::Zero();
	FVector NextPos = FVector::Zero();

	UPROPERTY(EditAnywhere, Category = "Pointer")
	class APillar* Pillar0 = nullptr;
	UPROPERTY(EditAnywhere, Category = "Pointer")
	class APillar* Pillar1 = nullptr;
	UPROPERTY(EditAnywhere, Category = "Pointer")
	class APillar* Pillar2 = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pointer")
	class AStaticMeshActor* LeftDoor = nullptr;
	UPROPERTY(EditAnywhere, Category = "Pointer")
	class AStaticMeshActor* RightDoor = nullptr;


	UPROPERTY(EditAnywhere, Category = "Pointer")
	class ALaser* MainLaser = nullptr;


	UPROPERTY(EditAnywhere, Category = "Pointer")
	class ALaser* SubLaser0 = nullptr;
	UPROPERTY(EditAnywhere, Category = "Pointer")
	class ALaser* SubLaser1 = nullptr;
	UPROPERTY(EditAnywhere, Category = "Pointer")
	class ALaser* SubLaser2 = nullptr;
	TArray<ALaser*> ArraySubLaser;
	int SubLaserIndex = 0;

	float SubLaserUpTime = 12.f;
	float CheckTime = -1.f;

	class UFsmComponent* FsmComp = nullptr;

	bool bPhase2 = false;
	bool bPhase3 = false;

	void SetupFsm();
	void CloseDoor();
};
