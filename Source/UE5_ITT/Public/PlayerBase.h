// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RespawnTrigger.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "Interfaces/OnlineSessionInterface.h"


#include "PlayerBase.generated.h"

UENUM(BlueprintType)
enum class CodySize : uint8
{
	NONE UMETA(DisPlayName = "NONE"),
	BIG UMETA(DisPlayName = "BIG"),
	NORMAL UMETA(DisPlayName = "NORMAL"),
	SMALL UMETA(DisPlayName = "SMALL"),
};
UENUM(BlueprintType)
enum class Cody_State : uint8
{
	IDLE UMETA(DisPlayName = "IDLE"),
	MOVE UMETA(DisPlayName = "MOVE"),
	SIT UMETA(DisPlayName = "SIT"),
	JUMP UMETA(DisPlayName = "JUMP"),
	DASH UMETA(DisPlayName = "DASH"),
	DASHEND UMETA(DisPlayName = "DASHEND"),
	PlayerDeath UMETA(DisPlayName = "PlayerDeath"),
	HOLDENEMY UMETA(DisPlayName = "HoldEnemy"),
	FLYING UMETA(DisPlayName = "FLYING"),
	CUTSCENE UMETA(DisPlayName = "CUTSCENE"),
};

UCLASS()
class UE5_ITT_API APlayerBase : public ACharacter
{
	GENERATED_BODY()
	UInputComponent* Input;
	ARespawnTrigger* ResPawnTriggers = nullptr;
public:
	UEnhancedInputComponent* PlayerInput;
	// Sets default values for this character's properties
	APlayerBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	///////////////////State///////////////////
	UFUNCTION(BlueprintCallable)
	inline void ChangeState(Cody_State _State)
	{
		ITTPlayerState = _State;
	}
	///////////////////////////////////////////
	
	///////////////////Input///////////////////
	//키가 매핑된 Context입니다
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* CodyMappingContext;

	//IA_Jump
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;
	//IA_Move
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MoveAction;
	//IA_Look
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;
	//IA_LeftM
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LeftMAction;
	//IA_RightM
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* RightMAction;
	//IA_Sit
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* SitAction;
	//IA_Dash
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* DashAction;
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* TestAction;
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* FlyMoveAction;
	///////////////////////////////////////////






	/////////////////Function/////////////////
	//Cody의 현재 state를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline Cody_State GetITTPlayerState() const
	{
		return ITTPlayerState;
	}
	//캐릭터 스프링암에 포합된 카메라 컴포넌트를 반환합니다.
	FORCEINLINE class UCameraComponent* GetCodyCameraComponent() const
	{
		return PlayerCameraComponent;
	}
	//캐릭터의 스프링암을 반환합니다.
	FORCEINLINE class USpringArmComponent* GetSpringArm() const
	{
		return SpringArm;
	}
	//플레이어의 이동이 끝났는지를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetIsMoveEnd() const
	{
		return IsMoveEnd;
	}
	//이동이 끝났을때 호출됩니다(CodyAnimNotify.cpp)
	inline void SetIsMoveEndT()
	{
		IsMoveEnd = true;
	}
	//대쉬 키를 눌렀는지를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetbIsDashing() const
	{
		return bIsDashing;
	}
	//대쉬의 지속시간을 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline float GetDashDuration()
	{
		return DashDuration;
	}
	//대쉬시작 애니메이션이 끝났는지를 확인합니다
	UFUNCTION(BlueprintCallable)
	inline bool GetbIsDashingStart() const
	{
		return bIsDashingStart;
	}
	//대쉬 시작 애니메이션이 끝났을 때 호출됩니다(CodyNotify_DashStart.cpp)
	inline void SetbIsDashingStart()
	{
		bIsDashingStart = true;
	}
	// 대쉬가 다시 입력가능한 상태인지를 확인합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetbCanDash() const
	{
		return bCanDash;
	}
	//Player의 HP를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline int32 GetPlayerHP() const
	{
		return PlayerHP;
	}
	//Player의 최대체력을 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline int32 GetPlayerFullHP() const
	{
		return FullHP;
	}

	//Player를 공격하는 함수입니다 인수=데미지
	UFUNCTION(BlueprintCallable)
	void AttackPlayer(const int att);
	
	UFUNCTION(Client, Reliable)
	void AttackPlayerClient(const int att);
	UFUNCTION(Server, Reliable, WithValidation)
	void AttackPlayerServer(const int att);

