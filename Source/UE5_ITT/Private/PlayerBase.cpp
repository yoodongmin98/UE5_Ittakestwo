// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "RespawnTrigger.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "HomingRocket.h"
#include "GameFramework/SpringArmComponent.h"
#include "Logging/LogMacros.h"
//#include "OnlineSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "MarkerWidget.h"

// Sets default values
APlayerBase::APlayerBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(FName("Player"));

	BigLength = 2000.0f;
	NormalLength = 1200.0f;
	SmallLength = 200.0f;

	// 캐릭터이동
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	//스프링 암
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmDefaultFunction();

	//카메라 생성
	PlayerCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	PlayerCameraComponent->bUsePawnControlRotation = false;

	PrimaryActorTick.bStartWithTickEnabled = true;

	MarkerUIWidget = CreateDefaultSubobject<UMarkerWidget>(TEXT("MarkerWidgetCom"));
	MarkerUIWidget->SetupAttachment(RootComponent);
}

void APlayerBase::SettingMarkerWidget()
{
	//MarkerUIWidget->SettingCustomVisible();
}


//void APlayerBase::GetOnlineSubsystem()
//{
//
//	// OnlineSubsystem에 Access
//	IOnlineSubsystem* CurOnlineSubsystem = IOnlineSubsystem::Get();
//	if (CurOnlineSubsystem)
//	{
//		// 온라인 세션 받아오기
//		OnlineSeesioninterface = CurOnlineSubsystem->GetSessionInterface();
//
//		if (GEngine)
//		{
//			// OnlineSubsystem 이름 출력하기
//			//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("Found subsystem %s"), *CurOnlineSubsystem->GetSubsystemName().ToString()));
//		}
//	}
//}

// Called when the game starts or when spawned
void APlayerBase::BeginPlay()
{
	Super::BeginPlay();

	CustomPlayerCapsuleComponent = GetCapsuleComponent();
	CustomPlayerCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerBase::OnOverlapBegin);
	CustomPlayerCapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APlayerBase::OnOverlapEnd);
	

	//GetOnlineSubsystem();
	//입력
	CodyController = Cast<APlayerController>(Controller);
	if (CodyController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CodyController->GetLocalPlayer());
		if (Subsystem != nullptr)
		{
			Subsystem->AddMappingContext(CodyMappingContext, 0);
		}
	}
	ITTPlayerState = Cody_State::IDLE;
	//Set
	PlayerHP = FullHP; //Player기본 Hp설정
	DashDuration = 0.7f; //Dash 시간
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction; //기본 지면 마찰력
	DefaultGravityScale = GetCharacterMovement()->GravityScale; //기본 중력 스케일
	PlayerDefaultSpeed = GetCharacterMovement()->MaxWalkSpeed; //기본 이동속도

	NowPlayerGravityScale = DefaultGravityScale;
	PlayerJumpZVelocity = GetCharacterMovement()->JumpZVelocity;
	PlayerDefaultJumpHeight = GetCharacterMovement()->JumpZVelocity;

	IsMoveEnd = false;
	bIsDashing = false;
	bIsDashingStart = false;
	bCanDash = false;
	BigCanDash = true;
	CurrentAnimationEnd = false;
	IsSprint = false;
	CanSit = true;
	SitDuration = 0.5f;
	ChangeIdle = true;
	CustomPlayerJumpCount = ACharacter::JumpMaxCount;
	FlyingSpeed = 600.0f;
}

// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	////////////////동기화를 위한 Tick Check/////////////////////
	 
	//점프 횟수 확인
	CharacterJumpCount = JumpCurrentCount;
	//중력상태확인
	GetCharacterMovement()->GravityScale = NowPlayerGravityScale;
	//이동속도 체크
	GetCharacterMovement()->MaxWalkSpeed = NowPlayerSpeed;
	GetCharacterMovement()->JumpZVelocity = PlayerJumpZVelocity;
	//플레이어 생존여부 확인
	PlayerDeathCheck();

	///////////////////////////////////////////////////////////


	if (GetCharacterMovement()->GravityScale <=5.5f)
	{
		IsDGravity = true;
	}
	else
	{
		IsDGravity = false;
	}
	//대쉬의 지속시간을 Tick에서 지속적으로 확인
	if (bIsDashing && bCanDash)
	{
		DashCurrentTime = GetWorld()->GetTimeSeconds();
		if (DashCurrentTime >= DashStartTime + DashDuration)
		{
			// 대시 지속 시간이 지나면 대시 종료
			DashEnd();
		}
	}

	if (JumplocationSet)
	{
		JumpLocationDeltas += GetWorld()->DeltaTimeSeconds;
		CustomTargetLocations = FMath::Lerp(CunstomStartLocation.X, CunstomEndLocation.X, JumpLocationDeltas);
		CustomTargetLocationsY = FMath::Lerp(CunstomStartLocation.Y, CunstomEndLocation.Y, JumpLocationDeltas);
		ResultTargetLocations = FVector(CustomTargetLocations, CustomTargetLocationsY, GetActorLocation().Z);
		SetActorLocation(ResultTargetLocations);
		if (JumpLocationDeltas >= 1.0f)
		{
			JumpLocationDeltas = 1.0f;
		}
	}
	UpdateCamTrans();
}

// Called to bind functionality to input
void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	Input = PlayerInputComponent;
	PlayerInputComponent->BindAction(TEXT("Player_Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	


	PlayerInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (PlayerInput != nullptr)
	{
		PlayerInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerBase::CustomMove);
		PlayerInput->BindAction(MoveAction, ETriggerEvent::None, this, &APlayerBase::Idle);

		PlayerInput->BindAction(FlyMoveAction, ETriggerEvent::Triggered, this, &APlayerBase::CustomFlyMove);
		PlayerInput->BindAction(FlyMoveAction, ETriggerEvent::None, this, &APlayerBase::CustomFlyNoneMove);

		PlayerInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerBase::Look);
		PlayerInput->BindAction(DashAction, ETriggerEvent::Triggered, this, &APlayerBase::DashInput);
		PlayerInput->BindAction(DashAction, ETriggerEvent::None, this, &APlayerBase::DashNoneInput);
		PlayerInput->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerBase::InteractInput);
		PlayerInput->BindAction(InteractAction, ETriggerEvent::None, this, &APlayerBase::InteractNoneInput);
		PlayerInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APlayerBase::SprintInput);
		PlayerInput->BindAction(SprintAction, ETriggerEvent::None, this, &APlayerBase::SprintNoneInput);

		
	}
}


//////////////////////////////캐릭터 이동관련///////////////////////////
//새로운 입력 Action
void APlayerBase::Idle(const FInputActionInstance& _Instance)
{
	if (HasAuthority() == true)
	{
		CustomClientIdle();
	}
	else
	{
		CustomServerIdle();
	}
}

void APlayerBase::CustomClientIdle_Implementation()
{
	IsSprint = false;
	IsMoveEnd = false;
	if (bCanDash == false && !IsPlayerDeath && !IsFly)
		ChangeState(Cody_State::IDLE);
}

bool APlayerBase::CustomServerIdle_Validate()
{
	return true;
}
void APlayerBase::CustomServerIdle_Implementation()
{
	OnRep_IsMoveEnd();
}

