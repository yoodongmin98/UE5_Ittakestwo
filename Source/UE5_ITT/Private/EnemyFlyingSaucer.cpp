// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFlyingSaucer.h"
#include "Engine/SkeletalMeshSocket.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SoundManageComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "FsmComponent.h"
#include "InteractionUIComponent.h"
#include "Misc/Paths.h"
#include "HomingRocket.h"
#include "EnemyMoonBaboon.h"
#include "ArcingProjectile.h"
#include "Floor.h"
#include "EnergyChargeEffect.h"
#include "FlyingSaucerAIController.h"
#include "PlayerBase.h"
#include "Cody.h"
#include "May.h"
#include "GroundPoundEffect.h"
#include "PhaseEndCameraRail.h"
#include "DrawDebugHelpers.h"
#include "CoreExplosionEffect.h"
#include "Misc/OutputDeviceNull.h"

// Sets default values
AEnemyFlyingSaucer::AEnemyFlyingSaucer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (true == HasAuthority())
	{
		SetupComponent();
		SetupFsmComponent();
		Tags.Add(FName("Boss"));

		bReplicates = true;
		SetReplicateMovement(true);
	}
}

void AEnemyFlyingSaucer::SetupComponent()
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	SetRootComponent(CapsuleComp);

	SkeletalMeshComp = GetMesh();
	SkeletalMeshComp->SetupAttachment(CapsuleComp);

	LaserSpawnPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserSpawnPointMesh"));
	LaserSpawnPointMesh->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::KeepRelativeTransform, TEXT("LaserSpawnPointSocket"));

	HomingRocketSpawnPointMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HomingRocketSpawnPointMesh1"));
	HomingRocketSpawnPointMesh1->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::KeepRelativeTransform, TEXT("HomingRocketSpawnPointSocket_01"));

	HomingRocketSpawnPointMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HomingRocketSpawnPointMesh2"));
	HomingRocketSpawnPointMesh2->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::KeepRelativeTransform, TEXT("HomingRocketSpawnPointSocket_02"));

	ArcingProjectileSpawnPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArcingProjectileSpawnPointMesh"));
	ArcingProjectileSpawnPointMesh->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::KeepRelativeTransform, TEXT("ArcingProjectileSpawnPointSocket"));

	RotatingComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));
	RotatingComp->RotationRate = FRotator(0.0f, 0.0f, 0.0f);

	PlayerOverlapCheckComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerOverlapCheckComponent"));
	PlayerOverlapCheckComp->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::KeepRelativeTransform, TEXT("PlayerOverlapCheckComponentSocket"));
	
	CodyOverlapCheckComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CodyOverlapCheckComponent"));
	CodyOverlapCheckComp->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::KeepRelativeTransform, TEXT("CodyOverlapCheckComponentSocket"));

	HatchOpenStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HatchOpenStaticMeshComponent"));
	HatchOpenStaticMeshComp->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Base"));
	HatchOpenStaticMeshComp->SetVisibility(false);

	SoundComp = CreateDefaultSubobject<USoundManageComponent>(TEXT("SoundManageComponent"));
	BGMComp = CreateDefaultSubobject<USoundManageComponent>(TEXT("BGMComponent"));
}

// Called when the game starts or when spawned
void AEnemyFlyingSaucer::BeginPlay()
{
	Super::BeginPlay();

	if (true == HasAuthority())
	{
		// 탑승한 원숭이 세팅
		EnemyMoonBaboon = GetWorld()->SpawnActor<AEnemyMoonBaboon>(EnemyMoonBaboonClass);
		EnemyMoonBaboon->SetOwner(this);
		EnemyMoonBaboon->GetMesh()->SetVisibility(false);
		
		// State 진입
		FsmComp->ChangeState(EBossState::None);
		
		// 오버랩이벤트 세팅
		SetupOverlapEvent();
		// 충돌이벤트 세팅
		SetupHitEvent();

		SoundComp->MulticastSetAttenuationDistance(12000.0f, 12000.0f);
		BGMComp->MulticastSetAttenuationDistance(12000.0f, 12000.0f);
	}
}

void AEnemyFlyingSaucer::AddPatternDestoryCount()
{
	// 패턴 파훼 카운트 추가
	++PatternDestroyCount;
	UBlackboardComponent* BlackboardComp = Cast<AFlyingSaucerAIController>(GetController())->GetBlackboardComponent();
	if (nullptr != BlackboardComp)
	{
		BlackboardComp->SetValueAsInt(TEXT("PatternDestroyCount"), PatternDestroyCount);
	}
}

const int32 AEnemyFlyingSaucer::GetCurrentState()
{
	if (nullptr == FsmComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("FsmComponent is Nullptr"));
		return -1;
	}

	return FsmComp->GetCurrentState();
}


void AEnemyFlyingSaucer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AEnemyFlyingSaucer, EnemyMoonBaboon);
	DOREPLIFETIME(AEnemyFlyingSaucer, FsmComp);
	DOREPLIFETIME(AEnemyFlyingSaucer, RotatingComp);
	DOREPLIFETIME(AEnemyFlyingSaucer, SkeletalMeshComp);
	DOREPLIFETIME(AEnemyFlyingSaucer, LaserSpawnPointMesh);
	DOREPLIFETIME(AEnemyFlyingSaucer, HomingRocketSpawnPointMesh1);
	DOREPLIFETIME(AEnemyFlyingSaucer, HomingRocketSpawnPointMesh2);
	DOREPLIFETIME(AEnemyFlyingSaucer, ArcingProjectileSpawnPointMesh);
	DOREPLIFETIME(AEnemyFlyingSaucer, AnimInstance);
	DOREPLIFETIME(AEnemyFlyingSaucer, AnimSequence);
	DOREPLIFETIME(AEnemyFlyingSaucer, EnergyChargeEffect);
	DOREPLIFETIME(AEnemyFlyingSaucer, bIsCodyHoldingEnter);
	DOREPLIFETIME(AEnemyFlyingSaucer, PrevLaserTargetLocation);
	DOREPLIFETIME(AEnemyFlyingSaucer, NextLaserTargetLocation);
	DOREPLIFETIME(AEnemyFlyingSaucer, CurrentLaserTargetLocation);
	DOREPLIFETIME(AEnemyFlyingSaucer, bPrevTargetLocationValid);
	DOREPLIFETIME(AEnemyFlyingSaucer, LaserLerpRatio);
	DOREPLIFETIME(AEnemyFlyingSaucer, LaserLerpRate);
	DOREPLIFETIME(AEnemyFlyingSaucer, LaserFireCount);
	DOREPLIFETIME(AEnemyFlyingSaucer, PatternDestroyCount);
	DOREPLIFETIME(AEnemyFlyingSaucer, ArcingProjectileFireTime);
	DOREPLIFETIME(AEnemyFlyingSaucer, ArcingProjectileMaxFireTime);
	DOREPLIFETIME(AEnemyFlyingSaucer, PlayerCody);
	DOREPLIFETIME(AEnemyFlyingSaucer, PlayerMay);
	DOREPLIFETIME(AEnemyFlyingSaucer, PrevAnimBoneLocation);
	DOREPLIFETIME(AEnemyFlyingSaucer, MaxHp);
	DOREPLIFETIME(AEnemyFlyingSaucer, CurrentHp);
	DOREPLIFETIME(AEnemyFlyingSaucer, bIsEject);
	DOREPLIFETIME(AEnemyFlyingSaucer, bIsRocketHit);
	DOREPLIFETIME(AEnemyFlyingSaucer, LaserLerpScale);
	DOREPLIFETIME(AEnemyFlyingSaucer, bIsCutSceneEnd);
	DOREPLIFETIME(AEnemyFlyingSaucer, bIsCutSceneStart);
	DOREPLIFETIME(AEnemyFlyingSaucer, HatchOpenStaticMeshComp);
	DOREPLIFETIME(AEnemyFlyingSaucer, PlayerOverlapCheckComp);
	DOREPLIFETIME(AEnemyFlyingSaucer, CodyOverlapCheckComp);
}



