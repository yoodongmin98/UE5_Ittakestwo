// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pedal.generated.h"

UCLASS()
class UE5_ITT_API APedal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APedal();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	enum class Fsm
	{
		ServerDelay,
		SmashWait,
		Smash,
		ReleaseWait,
		Release,
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class USoundManageComponent* SoundComp = nullptr;
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* SmasherMesh = nullptr;

	UPROPERTY(EditAnywhere)
	class USceneComponent* SceneComp = nullptr;

	class UFsmComponent* FsmComp = nullptr;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PedalCollision = nullptr;

	class APlayerBase* ColPlayer = nullptr;

	float SmashSize = 25.f;
	float SmashRatio =0.f;

	UPROPERTY(EditAnywhere)
	FRotator ReleaseRot = FRotator(-30.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere)
	FRotator SmashRot = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	class AEnemyMoonBaboon* MoonBaboon = nullptr;

	float CurAnimFrame = 0.f;
	void SetupFsm();

	bool bStart = false;

	float AnimMaxTime = 2.5f;
	float AnimMaxFrame = 75.f;
	float Anim1FrameTime = AnimMaxTime / AnimMaxFrame;

	float ServerDelayTime = 3.f;

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_ChangeAnim(const FString& strPath, bool bLoop);
	
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