void APlayerBase::OnRep_IsMoveEnd()
{
	IsSprint = false;
	IsMoveEnd = false;
	if (bCanDash == false && !IsPlayerDeath)
		ChangeState(Cody_State::IDLE);
}
void APlayerBase::CustomMove(const FInputActionInstance& _Instance)
{
	if (!IsFly && !CodyHoldEnemy)
	{
		if (bCanDash == false && ChangeIdle)
		{
			ChangeState(Cody_State::MOVE);
			// 컨트롤러의 회전 방향을 가져옴
			ControllerRotation = Controller->GetControlRotation();
			CustomTargetRotation = FRotator(0.f, ControllerRotation.Yaw, 0.f);
			// Move를 시작할 때 카메라의 위치를 반영하여 SetRotation함(그 전까지는 자유시점)
			// 컨트롤러의 회전 방향에서 Yaw 각도만 사용하여 캐릭터를 회전시킴

			// 컨트롤러의 회전 방향을 기준으로 월드 전방 벡터를 계산
			WorldForwardVector = FRotationMatrix(ControllerRotation).GetScaledAxis(EAxis::Y);
			// 컨트롤러의 회전 방향을 기준으로 월드 오른쪽 벡터를 계산
			WorldRightVector = FRotationMatrix(ControllerRotation).GetScaledAxis(EAxis::X);

			// 캐릭터를 입력 방향으로 이동시킴
			MoveInput = _Instance.GetValue().Get<FVector2D>();
			if (!MoveInput.IsNearlyZero())
			{
				// 입력 방향 노말라이즈
				MoveInput = MoveInput.GetSafeNormal();

				// MoveDirection기준으로 Yaw부분만 Player Rotation에 적용
				MoveDirection = WorldForwardVector * MoveInput.Y + WorldRightVector * MoveInput.X;
				FRotator CodyRotation(0.0f, MoveDirection.Rotation().Yaw, 0.0f);
				CustomTargetRotation = CodyRotation;

				// 입력 방향에 따라 캐릭터를 이동시킴
				AddMovementInput(MoveDirection);
			}
		}
		if (HasAuthority())
		{
			ChangeClientDir(CustomTargetRotation);
		}
		else
		{
			ChangeServerDir(CustomTargetRotation);
		}
	}
}

void APlayerBase::CustomFlyMove(const FInputActionInstance& _Instance)
{
	if (IsFly && !CodyHoldEnemy)
	{
		ChangeState(Cody_State::FLYING);
		if (bCanDash == false && ChangeIdle)
		{
			// 컨트롤러의 회전 방향을 가져옴
			ControllerRotation = Controller->GetControlRotation();
			CustomTargetRotation = FRotator(0.f, ControllerRotation.Yaw, 0.f);
			// Move를 시작할 때 카메라의 위치를 반영하여 SetRotation함(그 전까지는 자유시점)
			// 컨트롤러의 회전 방향에서 Yaw 각도만 사용하여 캐릭터를 회전시킴

			// 컨트롤러의 회전 방향을 기준으로 월드 전방 벡터를 계산
			WorldForwardVector = FRotationMatrix(ControllerRotation).GetScaledAxis(EAxis::Y);
			WorldRightVector = FRotationMatrix(ControllerRotation).GetScaledAxis(EAxis::Z);

			// 캐릭터를 입력 방향으로 이동시킴
			MoveInput = _Instance.GetValue().Get<FVector2D>();
			if (!MoveInput.IsNearlyZero())
			{
				// 입력 방향 노말라이즈
				MoveInput = MoveInput.GetSafeNormal();

				// MoveDirection기준으로 Yaw부분만 Player Rotation에 적용
				MoveDirection = WorldForwardVector * MoveInput.Y + WorldRightVector * MoveInput.X;
				// 컨트롤러의 회전 방향을 기준으로 월드 오른쪽 벡터를 계산.X;

					// 입력 방향에 따라 캐릭터를 이동시킴
				if (MoveInput.X >= 0)
					AddMovementInput(FVector(MoveDirection.X, MoveDirection.Y, MoveDirection.Z * 0.5f));
				else
					AddMovementInput(FVector(-MoveDirection.X, -MoveDirection.Y, MoveDirection.Z * 0.5f));
				///////////////////////////////////////////////////////////////////////////////////////////
			}
			if (HasAuthority() == true)
			{
				ChangeClientFlyDir(CustomTargetRotation);
			}
			else
			{
				ChangeServerFlyDir(CustomTargetRotation);
			}
		}
	}
}

void APlayerBase::CustomFlyNoneMove(const FInputActionInstance& _Instance)
{
	
}

void APlayerBase::ChangeClientDir_Implementation(FRotator _Rotator)
{
	IsMoveEnd = true;
	SetActorRotation(_Rotator);
}

bool APlayerBase::ChangeServerDir_Validate(FRotator _Rotator)
{
	return true;
}