// 애니메이션 리소스 타입에 따라서 애니메이션 변경
void AEnemyFlyingSaucer::MulticastChangeAnimationFlyingSaucer_Implementation(const FString& AnimationPath, const uint8 AnimType, bool AnimationLoop)
{
	if (nullptr == SkeletalMeshComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComp is nullptr"));
		return;
	}

	EAnimationAssetType AnimationAssetType = static_cast<EAnimationAssetType>(AnimType);
	switch (AnimationAssetType)
	{
	case AEnemyFlyingSaucer::EAnimationAssetType::Sequence:
	{
		UAnimSequence* LoadedAnimationSequence = LoadObject<UAnimSequence>(nullptr, *AnimationPath);
		if (nullptr == LoadedAnimationSequence)
		{
			UE_LOG(LogTemp, Warning, TEXT("Animation Sequence is nullptr"));
			return;
		}

		SkeletalMeshComp->PlayAnimation(LoadedAnimationSequence, AnimationLoop);
	}
		break;

	case AEnemyFlyingSaucer::EAnimationAssetType::Blueprint:
	{
		UAnimBlueprint* LoadedAnimBlueprint = LoadObject<UAnimBlueprint>(nullptr, *AnimationPath);
		if (nullptr == LoadedAnimBlueprint)
		{
			UE_LOG(LogTemp, Warning, TEXT("Animation Sequence is nullptr"));
			return;
		}

		SkeletalMeshComp->SetAnimInstanceClass(LoadedAnimBlueprint->GeneratedClass);
	}
		break;
	}
}

void AEnemyFlyingSaucer::MulticastChangeAnimationMoonBaboon_Implementation(const FString& AnimationPath, const uint8 AnimType, bool AnimationLoop)
{
	if (nullptr == EnemyMoonBaboon)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyMoonBaboon is nullptr"));
		return;
	}

	EAnimationAssetType AnimationType = static_cast<EAnimationAssetType>(AnimType);
	switch (AnimationType)
	{
	case AEnemyFlyingSaucer::EAnimationAssetType::Sequence:
	{
		UAnimSequence* LoadedAnimationSequence = LoadObject<UAnimSequence>(nullptr, *AnimationPath);
		if (nullptr == LoadedAnimationSequence)
		{
			UE_LOG(LogTemp, Warning, TEXT("Animation Sequence is nullptr"));
			return;
		}

		EnemyMoonBaboon->GetMesh()->PlayAnimation(LoadedAnimationSequence, AnimationLoop);
	}
		break;
	case AEnemyFlyingSaucer::EAnimationAssetType::Blueprint:
	{
		UAnimBlueprint* LoadedAnimBlueprint = LoadObject<UAnimBlueprint>(nullptr, *AnimationPath);
		if (nullptr == LoadedAnimBlueprint)
		{
			UE_LOG(LogTemp, Warning, TEXT("AnimBlueprint is nullptr"));
			return;
		}

		EnemyMoonBaboon->GetMesh()->SetAnimInstanceClass(LoadedAnimBlueprint->GeneratedClass);
	}
		break;
	}
}

void AEnemyFlyingSaucer::MulticastCheckCodyKeyPressedAndChangeState_Implementation(const bool bIsInput)
{
	if (nullptr != PlayerCody)
	{
		CodySize Size = PlayerCody->GetCodySize();

		// 현재 인터랙트 키 눌렀는지 체크
		if (true == bIsInput && CodySize::BIG == Size)
		{
			FsmComp->ChangeState(EBossState::CodyHolding_Enter);
			bIsCodyHoldingEnter = true;
			return;
		}
	}
}

void AEnemyFlyingSaucer::MulticastSetActivateUIComponent_Implementation(UInteractionUIComponent* UIComponent, bool ParentUIActivate, bool ChildUIActivate)
{
	if (nullptr != UIComponent)
	{
		UIComponent->SetVisibility(ParentUIActivate, ChildUIActivate);
	}
}

void AEnemyFlyingSaucer::MulticastCreateEnergyChargeEffect_Implementation()
{
	EnergyChargeEffect = GetWorld()->SpawnActor<AEnergyChargeEffect>(EnergyChargeEffectClass, LaserSpawnPointMesh->GetComponentLocation(), FRotator::ZeroRotator);
	EnergyChargeEffect->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, TEXT("EnergyChargeEffectSpawnPointSocket"));
}

void AEnemyFlyingSaucer::MulticastSetFocusTarget_Implementation()
{
	AFlyingSaucerAIController* AIController = Cast<AFlyingSaucerAIController>(GetController());
	if (nullptr != AIController)
	{
		AIController->SetFocus(Cast<AActor>(LaserTargetActor));
	}
}

void AEnemyFlyingSaucer::MulticastAttachMoonBaboonActorWithFloor_Implementation()
{
	EnemyMoonBaboon->SetActorRelativeLocation(FVector::ZeroVector);
	EnemyMoonBaboon->GetMesh()->SetVisibility(true);
	EnemyMoonBaboon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("ChairSocket"));
	AttachToActor(FloorObject, FAttachmentTransformRules::KeepWorldTransform);
}

void AEnemyFlyingSaucer::CorrectCodyLocationAndRotation()
{
	if (true == bIsCodyHoldingLerpEnd)
	{
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	CodyLerpRatio += DeltaTime;
	if (CodyLerpRatio >= 1.0f)
	{
		CodyLerpRatio = 1.0f;
		bIsCodyHoldingLerpEnd = true;
		{
			// 1.0으로 고정된 위치 보정 해준 후 
			FVector StartLocation = PlayerCody->GetActorLocation();
			FVector EndLocation = FVector(CodyLerpEndLocation.X, CodyLerpEndLocation.Y, StartLocation.Z);
			FVector TargetLocation = FMath::Lerp(StartLocation, EndLocation, CodyLerpRatio);
			PlayerCody->SetActorLocation(TargetLocation);
		}
		{
			// 회전 변경 후 return
			FVector TargetLocation = GetActorLocation();
			TargetLocation.Z = 0.0f;
			FVector StartLocation = PlayerCody->GetActorLocation();
			StartLocation.Z = 0.0f;

			FRotator TargetRotation = (TargetLocation - StartLocation).Rotation();
			TargetRotation.Yaw -= 14.0f;
			PlayerCody->SetActorRotation(TargetRotation);
		}
		
		return;
	}

	FVector StartLocation = PlayerCody->GetActorLocation();
	FVector EndLocation = FVector(CodyLerpEndLocation.X, CodyLerpEndLocation.Y, StartLocation.Z);
	FVector TargetLocation = FMath::Lerp(StartLocation, EndLocation, CodyLerpRatio);
	PlayerCody->SetActorLocation(TargetLocation);
}


void AEnemyFlyingSaucer::MulticastHideLaserBaseBoneAndSpawnDestroyEffect_Implementation()
{
	// 레이저 본 렌더링 off
	int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(TEXT("LaserBase"));
	FVector EffectSpawnLocation = SkeletalMeshComp->GetBoneLocation(TEXT("LaserBase"));
	if (INDEX_NONE != BoneIndex)
	{
		SkeletalMeshComp->HideBone(BoneIndex, EPhysBodyOp::PBO_Term);
		AActor* SpawnEffect = GetWorld()->SpawnActor<ACoreExplosionEffect>(CoreExplosionEffectClass, EffectSpawnLocation, FRotator::ZeroRotator);
		SpawnEffect->SetActorRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	}
}



// Called every frame
void AEnemyFlyingSaucer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (true == HasAuthority())
	{
		if (true == bIsAllPhaseEnd)
		{
			return;
		}

		if (false == bIsAllPhaseEnd && true == bIsEject)
		{
			FsmComp->ChangeState(EBossState::Phase3_Eject);
			bIsAllPhaseEnd = true;
			return;
		}

		// 비헤이비어 트리가 동작중이 아니라면 호출하지 않음.
		if (false == bIsPhase1End)
		{
			UpdateLerpRatioForLaserBeam(DeltaTime);
		}
	}
}