	//카메라의 Vector2D를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline FVector2D GetCameraVector() const
	{
		return CameraLookVector;
	}
	//캐릭터에 적용되어있는 기본 지면 마찰력을 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline float GetPlayerGroundFriction() const
	{
		return DefaultGroundFriction;
	}
	//캐릭터에 적용되어있는 기본 중력 스케일을 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline float GetPlayerDefaultGravityScale() const
	{
		return DefaultGravityScale;
	}
	//캐릭터에 현재 적용되어있는 중력 스케일을 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline float GetPlayerGravityScale() const
	{
		return GetCharacterMovement()->GravityScale;
	}
	//현재 중력이 캐릭터 Scale에 관계없이 정상 적용되고있는지를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline float GetIsDGravityScale() const
	{
		return IsDGravity;
	}
	//애니메이션이 끝났을 때 호출됩니다
	UFUNCTION(BlueprintCallable)
	inline void SetCurrentAnimationEndT()
	{
		CurrentAnimationEnd = true;
	}
	//애니메이션을 끝나지 않은 상태로 변경합니다.
	UFUNCTION(BlueprintCallable)
	inline void SetCurrentAnimationEndF()
	{
		CurrentAnimationEnd = false;
	}
	//이전 애니메이션이 끝났는지를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetCurrentAnimationEnd() const
	{
		return CurrentAnimationEnd;
	}
	//상호작용 키를 눌렀을때 true를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetIsInteract() const
	{
		return IsInteract;
	}
	// Player가 달리고있는 상태인지를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetIsSprint() const
	{
		return IsSprint;
	}
	//Player가 죽은상태인지를 반환합니다. 리스폰UI visible여부는 이걸로 확인.
	UFUNCTION(BlueprintCallable)
	inline bool GetIsPlayerDeath() const
	{
		return IsPlayerDeath;
	}
	//플레이어를 죽어있는 상태로 만듭니다.
	UFUNCTION(BlueprintCallable)
	inline void SetPlayerDeath(bool IsDeath = true)
	{
		IsPlayerDeath = IsDeath;
	}
	//Sit에서 내려오는 시간이 되었는지를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetCurrentSitTime() const
	{
		return CurrentSitTime >= SitStartTime + SitDuration - 0.15f;
	}
	//캐릭터가 지면에 있는지를 반환합니다
	UFUNCTION(BlueprintCallable)
	inline bool GetIsCharacterGround() const
	{
		return !GetMovementComponent()->IsFalling();
	}
	//Idle로 돌아갈 수 있는 상태로 만듭니다.(Sit)
	UFUNCTION(BlueprintCallable)
	inline void SetChangeIdle()
	{
		ChangeIdle = true;
	}
	//Idle로 돌아갈 수 있는 상태인지를 반환합니다.(Sit)
	UFUNCTION(BlueprintCallable)
	inline bool GetChangeIdle() const
	{
		return ChangeIdle;
	}
	//현재 캐릭터가 몇번째 점프를 하고있는지를 반환합니다
	UFUNCTION(BlueprintCallable)
	inline int32 GetCharacterJumpCount() const
	{
		return CharacterJumpCount;
	}
	//앉기시작하는모션이 가능한지 여부를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetIsSit() const
	{
		return IsSit;
	}
	//Cody가 큰 사이즈인지를 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline bool GetIsBig() const
	{
		return IsBig;
	}
	//Player가 Fly 상태인지를 반환합니다
	UFUNCTION(BlueprintCallable)
	inline bool GetIsFly() const
	{
		return IsFly;
	}
	//플레이어를 날고있지 않은 상태로 만듭니다.
	UFUNCTION(BlueprintCallable)
	inline void SetFlyFalse()
	{
		IsFly = false;
	}
	UFUNCTION(BlueprintCallable)
	inline void OverlapHomingFunc()
	{
		OverlapHoming = true;
	}
	UFUNCTION(BlueprintCallable)
	inline void SetOverlapHoming()
	{
		OverlapHoming = false;
	}
	UFUNCTION(BlueprintCallable)
	inline bool GetOverlapHoming() const
	{
		return OverlapHoming;
	}
	//Cody가 EnemyBoss를 들고있는 상태로 만듭니다.(움직임에 관여합니다)
	UFUNCTION(BlueprintCallable)
	inline void SetCodyHoldEnemyTrue()
	{
		CodyHoldEnemy = true;
	}
	//Cody가 EnemyBoss를 들고있지 않은 상태로 만듭니다.
	UFUNCTION(BlueprintCallable)
	inline void SetCodyHoldEnemyFalse()
	{
		CodyHoldEnemy = false;
	}
	//캐릭터가 리스폰되는 위치의 Vector값을 반환합니다.
	UFUNCTION(BlueprintCallable)
	inline FVector GetResPawnPosition()
	{
		return ResPawnPosition;
	}
	//ResPawn관련 TriggerActor와 충돌했을때 호출됩니다. 인수=충돌한TriggerActor
	UFUNCTION(BlueprintCallable)
	inline void SetTriggerActors(ARespawnTrigger* _Other);
	//충돌한 TriggerActor의 Position을 받아 Respawn지역을 세팅하는 함수.
	UFUNCTION(BlueprintCallable)
	inline void SetRespawnPosition();



	