void APlayerBase::ChangeServerDir_Implementation(FRotator _Rotator)
{
	IsMoveEnd = true;
	SetActorRotation(_Rotator);
}


void APlayerBase::ChangeClientFlyDir_Implementation(FRotator _Rotator)
{
	SetActorRotation(_Rotator);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	FlyForwardVector = GetActorForwardVector();
	GetCharacterMovement()->MaxFlySpeed = 1500.0f;
	AddActorWorldOffset(FlyForwardVector * FlyingSpeed * (GetWorld()->DeltaRealTimeSeconds));
}
bool APlayerBase::ChangeServerFlyDir_Validate(FRotator _Rotator)
{

	return true;
}
void APlayerBase::ChangeServerFlyDir_Implementation(FRotator _Rotator)
{
	SetActorRotation(_Rotator);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	FlyForwardVector = GetActorForwardVector();
	GetCharacterMovement()->MaxFlySpeed = 1500.0f;
	AddActorWorldOffset(FlyForwardVector * FlyingSpeed * (GetWorld()->DeltaRealTimeSeconds));
}




void APlayerBase::Look(const FInputActionInstance& _Instance)
{
	if (Controller != nullptr)
	{
		const TArray<FName>& CheckTag = Tags;
		for (const FName& V : CheckTag)
		{
			if (V == FName("Cody") || V == FName("May"))
			{
				// 1. 이쉐끼가 바라보는 방향이 플레이어의 전방벡터가 되어야함 →ㅇ
				// 2. Move가 입력중일땐 미적용-> Move가 끝난 시점에 적용되어야함(idle)
				// 3. Move중에도 벡터는 유지되고, 보는 방향은 달라져야함 →ㅇ
				// 4. 보는 방향 -> 피치,요 둘다 적용 →ㅇ
				// 5. 방향벡터 -> 요만 적용 →ㅇ

				CameraLookVector = _Instance.GetValue().Get<FVector2D>();

				if (IsFly)
				{
					AddControllerYawInput(CameraLookVector.X * 0.2f);
					//UE_LOG(LogTemp, Warning, TEXT("Look function called"));
					AddControllerPitchInput(-CameraLookVector.Y * 0.2f);
				}
				else
				{
					AddControllerYawInput(CameraLookVector.X * 0.7f);
					AddControllerPitchInput(-CameraLookVector.Y * 0.7f);
				}
			}
		}
	}
}

void APlayerBase::DashInput()
{
	IsSprint = false;
	if (HasAuthority() && !CodyHoldEnemy)
	{
		CustomClientDash();
	}
	else
	{
		CustomServerDash();
	}
}


void APlayerBase::CustomClientDash_Implementation()
{
	if (!bIsDashing && !bCanDash && BigCanDash && ChangeIdle)
	{
		ChangeState(Cody_State::DASH);
		//대쉬 시작시간을 체크
		DashStartTime = GetWorld()->GetTimeSeconds();
		//지면에 닿아있는지를 체크하여 실행할 함수 변경
		if (!GetCharacterMovement()->IsFalling())
		{
			GroundDash();
		}
		else
		{
			DashDuration = 0.2f;
			JumpDash();
		}
		bIsDashing = true;
		bCanDash = true;
	}
}
bool APlayerBase::CustomServerDash_Validate()
{
	return true;
}
void APlayerBase::CustomServerDash_Implementation()
{
	if (!bIsDashing && !bCanDash && BigCanDash && ChangeIdle)
	{
		ChangeState(Cody_State::DASH);
		//대쉬 시작시간을 체크
		DashStartTime = GetWorld()->GetTimeSeconds();
		//지면에 닿아있는지를 체크하여 실행할 함수 변경
		if (!GetCharacterMovement()->IsFalling())
		{
			GroundDash();
		}
		else
		{
			DashDuration = 0.2f;
			JumpDash();
		}
		bIsDashing = true;
		bCanDash = true;
	}
}

void APlayerBase::DashNoneInput()
{

}