void AEnemyFlyingSaucer::CorrectMayLocationAndRoation()
{
	PlayerMay->SetActorLocation(MayCorrectLocation);
	FVector LaserGunLocation = SkeletalMeshComp->GetBoneLocation(TEXT("LaserGunRing3"));
	FVector TargetLocation = LaserGunLocation - PlayerMay->GetActorLocation();
	TargetLocation.Z = 0.0f;
	FRotator TargetRotation = TargetLocation.Rotation();

	PlayerMay->SetActorRotation(FRotator(TargetRotation.Pitch, TargetRotation.Yaw, 0.0f));
}

void AEnemyFlyingSaucer::MulticastHideLaserBaseBone_Implementation()
{
	// 레이저 본 렌더링 off
	int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(TEXT("LaserBase"));
	FVector EffectSpawnLocation = SkeletalMeshComp->GetBoneLocation(TEXT("LaserBase"));
	if (INDEX_NONE != BoneIndex)
	{
		SkeletalMeshComp->HideBone(BoneIndex, EPhysBodyOp::PBO_Term);
	}
}

void AEnemyFlyingSaucer::SpawnCodyChaseHomingRocket()
{
	HomingRocketActor_1 = GetWorld()->SpawnActor<AHomingRocket>(HomingRocketClass);
	HomingRocketActor_1->SetTarget(Cast<AActor>(PlayerCody));
	HomingRocketActor_1->SetActorLocation(HomingRocketSpawnPointMesh1->GetComponentLocation());
	HomingRocketActor_1->SetOwner(this);
	
}

void AEnemyFlyingSaucer::SpawnMayChaseHomingRocket()
{
	HomingRocketActor_2 = GetWorld()->SpawnActor<AHomingRocket>(HomingRocketClass);
	HomingRocketActor_2->SetTarget(Cast<AActor>(PlayerMay));
	HomingRocketActor_2->SetActorLocation(HomingRocketSpawnPointMesh1->GetComponentLocation());
	HomingRocketActor_2->SetOwner(this);
}

void AEnemyFlyingSaucer::CodyChaseRocketSpawnCheck(float DeltaTime)
{
	if (nullptr == HomingRocketActor_1)
	{
		HomingRocket1FireTime -= DeltaTime;

		if (0.0f >= HomingRocket1FireTime && Cody_State::PlayerDeath != PlayerCody->GetITTPlayerState())
		{
			SpawnCodyChaseHomingRocket();
		}
	}

	if (nullptr != HomingRocketActor_1 && static_cast<int32>(AHomingRocket::ERocketState::Destroy) == HomingRocketActor_1->GetCurrentState())
	{
		HomingRocketActor_1->Destroy();
		HomingRocketActor_1 = nullptr;
		HomingRocket1FireTime = HomingRocketCoolTime;
	}
}

void AEnemyFlyingSaucer::MayChaseRocketSpawnCheck(float DeltaTime)
{
	if (nullptr == HomingRocketActor_2)
	{
		HomingRocket2FireTime -= DeltaTime;

		if (0.0f >= HomingRocket2FireTime && Cody_State::PlayerDeath != PlayerMay->GetITTPlayerState())
		{
			SpawnMayChaseHomingRocket();
		}
	}

	if (nullptr != HomingRocketActor_2 && static_cast<int32>(AHomingRocket::ERocketState::Destroy) == HomingRocketActor_2->GetCurrentState())
	{
		HomingRocketActor_2->Destroy();
		HomingRocketActor_2 = nullptr;
		HomingRocket2FireTime = HomingRocketCoolTime;
	}
}