	///////////////////Movement+Replicate Function///////////////////
	UFUNCTION(Client, Reliable)
	void CustomClientIdle();
	UFUNCTION(Server, Reliable, WithValidation)
	void CustomServerIdle();

	UFUNCTION(Client,Reliable)
	void ChangeClientDir(FRotator _Rotator);
	UFUNCTION(Server,Reliable, WithValidation)
	void ChangeServerDir(FRotator _Rotator);

	UFUNCTION(Client, Reliable)
	void ChangeClientFlyDir(FRotator _Rotator);
	UFUNCTION(Server, Reliable, WithValidation)
	void ChangeServerFlyDir(FRotator _Rotator);

	UFUNCTION(Client, Reliable)
	void CustomClientDash();
	UFUNCTION(Server, Reliable, WithValidation)
	void CustomServerDash();
	
	UFUNCTION(Server, Reliable)
	void InteractInput();
	UFUNCTION(Server, Reliable)
	void InteractNoneInput();


	void Idle(const FInputActionInstance& _Instance);
	void Look(const FInputActionInstance& _Instance);
	virtual void SprintInput() {};
	virtual void SprintNoneInput() {};
	void CustomMove(const FInputActionInstance& _Instance);
	void CustomFlyMove(const FInputActionInstance& _Instance);
	void CustomFlyNoneMove(const FInputActionInstance& _Instance);
	void DashInput();
	void DashNoneInput();
	void GroundDash();
	virtual void DashEnd() {};
	void JumpDash();
	///////////////////////////////////////////////////////

	//void GetOnlineSubsystem();
	//IOnlineSessionPtr OnlineSeesioninterface;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	///////////////////Widget//////////////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	class UMarkerWidget* MarkerUIWidget;

	UFUNCTION(BlueprintCallable)
	void SettingMarkerWidget();
	///////////////////////////////////////////



	///////////////////Overlap//////////////////
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	////////////////////////////////////////////



	///////////////////Rocket//////////////////
	void PlayerToHomingRocketJumpStart();

	UFUNCTION(BlueprintCallable)
	inline void PlayerToHomingRoketJumpFinished();
	UFUNCTION(Client, Reliable)
	inline void ClientPlayerToHomingRoketJumpFinished();
	UFUNCTION(Server, Reliable, WithValidation)
	inline void ServerPlayerToHomingRoketJumpFinished();

	UFUNCTION(Client, Reliable)
	void CustomClientRideJump();
	UFUNCTION(Server, Reliable, WithValidation)
	void CustomServerRideJump();
	///////////////////////////////////////////
	

	

	///////////////////??????//////////////////

	UPROPERTY(Replicated)
	FRotator CurControllerRot;

	UFUNCTION(BlueprintCallable)
	inline FRotator GetCurControllerRot() const
	{
		return CurControllerRot;
	}
	UPROPERTY(Replicated)
	FVector CurControllerLoc;