void APlayerBase::GroundDash()
{
	// 마찰력 없앰
	GetCharacterMovement()->GroundFriction = 0.0f;
	// Cody의 전방벡터
	DashDirection = GetActorForwardVector();
	// 벡터Normalize
	DashDirection.Normalize();
	// 거리 x 방향 계산
	DashVelocity = DashDirection * DashDistance;
	// 시간에따른 속도설정
	GetCharacterMovement()->Velocity = DashVelocity;
}

void APlayerBase::JumpDash()
{
	// 중력 없앰
	GetCharacterMovement()->GravityScale = 0.0f;
	// Cody의 전방벡터
	DashDirection = GetActorForwardVector();
	// 방향벡터normalize
	DashDirection.Normalize();
	// 거리 x 방향 계산
	DashVelocity = DashDirection * DashDistance * 0.7f;
	// 시간에따른 속도설정
	GetCharacterMovement()->Velocity = DashVelocity;
}



void APlayerBase::InteractInput_Implementation()
{
	IsInteract = true;
	if (IsPlayerDeath)
	{
		PlayerHP += 1;
	}
}
void APlayerBase::InteractNoneInput_Implementation()
{
	IsInteract = false;
}

void APlayerBase::PlayerDeathCheck()
{
	if (IsPlayerDeath == false && 0 >= PlayerHP)
	{
		IsPlayerDeath = true;
	}
	if (IsPlayerDeath)
	{
		ChangeState(Cody_State::PlayerDeath);
		if (FullHP == PlayerHP)
		{
			IsPlayerDeath = false;
			ChangeState(Cody_State::IDLE);
		}
	}
}

void APlayerBase::PlayerToHomingRocketJumpStart()
{
	IsFly = true;
	if (HasAuthority())
	{
		CustomClientRideJump();
	}
	else
	{
		CustomServerRideJump();
	}
}


void APlayerBase::CustomClientRideJump_Implementation()
{
	Jump();
	JumplocationSet = true;
	ChangeState(Cody_State::FLYING);
	SpringArm->TargetArmLength = NormalLength;
	SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
}

bool APlayerBase::CustomServerRideJump_Validate()
{
	return true;
}
void APlayerBase::CustomServerRideJump_Implementation()
{
	Jump();
	JumplocationSet = true;
	ChangeState(Cody_State::FLYING);
	SpringArm->TargetArmLength = NormalLength;
	SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
}

void APlayerBase::PlayerToHomingRoketJumpFinished()
{
	ClientPlayerToHomingRoketJumpFinished();
	ServerPlayerToHomingRoketJumpFinished();
}

void APlayerBase::ClientPlayerToHomingRoketJumpFinished_Implementation()
{
	JumplocationSet = false; 
	JumpLocationDeltas = 0.0f;
}
bool APlayerBase::ServerPlayerToHomingRoketJumpFinished_Validate()
{
	return true;
}
void APlayerBase::ServerPlayerToHomingRoketJumpFinished_Implementation()
{
	JumplocationSet = false;
	JumpLocationDeltas = 0.0f;
}

void APlayerBase::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("HomingRocket"))
	{
		CunstomEndLocation = OtherComp->GetComponentLocation();
		CunstomStartLocation = GetActorLocation();
	}
}

void APlayerBase::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void APlayerBase::SpringArmDefaultFunction()
{
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->TargetArmLength = NormalLength;
	SpringArm->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = true;
}

void APlayerBase::SetTriggerActors(ARespawnTrigger* _Other)
{
	ResPawnTriggers = _Other;
	
}

void APlayerBase::SetRespawnPosition()
{
	if (HasAuthority())
	{
		ResPawnPosition = ResPawnTriggers->GetRespawnPosition();
	}
}

void APlayerBase::AttackPlayer(const int att)
 {
	AttackPlayerServer(att);
}

bool APlayerBase::AttackPlayerServer_Validate(const int att)
{
	return true;
}
void APlayerBase::AttackPlayerServer_Implementation(const int att)
{
	if (PlayerHP<=0)
	{
		return;
	}
	PlayerHP -= att;
}

