// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyFlyingSaucer.generated.h"

UCLASS()
class UE5_ITT_API AEnemyFlyingSaucer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyFlyingSaucer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 보스 전투관련 
	UFUNCTION(BlueprintCallable)
	void FireHomingRocket();

	UFUNCTION(BlueprintCallable)
	void FireArcingProjectile();

	UPROPERTY(BlueprintReadWrite, Replicated)
	float ArcingProjectileFireTime = 3.0f;

	UPROPERTY(BlueprintReadWrite, Replicated)
	float ArcingProjectileMaxFireTime = 3.0f;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_CreateEnergyChargeEffect();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_CreateGroundPoundEffect();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_SetFocusTarget();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_AttachToMoonBaboonActorAndFloor();

	// 레이저 추적 로직 관련 변수
	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector PrevTargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector PrevTargetLocationBuffer = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bPrevTargetLocationValid = false;

	UPROPERTY(BlueprintReadWrite, Replicated)
	float LaserLerpRatio = 0.0f;

	UPROPERTY(BlueprintReadWrite, Replicated)
	float LaserLerpRate = 25.0f;

	UPROPERTY(BlueprintReadWrite, Replicated)
	int32 LaserFireCount = 0;

	UPROPERTY(VisibleDefaultsOnly)
	int32 CurrentArcingProjectileTargetIndex = 0;

	UPROPERTY(VisibleDefaultsOnly)
	class AActor* LaserTargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float CoreExplodeDamage = 11.0f;

	UFUNCTION(BlueprintCallable)
	void SetDamage(const float Damage)
	{
		CurrentHp -= Damage;
		if (CurrentHp <= 0.0f)
		{
			CurrentHp = 0.0f;
		}
	}

	UPROPERTY(BlueprintReadWrite, Replicated)
	int32 PatternDestroyCount = 0;

	UFUNCTION(BlueprintCallable)
	void AddPatternDestoryCount();
	
	UPROPERTY(VisibleDefaultsOnly)
	TArray<class AActor*> PlayerActors;

	// Get, Set
	// 체력(임시) 
	UFUNCTION(BlueprintCallable)
	void SetCurrentHp(float HpValue) { CurrentHp = HpValue; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentHp() const { return CurrentHp; }

	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* GetLaserSpawnPointMesh() const { return LaserSpawnPointMesh; }

	UFUNCTION(BlueprintCallable)
	class AEnemyMoonBaboon* GetMoonBaboonActor() const { return EnemyMoonBaboon; }

	UFUNCTION(BlueprintCallable)
	AFloor* GetFloor() { return FloorObject; }
	
	UFUNCTION(BlueprintCallable)
	const bool IsCodyHoldingEnter() const { return bIsCodyHoldingEnter; }

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector PrevAnimBoneLocation = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable)
	const int32 GetCurrentState();

	UFUNCTION(BlueprintCallable)
	ACody* GetPlayerCody() const { return PlayerCody; }

	UFUNCTION(BlueprintCallable)
	AMay* GetPlayerMay() const { return PlayerMay; }


	enum class EBossState
	{
		None,

		Intro,
		Phase1_Progress_LaserBeam_1,
		Phase1_Progress_LaserBeam_1_Destroy,
		Phase1_Progress_ArcingProjectile_1,
		Phase1_Progress_LaserBeam_2,
		Phase1_Progress_LaserBeam_2_Destroy,
		Phase1_Progress_ArcingProjectile_2,
		Phase1_Progress_LaserBeam_3,
		Phase1_BreakThePattern,

		CodyHolding_Enter,
		CodyHolding_Low,
		CodyHolding_ChangeOfAngle,
		CodyHolding_InputKey,
		CodyHolding_ChangeOfAngle_Reverse,
		Phase1_ChangePhase_2,

		Phase2_RotateSetting,
		Phase2_Rotating,
		Phase2_RocketHit,
		Phase2_BreakThePattern,
		Phase2_ChangePhase_Wait,
		Phase2_Fly,
		Phase2_MoveToCenter,

		Phase3_MoveFloor,
		Phase3_MoveToTarget,
		Phase3_GroundPounding,

		Phase3_Eject,


		FireHomingRocket,
		FireArcingProjectile,

		AllPhaseEnd,
		TestState
	};

	void EnableEject() 
	{ 
		bIsEject = true; 
		bIsAllPhaseEnd = true;
	}
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// 보스애니메이션 변경시 사용하는 애니메이션 리소스 정보 애니메이션 시퀀스 or 애니메이션 블루프린트 
	enum class EAnimationAssetType : uint8
	{
		None,
		Sequence,
		Blueprint,
	};

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SetupOverlapEvent();

	// 멀티캐스트 관련 
	// multicast 함수 
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ChangeAnimationFlyingSaucer(const FString& AnimPath, const uint8 AnimType, bool AnimLoop);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ChangeAnimationMoonBaboon(const FString& AnimPath, const uint8 AnimType, bool AnimLoop);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CheckCodyKeyPressedAndChangeState(const bool bIsInput);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetActivateUIComponent(UInteractionUIComponent* UIComponent, bool ParentUIActivate, bool ChildUIActivate);

	int32 GetFloorCurrentState();
	void DrawDebugMesh();

	// 클라이언트 접속까지의 대기시간, 추후 수정할수도 
	UPROPERTY(EditDefaultsOnly)
	float ServerDelayTime = 6.0f;

	void SetupComponent();
	void SetupFsmComponent();
	
	// 패턴 파훼시 플레이어 추가 키 입력 관련 변수 
	UPROPERTY(EditDefaultsOnly)
	bool bIsKeyInput = false;
	float KeyInputTime = 0.0f;
	float KeyInputMaxTime = 1.25f;
	float KeyInputAdditionalTime = 0.75f;

	UPROPERTY(Replicated, EditAnywhere)
	float CurrentHp = 100.0f;

	UPROPERTY(Replicated, EditAnywhere)
	float MaxHp = 100.0f;

	// 특정시간 내에 State 변경 시 해당 변수 사용
	UPROPERTY(EditDefaultsOnly)
	float StateCompleteTime = 0.0f;

	// tsubclass
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AEnemyMoonBaboon> EnemyMoonBaboonClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AHomingRocket> HomingRocketClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AArcingProjectile> ArcingProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AFloor> FloorClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AEnergyChargeEffect> EnergyChargeEffectClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AGroundPoundEffect> GroundPoundEffectClass = nullptr;

	// 보스 투사체 관련 
	UFUNCTION(BlueprintCallable)
	void ResetArcingProjectileFireCount() { ArcingProjectileFireCount = 0; }
	
	UPROPERTY(EditAnywhere)
	int32 ArcingProjectileFireCount = 0;

	// 보스가 생성하는 액터 
	UPROPERTY(Replicated, EditDefaultsOnly)
	class AEnemyMoonBaboon* EnemyMoonBaboon = nullptr;

	UPROPERTY(EditAnywhere)
	class AFloor* FloorObject = nullptr;
	
	UPROPERTY(Replicated, EditDefaultsOnly)
	class AEnergyChargeEffect* EnergyChargeEffect = nullptr;

	// Component
	UPROPERTY(Replicated, EditDefaultsOnly)
	class UFsmComponent* FsmComp = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class UInteractionUIComponent* CodyHoldingUIComp = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class UInteractionUIComponent* MayLaserDestroyUIComp = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class URotatingMovementComponent* RotatingComp = nullptr;

	UPROPERTY(Replicated, EditAnywhere)
	class USkeletalMeshComponent* SkeletalMeshComp = nullptr;


	// 오버랩 체크 관련 
	UFUNCTION(BlueprintCallable)
	void SpawnOverlapCheckActor();

	UPROPERTY(Replicated, EditDefaultsOnly)
	class AOverlapCheckActor* OverlapCheckActor = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AOverlapCheckActor> OverlapCheckActorClass = nullptr;

	// 보스 공격 생성 지점, 디버그드로우 활성화로 위치 확인 가능 
	UPROPERTY(Replicated, EditDefaultsOnly)
	class UStaticMeshComponent* LaserSpawnPointMesh = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class UStaticMeshComponent* HomingRocketSpawnPointMesh1 = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class UStaticMeshComponent* HomingRocketSpawnPointMesh2 = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class UStaticMeshComponent* ArcingProjectileSpawnPointMesh = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class UAnimInstance* AnimInstance = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	class UAnimSequence* AnimSequence = nullptr;

	UPROPERTY(Replicated, EditDefaultsOnly)
	bool bIsCodyHoldingEnter = false;

	// 레이저 추적 로직 관련
	void UpdateLerpRatioForLaserBeam(float DeltaTime);
	void SavePreviousTargetLocation();

	UFUNCTION(BlueprintCallable)
	void SetupLaserTargetActor();

	// 코디 홀딩 상태 시작시 코디 위치 이동에 관련한 값
	// 이동 시켜서 고정시킬 코디 위치
	FVector CodyLerpEndLocation = FVector(521.47f, -568.51f, 376.55f);

	// 러프 비율을 저장할 float
	float CodyLerpRatio = 0.0f;
	// 러프 완료를 체크할 bool 
	bool bIsCodyHoldingLerpEnd = false;
	
	void SetCodyHoldingEnter_CodyLocation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetFocusHoldingCody();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnPossess();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HideLaserBaseBone();

	UPROPERTY(Replicated)
	class ACody* PlayerCody = nullptr;

	UPROPERTY(Replicated)
	class AMay* PlayerMay = nullptr;

	bool bIsCorretLocation = false;
	FVector RotatePivotVector = FVector(3959.88f, 60.44f, 0.0f);
	float HomingRocketFireTime = 0.0f;
	float HomingRocketCoolTime = 3.0f;

	// 로켓 액터 
	class AHomingRocket* HomingRocketActor_1 = nullptr;
	class AHomingRocket* HomingRocketActor_2 = nullptr;

	// 센터이동로직관련 
	float MoveToCenterLerpRatio = 0.0f;
	FVector MoveStartLocation = FVector::ZeroVector;

	// GroundPound 이동관련 
	float MoveToTargetLerpRatio = 0.0f;
	FVector GroundPoundTargetLocation = FVector::ZeroVector;
	
	float GroundPoundEffectCreateTime = 1.61f;
	bool bIsSetGroundPoundEffect = false;

	// 보스 탈출 관련
	UPROPERTY(Replicated)
	bool bIsEject = false;

	class APlayerController* ViewTargetChangeController = nullptr;
	class AActor* PrevViewTarget = nullptr;

	UPROPERTY(EditAnywhere)
	class APhaseEndCameraRail* Phase1EndCameraRail = nullptr;

	UPROPERTY(EditAnywhere)
	class APhaseEndCameraRail* Phase2EndCameraRail = nullptr;

	UPROPERTY(EditAnywhere)
	class APhaseEndCameraRail* Phase3EndCameraRail_1 = nullptr;

	UPROPERTY(EditAnywhere)
	class APhaseEndCameraRail* Phase3EndCameraRail_2 = nullptr;


	UPROPERTY(EditAnywhere)
	bool bIsAllPhaseEnd = false;
};