	UFUNCTION(BlueprintCallable)
	inline FVector GetCurControllerLoc() const
	{
		return CurControllerLoc;
	}
	void UpdateCamTrans();
	///////////////////////////////////////////


protected:
	///////////////////Player/////////////////////
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = Player)
	int32 PlayerHP = 12;
	int32 FullHP = 12;
	UPROPERTY(Replicated)
	bool IsPlayerDeath = false;


	UFUNCTION(BlueprintCallable)
	void PlayerDeathCheck();
	UPROPERTY(Replicated)
	float NowPlayerGravityScale;
	UPROPERTY(Replicated)
	float PlayerJumpZVelocity;
	float PlayerDefaultJumpHeight;
	//////////////////////////////////////////////

	//////////////////Camera////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* PlayerCameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	float BigLength;
	float NormalLength;
	float SmallLength;
	FVector2D CameraLookVector = FVector2D::ZeroVector;
	UPROPERTY(Replicated)
	FVector ResPawnPosition = FVector(1000.0f, 1000.0f, 100.0f); //임시

	//SpringArm기본 세팅 함수입니다.
	UFUNCTION(BlueprintCallable)
	void SpringArmDefaultFunction();
	//////////////////////////////////////////////
	//////////////////Movement////////////////////
	UPROPERTY(Replicated)
	float DashDistance = 2500.0f; // 앞구르기 거리
	bool IsDGravity;
	UPROPERTY(Replicated)
	float PlayerDefaultSpeed;
	UPROPERTY(Replicated)
	float NowPlayerSpeed;
	UPROPERTY(Replicated = OnRep_IsMoveEnd)
	bool IsMoveEnd; //움직임이 끝났는지
	UFUNCTION()
	void OnRep_IsMoveEnd();
	UPROPERTY(Replicated)
	float DashDuration; // 앞구르기 지속 시간
	UPROPERTY(Replicated)
	float DashStartTime;//앞구르기 시작시간 체크
	UPROPERTY(Replicated)
	bool bIsDashing; // 앞구르기 중 여부를 나타내는 플래그
	UPROPERTY(Replicated)
	bool bIsDashingStart; //앞구르기 시작단계를 나타내는 플래그
	UPROPERTY(Replicated)
	bool bCanDash; //대쉬가 가능한 상태인지
	FTimerHandle DashTimerHandle; // 앞구르기 타이머 핸들
	float DefaultGroundFriction; // 기본 지면 마찰력
	UPROPERTY(Replicated)
	float DefaultGravityScale; //기본 중력
	bool BigCanDash; //커진상태에서 대쉬가 가능한지 여부
	UPROPERTY(Replicated)
	float DashCurrentTime;
	UPROPERTY(Replicated)
	bool IsSprint; //달리기 여부

	UPROPERTY(Replicated)
	float SitStartTime;
	UPROPERTY(Replicated)
	float SitDuration;
	UPROPERTY(Replicated)
	bool CanSit;
	UPROPERTY(Replicated)
	bool IsSit;
	UPROPERTY(Replicated)
	float CurrentSitTime;
	UPROPERTY(Replicated)
	bool ChangeIdle;
	UPROPERTY(Replicated)
	int32 CharacterJumpCount;
	UPROPERTY(Replicated)
	FVector MoveDirection;
	UPROPERTY(Replicated)
	FVector2D MoveInput;
	UPROPERTY(Replicated)
	FRotator ControllerRotation;
	UPROPERTY(Replicated)
	FRotator CustomTargetRotation;
	UPROPERTY(Replicated)
	FVector WorldForwardVector;
	UPROPERTY(Replicated)
	FVector WorldRightVector;
	UPROPERTY(Replicated)
	FVector DashDirection;
	UPROPERTY(Replicated)
	FVector DashVelocity;

	UPROPERTY(Replicated)
	int32 CustomPlayerJumpCount;

	UPROPERTY(Replicated)
	FVector FlyForwardVector;

	//Fly
	UPROPERTY(Replicated)
	bool IsFly = false;
	FVector CurrentDirection;
	UPROPERTY(Replicated)
	float FlyingSpeed;
	//////////////////////////////////////////////

	///////////////////Cody//////////////////
	UPROPERTY(Replicated)
	CodySize CurCodySize = CodySize::NONE;
	UPROPERTY(Replicated)
	CodySize NextCodySize = CodySize::NONE;

	//CodySize
	UPROPERTY(Replicated)
	bool IsBig;
	UPROPERTY(Replicated)
	bool CodyHoldEnemy = false;
	/////////////////////////////////////////
private:
	///////////////////Widget//////////////////
	UCapsuleComponent* CustomPlayerCapsuleComponent;

	///////////////////Rocket//////////////////
	UPROPERTY(Replicated)
	FVector CunstomEndLocation;
	UPROPERTY(Replicated)
	FVector CunstomStartLocation;
	UPROPERTY(Replicated)
	bool JumplocationSet = false;
	UPROPERTY(Replicated)
	float JumpLocationDeltas = 0.0f;
	UPROPERTY(Replicated)
	double CustomTargetLocations;
	UPROPERTY(Replicated)
	double CustomTargetLocationsY;
	UPROPERTY(Replicated)
	FVector ResultTargetLocations;
	bool OverlapHoming = false;

	
	///////////////////Animation//////////////////
	UPROPERTY(Replicated)
	bool CurrentAnimationEnd;
	//////////////////////////////////////////////

	/////////////////Controller///////////////////
	APlayerController* CodyController = nullptr;
	//////////////////////////////////////////////

	///////////////////State/////////////////////
	UPROPERTY(Replicated)
	Cody_State ITTPlayerState;
	//////////////////////////////////////////////

	//////////////////////////////////////////
	//마우스 돌아가는 스피드
	float RotationInterpSpeed = 2.0f;
	//상호작용
	UPROPERTY(Replicated)
	bool IsInteract = false;
	//////////////////////////////////////////
};