void AEnemyFlyingSaucer::FireArcingProjectile()
{
	SoundComp->MulticastPlaySoundDirect(TEXT("LaserBomb_ShotFired_Cue"));

	AArcingProjectile* Projectile = GetWorld()->SpawnActor<AArcingProjectile>(ArcingProjectileClass, ArcingProjectileSpawnPointMesh->GetComponentLocation(), FRotator::ZeroRotator);
	Projectile->SetOwner(this);
	
	if (nullptr != Projectile)
	{
		FVector TargetLocation = FVector::ZeroVector;
		AActor* TargetActor = nullptr;
		
		if (0 == CurrentArcingProjectileTargetIndex)
		{
			TargetActor = PlayerCody;
			++CurrentArcingProjectileTargetIndex;
		}
		else
		{
			TargetActor = PlayerMay;
			CurrentArcingProjectileTargetIndex = 0;
		}

		// 타겟 위치세팅
		TargetLocation = TargetActor->GetActorLocation();
		Projectile->SetupTargetLocation(TargetLocation);
		Projectile->SetupProjectileMovementComponent();
		Projectile->AttachToActor(FloorObject, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AEnemyFlyingSaucer::MulticastCreateGroundPoundEffect_Implementation()
{
	FVector TargetLocation = GetActorLocation();
	TargetLocation.Z = TargetLocation.Z - 1100.0f;
	AGroundPoundEffect* GroundPoundEffect = GetWorld()->SpawnActor<AGroundPoundEffect>(GroundPoundEffectClass, TargetLocation, FRotator::ZeroRotator);
}

void AEnemyFlyingSaucer::DrawDebugMesh()
{
	FVector LaserSpawnPointMeshLocation = LaserSpawnPointMesh->GetComponentLocation();

	float SphereRadius = 50.0f;
	int32 Segments = 12;
	float LifeTime = 0.1f;
	float Thickness = 2.0f;

	DrawDebugSphere(
		GetWorld(),
		LaserSpawnPointMeshLocation,
		SphereRadius,
		Segments,
		FColor::Red,
		false,
		LifeTime,
		0,
		Thickness
		);

	FVector HomingRocketSpawnPointMesh1Locaiton = HomingRocketSpawnPointMesh1->GetComponentLocation();
	
	DrawDebugSphere(
		GetWorld(),
		HomingRocketSpawnPointMesh1Locaiton,
		SphereRadius,
		Segments,
		FColor::Blue,
		false,
		LifeTime,
		0,
		Thickness
	);

	FVector HomingRocketSpawnPointMesh2Locaiton = HomingRocketSpawnPointMesh2->GetComponentLocation();

	DrawDebugSphere(
		GetWorld(),
		HomingRocketSpawnPointMesh2Locaiton,
		SphereRadius,
		Segments,
		FColor::Blue,
		false,
		LifeTime,
		0,
		Thickness
	);

	FVector ArcingProjectileSpawnPointMeshLocaiton = ArcingProjectileSpawnPointMesh->GetComponentLocation();

	DrawDebugSphere(
		GetWorld(),
		ArcingProjectileSpawnPointMeshLocaiton,
		SphereRadius,
		Segments,
		FColor::Green,
		false,
		LifeTime,
		0,
		Thickness
	);
}

void AEnemyFlyingSaucer::UpdateLerpRatioForLaserBeam(float DeltaTime)
{
	LaserLerpRatio += DeltaTime * LaserLerpScale;
	if (1.0f <= LaserLerpRatio)
	{
		SavePreviousTargetLocation();
		LaserLerpRatio -= 1.0f;
		if (0.1f <= LaserLerpRatio)
		{
			LaserLerpRatio = 0.0f;
		}
	}
}

void AEnemyFlyingSaucer::SavePreviousTargetLocation()
{
	// 둘다 죽었을 경우는 고려 안함. 

	// 이전 레이저 타겟 액터가 존재할 경우 
	// 그럼 현재 대상은 타겟이 한번 변경된 대상인 거고. 
	if (nullptr != PrevLaserTargetActor)
	{
		// 이전 타겟이 데스상태가 아니라면 살아난거고. 
		if (Cody_State::PlayerDeath != PrevLaserTargetActor->GetITTPlayerState())
		{
			// 타겟을 다시 이전 타겟으로 세팅해주고 초기화.
			LaserTargetActor = PrevLaserTargetActor;
			PrevLaserTargetActor = nullptr;

			// 타겟 포커스. 
			MulticastSetFocusTarget();
		}
	}

	// 여기서 현재 타겟이 누구냐에 따라서 세팅될 수 있도록.
	if (nullptr != LaserTargetActor)
	{
		// 만약 현재 타겟액터가 데스 상태라면 
		if (Cody_State::PlayerDeath == LaserTargetActor->GetITTPlayerState())
		{
			/*Cody_State CodyPlayerState = LaserTargetActor->GetITTPlayerState();
			UE_LOG(LogTemp, Warning, TEXT("%d"), static_cast<int32>(CodyPlayerState));*/

			// 이전 레이저 타겟 액터에 저장
			PrevLaserTargetActor = LaserTargetActor;
			if (true == LaserTargetActor->ActorHasTag(TEXT("Cody")))
			{
				LaserTargetActor = PlayerMay;
				
			}
			else
			{
				LaserTargetActor = PlayerCody;
				
			}

			MulticastSetFocusTarget();
		}
		else
		{
			Cody_State CodyPlayerState = LaserTargetActor->GetITTPlayerState();
			
		}

		// 공격할 액터의 위치를 받아온다. 
		// 다음으로 공격할 위치일 거고. 
		CurrentLaserTargetLocation = LaserTargetActor->GetActorLocation();

		// 이전 타겟의 위치가 유효 하다면 ( 세팅 O ) 
		if (true == bPrevTargetLocationValid)
		{
			PrevLaserTargetLocation = NextLaserTargetLocation;
			NextLaserTargetLocation = CurrentLaserTargetLocation;
		}
		else
		{
			// 이전 타겟의 위치가 유효 하지않다면 ( 세팅 X )
			NextLaserTargetLocation = CurrentLaserTargetLocation;
			bPrevTargetLocationValid = true;
		}
	}
}

void AEnemyFlyingSaucer::SetupLaserTargetActor()
{
	switch (PatternDestroyCount)
	{
	case 0:
		if (nullptr != PlayerMay)
		{
			LaserTargetActor = PlayerMay;
		}
		break;
	case 1:
		if (nullptr != PlayerCody)
		{
			LaserTargetActor = PlayerCody;
		}
		break;
	case 2:
		if (nullptr != PlayerCody)
		{
			LaserTargetActor = PlayerMay;
		}
		break;
	}
}

void AEnemyFlyingSaucer::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (nullptr != OtherActor && true == OtherActor->ActorHasTag(TEXT("Player")))
	{
		// 로켓으로 cast 
		APlayerBase* PlayerBase = Cast<APlayerBase>(OtherActor);
		if (nullptr != PlayerBase)
		{
			int32 CurrentStateToInt = FsmComp->GetCurrentState();

			// 현재 그라운드 파운딩 스테이트일 때 충돌하면. 
			if (static_cast<int32>(EBossState::Phase3_GroundPounding) == CurrentStateToInt)
			{
				PlayerBase->AttackPlayer(GroundPoundDamage);
			}
		}
	}
}

void AEnemyFlyingSaucer::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AEnemyFlyingSaucer::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AEnemyFlyingSaucer::PlayerCheckComponentOnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (nullptr != OtherActor)
	{
		if (true == OtherActor->ActorHasTag(TEXT("Cody")))
		{
			bIsCodyOverlap = true;
		}

		if (true == OtherActor->ActorHasTag(TEXT("May")))
		{
			bIsMayOverlap = true;
		}
	}
}

void AEnemyFlyingSaucer::PlayerCheckComponentOnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (nullptr != OtherActor)
	{
		if (true == OtherActor->ActorHasTag(TEXT("Cody")))
		{
			bIsCodyOverlap = false;
		}

		if (true == OtherActor->ActorHasTag(TEXT("May")))
		{
			bIsMayOverlap = false;
		}
	}
}

void AEnemyFlyingSaucer::CodyCheckComponentOnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (nullptr != OtherActor)
	{
		if (true == OtherActor->ActorHasTag(TEXT("Cody")))
		{
			bIsCodyOverlap = true;
		}
	}
}

void AEnemyFlyingSaucer::CodyCheckComponentOnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (nullptr != OtherActor)
	{
		if (true == OtherActor->ActorHasTag(TEXT("Cody")))
		{
			bIsCodyOverlap = false;
		}
	}
}

void AEnemyFlyingSaucer::SetupHitEvent()
{
	SkeletalMeshComp->OnComponentHit.AddDynamic(this, &AEnemyFlyingSaucer::OnComponentHit);
}

void AEnemyFlyingSaucer::SetupOverlapEvent()
{
	if (nullptr != SkeletalMeshComp)
	{
		SkeletalMeshComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyFlyingSaucer::OnOverlapBegin);
		SkeletalMeshComp->OnComponentEndOverlap.AddDynamic(this, &AEnemyFlyingSaucer::OnOverlapEnd);

		PlayerOverlapCheckComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyFlyingSaucer::PlayerCheckComponentOnOverlapBegin);
		PlayerOverlapCheckComp->OnComponentEndOverlap.AddDynamic(this, &AEnemyFlyingSaucer::PlayerCheckComponentOnOverlapEnd);

		CodyOverlapCheckComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyFlyingSaucer::CodyCheckComponentOnOverlapBegin);
		CodyOverlapCheckComp->OnComponentEndOverlap.AddDynamic(this, &AEnemyFlyingSaucer::CodyCheckComponentOnOverlapEnd);
	}
}

void AEnemyFlyingSaucer::SetupPlayerActorsCodyAndMay()
{
	// 현재 월드의 액터를 받아오고 
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerBase::StaticClass(), PlayerActors);
	if (0 == PlayerActors.Num())
	{
		return;
	}

	// 순회해서 세팅해주는 걸로 변경
	for (AActor* Actor : PlayerActors)
	{
		if (true == Actor->ActorHasTag(TEXT("Cody")))
		{
			PlayerCody = Cast<ACody>(Actor);
			break;
		}
	}

	for (AActor* Actor : PlayerActors)
	{
		if (true == Actor->ActorHasTag(TEXT("May")))
		{
			PlayerMay = Cast<AMay>(Actor);
			break;
		}
	}
}