void APlayerBase::AttackPlayerClient_Implementation(const int att)
{
	if (PlayerHP <= 0)
	{
		return;
	}
	PlayerHP -= att;
}

void APlayerBase::UpdateCamTrans()
{
	if (true == HasAuthority())
	{
		CurControllerLoc = PlayerCameraComponent->GetComponentLocation();
		CurControllerRot = PlayerCameraComponent->GetComponentRotation();
	}
}











void APlayerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APlayerBase, ITTPlayerState);
	DOREPLIFETIME(APlayerBase, IsMoveEnd);
	DOREPLIFETIME(APlayerBase, CurrentAnimationEnd);
	DOREPLIFETIME(APlayerBase, bCanDash);
	DOREPLIFETIME(APlayerBase, bIsDashingStart);
	DOREPLIFETIME(APlayerBase, IsSit);
	DOREPLIFETIME(APlayerBase, CanSit);
	DOREPLIFETIME(APlayerBase, CharacterJumpCount);
	DOREPLIFETIME(APlayerBase, IsBig);
	DOREPLIFETIME(APlayerBase, ChangeIdle);
	DOREPLIFETIME(APlayerBase, MoveDirection);
	DOREPLIFETIME(APlayerBase, MoveInput);
	DOREPLIFETIME(APlayerBase, ControllerRotation);
	DOREPLIFETIME(APlayerBase, CustomTargetRotation);
	DOREPLIFETIME(APlayerBase, WorldForwardVector);
	DOREPLIFETIME(APlayerBase, WorldRightVector);
	DOREPLIFETIME(APlayerBase, DashDirection);
	DOREPLIFETIME(APlayerBase, DashDistance);
	DOREPLIFETIME(APlayerBase, DashVelocity);
	DOREPLIFETIME(APlayerBase, bIsDashing);
	DOREPLIFETIME(APlayerBase, bIsDashingStart);
	DOREPLIFETIME(APlayerBase, bCanDash);
	DOREPLIFETIME(APlayerBase, DashDuration);
	DOREPLIFETIME(APlayerBase, DashStartTime);
	DOREPLIFETIME(APlayerBase, DashCurrentTime);
	DOREPLIFETIME(APlayerBase, CustomPlayerJumpCount);
	DOREPLIFETIME(APlayerBase, IsInteract);
	DOREPLIFETIME(APlayerBase, IsFly);
	DOREPLIFETIME(APlayerBase, FlyForwardVector);
	DOREPLIFETIME(APlayerBase, FlyingSpeed);
	DOREPLIFETIME(APlayerBase, CodyHoldEnemy);
	DOREPLIFETIME(APlayerBase, CunstomEndLocation);
	DOREPLIFETIME(APlayerBase, CunstomStartLocation);
	DOREPLIFETIME(APlayerBase, JumplocationSet);
	DOREPLIFETIME(APlayerBase, JumpLocationDeltas);
	DOREPLIFETIME(APlayerBase, CustomTargetLocations);
	DOREPLIFETIME(APlayerBase, CustomTargetLocationsY);
	DOREPLIFETIME(APlayerBase, ResultTargetLocations);
	DOREPLIFETIME(APlayerBase, IsSprint);
	DOREPLIFETIME(APlayerBase, IsPlayerDeath);
	DOREPLIFETIME(APlayerBase, PlayerHP);
	DOREPLIFETIME(APlayerBase, CurControllerRot);
	DOREPLIFETIME(APlayerBase, CurControllerLoc);
	DOREPLIFETIME(APlayerBase, PlayerDefaultSpeed);
	DOREPLIFETIME(APlayerBase, NowPlayerSpeed);
	DOREPLIFETIME(APlayerBase, CurCodySize);
	DOREPLIFETIME(APlayerBase, NextCodySize);
	DOREPLIFETIME(APlayerBase, NowPlayerGravityScale);
	DOREPLIFETIME(APlayerBase, PlayerJumpZVelocity);
	DOREPLIFETIME(APlayerBase, CurrentSitTime);
	DOREPLIFETIME(APlayerBase, SitStartTime);
	DOREPLIFETIME(APlayerBase, SitDuration);
}