void AEnemyFlyingSaucer::MulticastSwapMesh_Implementation(bool bSkeletalMeshCompVisible, bool bStaticMeshCompVisible)
{
	if (false == bSkeletalMeshCompVisible)
	{
		SkeletalMeshComp->SetVisibility(false);
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HatchOpenStaticMeshComp->SetVisibility(true);
	}
	else
	{
		SkeletalMeshComp->SetVisibility(true);
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HatchOpenStaticMeshComp->SetVisibility(false);
	}
}

int32 AEnemyFlyingSaucer::GetFloorCurrentState()
{
	if (nullptr == FloorObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("Floor Object is nullptr"));
		return -1;
	}

	return FloorObject->GetCurrentPhase();
}

void AEnemyFlyingSaucer::MulticastEnableCutSceneCameraBlend_Implementation(APhaseEndCameraRail* CameraRail, const float BlendTime, const float BlendRatio)
{
	// AController* PlayerController = BlendTargetActor->GetController();
	AController* PlayerController = GetWorld()->GetFirstPlayerController();
	
	if (nullptr != PlayerController)
	{
		ViewTargetChangeController = Cast<APlayerController>(PlayerController);
		if (nullptr != ViewTargetChangeController)
		{
			// 컨트롤러의 현재 뷰타겟액터 저장
			PrevViewTarget = ViewTargetChangeController->GetViewTarget();
			if (nullptr != PrevViewTarget)
			{
				// 카메라 변경 후 재생비율 세팅
				ViewTargetChangeController->SetViewTargetWithBlend(Cast<AActor>(CameraRail), BlendTime);
				CameraRail->EnableCameraMove(BlendRatio);
			}
		}
	}

	bIsCutSceneStart = true;
}

void AEnemyFlyingSaucer::MulticastDisableCutSceneCameraBlend_Implementation(AActor* PrevViewTargetActor, const float BlendTime)
{
	if (nullptr == PrevViewTargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("PrevViewTargetActor is nullptr"));
		return;
	}

	if (nullptr == ViewTargetChangeController)
	{
		UE_LOG(LogTemp, Warning, TEXT("ViewTargetChangeController is nullptr"));
		return;
	}

	ViewTargetChangeController->SetViewTargetWithBlend(PrevViewTargetActor, BlendTime);
	ViewTargetChangeController = nullptr;
	bIsCutSceneStart = false;
}

void AEnemyFlyingSaucer::ActiveCodyUI(bool bNewVisibility, bool bPropagateToChildren)
{
	FOutputDeviceNull OutputDevice;
	FString FunctionString = TEXT("ActiveCodyInteractionUI");
	FString NewVisibilityToString;
	FString PropagateToChildrenToString;

	if (true == bNewVisibility)
	{
		NewVisibilityToString = TEXT(" true");
	}
	else
	{
		NewVisibilityToString = TEXT(" false");
	}

	if (true == bPropagateToChildren)
	{
		PropagateToChildrenToString = TEXT(" true");
	}
	else
	{
		PropagateToChildrenToString = TEXT(" false");
	}


	FunctionString += NewVisibilityToString;
	FunctionString += PropagateToChildrenToString;

	CallFunctionByNameWithArguments(*FunctionString, OutputDevice, nullptr, true);
}

void AEnemyFlyingSaucer::ActiveMayUI(bool bNewVisibility, bool bPropagateToChildren)
{
	FOutputDeviceNull OutputDevice;
	FString FunctionString = TEXT("ActiveMayInteractionUI");
	FString NewVisibilityToString;
	FString PropagateToChildrenToString;

	if (true == bNewVisibility)
	{
		NewVisibilityToString = TEXT(" true");
	}
	else
	{
		NewVisibilityToString = TEXT(" false");
	}

	if (true == bPropagateToChildren)
	{
		PropagateToChildrenToString = TEXT(" true");
	}
	else
	{
		PropagateToChildrenToString = TEXT(" false");
	}


	FunctionString += NewVisibilityToString;
	FunctionString += PropagateToChildrenToString;

	CallFunctionByNameWithArguments(*FunctionString, OutputDevice, nullptr, true);
}

void AEnemyFlyingSaucer::ActiveSplitScreen(bool bActive)
{
	FOutputDeviceNull OutputDevice;
	FString FunctionString = TEXT("ActiveSplitScreenMode");
	FString ActiveString;
	FString PropagateToChildrenToString;

	if (true == bActive)
	{
		ActiveString = TEXT(" true");
	}
	else
	{
		ActiveString = TEXT(" false");
	}

	FunctionString += ActiveString;
	CallFunctionByNameWithArguments(*FunctionString, OutputDevice, nullptr, true);
}



///////////////////////////////////FSM//////////////////////////////////////




void AEnemyFlyingSaucer::SetupFsmComponent()
{
	FsmComp = CreateDefaultSubobject<UFsmComponent>(TEXT("FsmComponent"));
	FsmComp->CreateState(EBossState::None,
		[this]
		{
			
		},

		[this](float DT)
		{
			if (true == bIsFsmStart)
			{
				ServerDelayTime -= DT;
				if (ServerDelayTime <= 0.0f)
				{
					FsmComp->ChangeState(EBossState::Phase1_LaserBeam_1);
					AFlyingSaucerAIController* AIController = Cast<AFlyingSaucerAIController>(GetController());
					AIController->GetBlackboardComponent()->SetValueAsBool(TEXT("bIsFsmStart"), true);

					// test code , 
					//AIController->GetCurrentBehaviorTree()->StopTree();
					//AIController->ClearFocus(EAIFocusPriority::Gameplay);
					return;
				}
			}
		},

		[this]
		{
			MulticastAttachMoonBaboonActorWithFloor();
		});

	FsmComp->CreateState(EBossState::Phase1_LaserBeam_1,
		[this]
		{
			BGMComp->MulticastChangeSound(TEXT("Phase1_BGM_Cue"));

			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Mh_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		},

		[this](float DT)
		{
			if (true == bIsDebugChangePhase)
			{
				// 레벨에서 디버그 키입력시 미사일발사스테이트로 변경
				AFlyingSaucerAIController* AIController = Cast<AFlyingSaucerAIController>(GetController());
				AIController->GetCurrentBehaviorTree()->StopTree();
				AIController->ClearFocus(EAIFocusPriority::Gameplay);
				FsmComp->ChangeState(EBossState::TestState);
				return;
			}

			if (1 == PatternDestroyCount)
			{
				FsmComp->ChangeState(EBossState::Phase1_LaserBeam_1_Destroy);
				return;
			}
		},

		[this]
		{

		});

	FsmComp->CreateState(EBossState::Phase1_LaserBeam_1_Destroy,
		[this]
		{
			SetDamage(CoreExplodeDamage);
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Laser_HitPod_Anim"), 1, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Laser_HitPod_Anim"), 1, false);
		},

		[this](float DT)
		{
			USkeletalMeshComponent* MoonBaboonMesh = EnemyMoonBaboon->GetMesh();
			if (false == SkeletalMeshComp->IsPlaying() && false == MoonBaboonMesh->IsPlaying())
			{
				FsmComp->ChangeState(EBossState::Phase1_ArcingProjectile_1);
				return;
			}
		},

		[this]
		{

		});

	FsmComp->CreateState(EBossState::Phase1_ArcingProjectile_1,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Mh_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		},

		[this](float DT)
		{
			// 바닥의 상태값에 따라서 changestate 
			int32 CurrentFloorPhaseToInt = static_cast<int32>(AFloor::Fsm::Phase1_2);
			if (CurrentFloorPhaseToInt == GetFloorCurrentState())
			{
				FsmComp->ChangeState(EBossState::Phase1_LaserBeam_2);
				return;
			}

			ArcingProjectileFireTime -= DT;
			if (ArcingProjectileFireTime <= 0.0f)
			{
				FireArcingProjectile();
				ArcingProjectileFireTime = ArcingProjectileMaxFireTime;
			}
		},

		[this]
		{
			ArcingProjectileFireTime = ArcingProjectileMaxFireTime;
		});

	FsmComp->CreateState(EBossState::Phase1_LaserBeam_2,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Mh_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		},

		[this](float DT)
		{
			if (2 == PatternDestroyCount)
			{
				FsmComp->ChangeState(EBossState::Phase1_LaserBeam_2_Destroy);
				return;
			}
		},

		[this]
		{
			
		});


	FsmComp->CreateState(EBossState::Phase1_LaserBeam_2_Destroy,
		[this]
		{
			SetDamage(CoreExplodeDamage);
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Laser_HitPod_Anim"), 1, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Laser_HitPod_Anim"), 1, false);
		},

		[this](float DT)
		{
			USkeletalMeshComponent* MoonBaboonMesh = EnemyMoonBaboon->GetMesh();
			if (false == SkeletalMeshComp->IsPlaying() && false == MoonBaboonMesh->IsPlaying())
			{
				FsmComp->ChangeState(EBossState::Phase1_ArcingProjectile_2);
				return;
			}
		},

		[this]
		{

		});


	FsmComp->CreateState(EBossState::Phase1_ArcingProjectile_2,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Mh_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		},

		[this](float DT)
		{
			int32 CurrentFloorPhaseToInt = static_cast<int32>(AFloor::Fsm::Phase1_3);
			if (CurrentFloorPhaseToInt == GetFloorCurrentState())
			{
				FsmComp->ChangeState(EBossState::Phase1_LaserBeam_3);
				return;
			}

			ArcingProjectileFireTime -= DT;
			if (ArcingProjectileFireTime <= 0.0f)
			{
				FireArcingProjectile();
				ArcingProjectileFireTime = ArcingProjectileMaxFireTime;
			}
		},

		[this]
		{
			ArcingProjectileFireTime = ArcingProjectileMaxFireTime;
		});


	FsmComp->CreateState(EBossState::Phase1_LaserBeam_3,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Mh_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		},

		[this](float DT)
		{
			if (3 == PatternDestroyCount)
			{
				// UE_LOG(LogTemp, Warning, TEXT("Change State : Phase1_BreakThePattern"));
				FsmComp->ChangeState(EBossState::Phase1_BreakThePattern);
				return;
			}
		},

		[this]
		{

		});

	FsmComp->CreateState(EBossState::Phase1_BreakThePattern,
		[this]
		{
			SetDamage(CoreExplodeDamage);
			BGMComp->MulticastChangeSound(TEXT("Phase2_BGM_Cue"));
			SoundComp->MulticastChangeSound(TEXT("PowerCoreDestroyed_Cue"));
			// 추락애니메이션 재생
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/CutScenes/PlayRoom_SpaceStation_BossFight_PowerCoresDestroyed_FlyingSaucer_Anim"), 1, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Programming_Anim"), 1, true);

			// 화면분할 해제
			ActiveSplitScreen(true);
			// 카메라블렌드 
			MulticastEnableCutSceneCameraBlend(PowerCoreDestroyCameraRail, 0.2f, 0.25f);
			// MulticastEnableCutSceneCameraBlend(PlayerMay, PowerCoreDestroyCameraRail, 0.2f, 0.25f);

			// 포커스 해제 및 비헤이비어트리 동작중지 
			AFlyingSaucerAIController* AIController = Cast<AFlyingSaucerAIController>(GetController());
			AIController->GetCurrentBehaviorTree()->StopTree();
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
			bIsPhase1End = true;
		},

		[this](float DT)
		{
			// 게임스테이트 카메라 반갈 온오프 함수 생기면 주석해제 
			if (true == PowerCoreDestroyCameraRail->IsMoveEnd())
			{
				if (nullptr != ViewTargetChangeController)
				{
					MulticastDisableCutSceneCameraBlend(PrevViewTarget, 0.2f);

					// 화면분할 적용 
					ActiveSplitScreen(false);
					// ResetFollowView();
					ActiveCodyUI(true, true);
				}
			}
			
			// 코디가 오버랩 상태일 때만 들어오고.
			if (true == bIsCodyOverlap)
			{
				bool bIsInteract = PlayerCody->GetIsInteract();
				if (true == bIsInteract)
				{
					// chage state : CodyHolding_Enter
					MulticastCheckCodyKeyPressedAndChangeState(bIsInteract);
				}
			}
		},

		[this]
		{
			
		});

	// 코디가 널부러져있는 보스 살짝 들기 수행
	FsmComp->CreateState(EBossState::CodyHolding_Enter,
		[this]
		{
			// 시작할때 한번 돌리고 끝나고 한번 더돌려 
			FVector TargetLocation = GetActorLocation();
			TargetLocation.Z = 0.0f;
			FVector StartLocation = PlayerCody->GetActorLocation();
			StartLocation.Z = 0.0f;

			FRotator TargetRotation = (TargetLocation - StartLocation).Rotation();
			TargetRotation.Yaw -= 14.0f;
			PlayerCody->SetActorRotation(TargetRotation);

			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_CodyHolding_Enter_Anim"), 1, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_CodyHolding_Enter_Anim"), 1, false);
		},

		[this](float DT)
		{
			// 둘다 애니메이션 종료됐으면, Low, 애니메이션 반복재생상태로 변경
			USkeletalMeshComponent* MoonBaboonMesh = EnemyMoonBaboon->GetMesh();
			if (false == SkeletalMeshComp->IsPlaying() && false == MoonBaboonMesh->IsPlaying() && true == bIsCodyHoldingLerpEnd)
			{
				FsmComp->ChangeState(EBossState::CodyHolding_Low);
				return;
			}

			CorrectCodyLocationAndRotation();
		},

		[this]
		{
			bIsCodyHoldingEnter = false;
			bIsCodyHoldingLerpEnd = false;
		});

	// 코디가 살짝 들고 있는 상태 
	FsmComp->CreateState(EBossState::CodyHolding_Low,
		[this]
		{
			// UFO 각도변경 애니메이션 
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_CodyHolding_Low_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_CodyHolding_Anim"), 1, true);
		},

		[this](float DT)
		{
			bool IsPressed = PlayerCody->GetIsInteract();
			if (true == IsPressed)
			{
				FsmComp->ChangeState(EBossState::CodyHolding_ChangeOfAngle);
				return;
			}
		},

		[this]
		{
			
		});


	FsmComp->CreateState(EBossState::CodyHolding_ChangeOfAngle,
		[this]
		{
			// ufo 각도 변경 애니메이션 재생
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/BluePrints/ABP_EnemyFlyingSaucer_CodyHoldingRotation"), 2, false);
			StateCompleteTime = 2.5f;
		},

		[this](float DT)
		{
			if (StateCompleteTime <= FsmComp->GetStateLiveTime())
			{
				FsmComp->ChangeState(EBossState::CodyHolding_InputKey);
				StateCompleteTime = 0.0f;
				return;
			}
		},

		[this]
		{
			
		});

	FsmComp->CreateState(EBossState::CodyHolding_InputKey,
		[this]
		{
			// ufo 각도 변경 애니메이션 재생
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_CodyHolding_Anim"), 1, true);
			bIsKeyInput = true;
			KeyInputTime = KeyInputMaxTime;

			// 여기들어왔을때 메이 UI On
			ActiveMayUI(true, true);
		},

		[this](float DT)
		{
			// 시간내에 키가 안눌렸을 경우 우주선을 내려놓음
			if (KeyInputTime <= 0.0f)
			{
				FsmComp->ChangeState(EBossState::CodyHolding_ChangeOfAngle_Reverse);
				return;
			}
				
			// 키체크 
			bool bCodyInputCheck = PlayerCody->GetIsInteract();
			if (true == bCodyInputCheck)
			{
				KeyInputTime = KeyInputAdditionalTime;
			}
			
			// 메이가 오버랩 상태일때만 들어옴
			if (true == bIsMayOverlap)
			{
				// 키입력 체크
				bool bMayInputCheck = PlayerMay->GetIsInteract();
				if (true == bMayInputCheck)
				{
					FsmComp->ChangeState(EBossState::Phase1_ChangePhase_2);
					return;
				}
			}

			KeyInputTime -= DT;
		},

		[this]
		{
			bIsKeyInput = false;
			KeyInputTime = KeyInputMaxTime;
			
			ActiveMayUI(false, true);
			
			PrevAnimBoneLocation = SkeletalMeshComp->GetBoneLocation(TEXT("Base"));
		});

	FsmComp->CreateState(EBossState::CodyHolding_ChangeOfAngle_Reverse,
		[this]
		{
			// 고각도 -> 저각도 애니메이션 
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/BluePrints/ABP_EnemyFlyingSaucer_CodyHoldingRotationReverse"), 2, false);
			StateCompleteTime = 2.0f;
		},

		[this](float DT)
		{
			if (StateCompleteTime <= FsmComp->GetStateLiveTime())
			{
				FsmComp->ChangeState(EBossState::CodyHolding_Low);
				StateCompleteTime = 0.0f;
				return;
			}
		},

		[this]
		{
			
		});

	
	FsmComp->CreateState(EBossState::Phase1_ChangePhase_2,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/PlayRoom_SpaceStation_BossFight_LaserRippedOff_FlyingSaucer_Anim"), 1, false);
			PlayerCody->CutScenceStart();
			PlayerMay->CutSceneStart();
			CorrectMayLocationAndRoation();

			ActiveSplitScreen(true);
			ActiveCodyUI(false, true);
			MulticastEnableCutSceneCameraBlend(LaserDestroyCameraRail, 0.2f, 0.23f);
		},

		[this](float DT)
		{
			// 애니메이션이 종료 되었을 때 
			if (false == SkeletalMeshComp->IsPlaying())
			{
				MulticastHideLaserBaseBoneAndSpawnDestroyEffect();
				FsmComp->ChangeState(EBossState::Phase2_RotateSetting);
				PrevAnimBoneLocation = SkeletalMeshComp->GetBoneLocation(TEXT("Root"));
				return;
			}

			if (false == bIsCorretLocation)
			{
				SetActorLocation(PrevAnimBoneLocation);
				bIsCorretLocation = true;
			}
		},

		[this]
		{
			bIsCorretLocation = false;

			// 여기서 컴포넌트 디스트로이 오버랩 컴포넌트 비활성
			PlayerOverlapCheckComp->SetVisibility(false);

			MulticastDisableCutSceneCameraBlend(PrevViewTarget, 0.2f);
			ActiveSplitScreen(false);
		});

	
	FsmComp->CreateState(EBossState::Phase2_RotateSetting,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Left_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		},

		[this](float DT)
		{
			// 보스 위치보정 적용안되어 있으면 적용하고. 로테이팅컴포넌트 피봇 세팅 
			if (false == bIsCorretLocation)
			{
				SetActorLocation(PrevAnimBoneLocation);
				bIsCorretLocation = true;
			}

			FsmComp->ChangeState(EBossState::Phase2_Rotating);
		},

		[this]
		{
			// 타겟의 위치를 받아오고. 
			FVector TargetLocation = FloorObject->GetActorLocation() - GetActorLocation();
			TargetLocation = TargetLocation.RotateAngleAxis(-(GetActorRotation().Yaw), FVector::UpVector);
			RotatingComp->PivotTranslation = TargetLocation;
			RotatingComp->RotationRate = FRotator(0.0f, 7.0f, 0.0f);
			bIsCorretLocation = false;
		});

	FsmComp->CreateState(EBossState::Phase2_Rotating,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Left_Anim"), 1, true);

			SpawnCodyChaseHomingRocket();
			SpawnMayChaseHomingRocket();
		},

		[this](float DT)
		{
			if (CurrentHp <= 33.0f)
			{
				FsmComp->ChangeState(EBossState::Phase2_BreakThePattern);
				return;
			}

			if (true == bIsRocketHit)
			{
				FsmComp->ChangeState(EBossState::Phase2_RocketHit);
				return;
			}

			if (CurrentHp > 33.0f)
			{
				CodyChaseRocketSpawnCheck(DT);
				MayChaseRocketSpawnCheck(DT);
			}
		},

		[this]
		{
			HomingRocket1FireTime = HomingRocketCoolTime;
			HomingRocket2FireTime = HomingRocketCoolTime;
		});

	FsmComp->CreateState(EBossState::Phase2_RocketHit,
		[this]
		{
			// 우주선 히트 애니메이션 적용 
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Laser_HitPod_Anim"), 1, false);
		},

		[this](float DT)
		{
			// 애니메이션 종료시 Phase2_Rotating으로 상태변경
			if (false == SkeletalMeshComp->IsPlaying())
			{
				FsmComp->ChangeState(EBossState::Phase2_Rotating);
				return;
			}
		},

		[this]
		{
			bIsRocketHit = false;
		});



	FsmComp->CreateState(EBossState::Phase2_BreakThePattern,
		[this]
		{
			SoundComp->MulticastChangeSound(TEXT("RocketsPhaseFinished_Cue"));
			RotatingComp->RotationRate = FRotator(0.0f, 0.0f, 0.0f);
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/BluePrints/ABP_EnemyFlyingSaucer_RocketPhaseEnd"), 2, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/CutScenes/PlayRoom_SpaceStation_BossFight_RocketsPhaseFinished_MoonBaboon_Anim"), 1, false);
			
			// 화면분할 해제
			ActiveSplitScreen(true);
			// 카메라블렌드 
			MulticastEnableCutSceneCameraBlend(BossFallCameraRail, 0.2f, 0.25f);
			
		},

		[this](float DT)
		{
			if (7.5f <= FsmComp->GetStateLiveTime())
			{
				FsmComp->ChangeState(EBossState::Phase2_ChangePhase_Wait);
				return;
			}
		},

		[this]
		{
			MulticastDisableCutSceneCameraBlend(PrevViewTarget, 0.3f);
			ActiveSplitScreen(false);
		});

	FsmComp->CreateState(EBossState::Phase2_ChangePhase_Wait,
		[this]
		{
			MulticastSwapMesh(false, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_KnockDownMh_Anim"), 1, true);
		},

		[this](float DT)
		{
			if (true == bIsCodyOverlap)
			{
				FsmComp->ChangeState(EBossState::Phase2_Fly);
				return;
			}
		},

		[this]
		{
		});	

	FsmComp->CreateState(EBossState::Phase2_Fly,
		[this]
		{
			MulticastSwapMesh(true, false);
			MulticastHideLaserBaseBoneAndSpawnDestroyEffect();

			PlayerCody->TeleportTo(CodyUfoInsideLocation, CodyUfoInsideRotation);


			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/CutScenes/PlayRoom_SpaceStation_BossFight_EnterUFO_FlyingSaucer_Anim"), 1, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/CutScenes/PlayRoom_SpaceStation_BossFight_EnterUFO_MoonBaboon_Anim"), 1, false);
		},

		[this](float DT)
		{
			// 우주선 애니메이션 재생 완료시 중앙으로 이동
			if (false == SkeletalMeshComp->IsPlaying())
			{
				FsmComp->ChangeState(EBossState::Phase2_MoveToCenter);
				return;
			}
		},

		[this]
		{

		});

	FsmComp->CreateState(EBossState::Phase2_MoveToCenter,
		[this]
		{
			// 우주선 포워드이동 애니메이션 적용, 
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Fwd_Anim"), 1, false);
			MoveStartLocation = GetActorLocation();
		},

		[this](float DT)
		{
			MoveToCenterLerpRatio += DT / 2.0f;
			if (1.0f <= MoveToCenterLerpRatio)
			{
				MoveToCenterLerpRatio = 1.0f;
				FVector TargetLocation = FMath::Lerp(MoveStartLocation, FVector(0.0f, 0.0f, MoveStartLocation.Z), MoveToCenterLerpRatio);
				(TargetLocation); // ???? 뭐지이건 
				FsmComp->ChangeState(EBossState::Phase3_MoveFloor);
				FloorObject->SetPhase(AFloor::Fsm::Phase3);
				return;
			}
			
			FVector TargetLocation = FMath::Lerp(MoveStartLocation, FVector(0.0f, 0.0f, MoveStartLocation.Z), MoveToCenterLerpRatio);
			SetActorLocation(TargetLocation);
		},

		[this]
		{
			MoveToCenterLerpRatio = 0.0f;
			MoveStartLocation = FVector::ZeroVector;
			// 중앙으로 이동 종료 되었을 때 원숭이 루프애니메이션 적용
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		});

	FsmComp->CreateState(EBossState::Phase3_MoveFloor,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Mh_Anim"), 1, true);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		},
		
		[this](float DT)
		{
			if (static_cast<int32>(AFloor::Fsm::KeepPhase) == FloorObject->GetCurrentPhase())
			{
				FsmComp->ChangeState(EBossState::Phase3_MoveToTarget);
			}

		},

		[this]
		{
		});

	FsmComp->CreateState(EBossState::Phase3_MoveToTarget,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_Fwd_Anim"), 1, false);
			MoveStartLocation = GetActorLocation();
			GroundPoundTargetLocation = PlayerMay->GetActorLocation();
			GroundPoundTargetLocation.Z = MoveStartLocation.Z;

			AController* BossController = GetController();
			if (nullptr == BossController)
			{
				UE_LOG(LogTemp, Warning, TEXT("Boss Controller is nullptr"));
				return;
			}

			AFlyingSaucerAIController* Controller = Cast<AFlyingSaucerAIController>(GetController());
			if (nullptr != Controller)
			{
				Controller->SetFocus(Cast<AActor>(PlayerMay));
			}
		},

		[this](float DT)
		{
			if (Cody_State::PlayerDeath == PlayerMay->GetITTPlayerState())
			{
				return;
			}

			MoveToTargetLerpRatio += DT;
			if (1.0f <= MoveToTargetLerpRatio)
			{
				MoveToTargetLerpRatio = 1.0f;
				FVector TargetLocation = FMath::Lerp(MoveStartLocation, GroundPoundTargetLocation, MoveToTargetLerpRatio);
				SetActorLocation(TargetLocation);
				FsmComp->ChangeState(EBossState::Phase3_GroundPounding);
				return;
			}

			FVector TargetLocation = FMath::Lerp(MoveStartLocation, GroundPoundTargetLocation, MoveToTargetLerpRatio);
			SetActorLocation(TargetLocation);
		},

		[this]
		{
			MoveToTargetLerpRatio = 0.0f;
			MoveStartLocation = FVector::ZeroVector;
		});

	FsmComp->CreateState(EBossState::Phase3_GroundPounding,
		[this]
		{
			// 그라운드파운딩 애니메이션 
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/Animations/FlyingSaucer_Ufo_GroundPound_Anim"), 1, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_GroundPound_Anim"), 1, false);
		},

		[this](float DT)
		{
			if (false == SkeletalMeshComp->IsPlaying())
			{
				FsmComp->ChangeState(EBossState::Phase3_MoveToTarget);
				return;
			}

			if (GroundPoundEffectCreateTime <= FsmComp->GetStateLiveTime() && false == bIsSetGroundPoundEffect)
			{
				MulticastCreateGroundPoundEffect();
				bIsSetGroundPoundEffect = true;
			}

		},

		[this]
		{
			bIsSetGroundPoundEffect = false;
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Mh_Anim"), 1, true);
		});

	FsmComp->CreateState(EBossState::Phase3_Eject,
		[this]
		{
			MulticastChangeAnimationFlyingSaucer(TEXT("/Game/Characters/EnemyFlyingSaucer/CutScenes/PlayRoom_SpaceStation_BossFight_Eject_FlyingSaucer_Anim"), 1, false);
			MulticastChangeAnimationMoonBaboon(TEXT("/Game/Characters/EnemyMoonBaboon/Animations/MoonBaboon_Ufo_Programming_Anim.MoonBaboon_Ufo_Programming_Anim"), 1, false);
			
			// 남은체력만큼 데미지 
			SetDamage(GetCurrentHp());

			ActiveSplitScreen(true);
			MulticastEnableCutSceneCameraBlend(BossEjectCameraRail, 0.2f, 0.35f);
		},

		[this](float DT)
		{
			// 애니메이션 재생 완료시 페이즈 종료 상태로 전환, 아무작업도 수행 X 
			// 여기서 되돌리지말고 그냥 End; 
			if (6.0f <= FsmComp->GetStateLiveTime())
			{
				FsmComp->ChangeState(EBossState::AllPhaseEnd);
				return;
			}
		},

		[this]
		{
			//MulticastDisableCutSceneCameraBlend(PrevViewTarget, 0.2f);
			//ActiveSplitScreen(false);
		});

	FsmComp->CreateState(EBossState::AllPhaseEnd,
		[this]
		{
		},

		[this](float DT)
		{
		},

		[this]
		{
			
		});


	// TEST 전용 state 
	FsmComp->CreateState(EBossState::TestState,
		[this]
		{
		},

		[this](float DT)
		{
			if (true == bIsDebugChangePhase)
			{
				// 레벨에서 디버그 키입력시 미사일발사스테이트로 변경
				AFlyingSaucerAIController* AIController = Cast<AFlyingSaucerAIController>(GetController());
				
				FsmComp->ChangeState(EBossState::Phase2_BreakThePattern);
				return;
			}

		
			CodyChaseRocketSpawnCheck(DT);
			MayChaseRocketSpawnCheck(DT);
			
		},

		[this]
		{
		});

	// TEST 전용 state 
	
}


