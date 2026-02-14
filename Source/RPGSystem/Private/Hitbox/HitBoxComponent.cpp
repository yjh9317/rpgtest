// Fill out your copyright notice in the Description page of Project Settings.


#include "Hitbox/HitBoxComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Combat/CombatData.h"
#include "Combat/Combatable.h"
#include "Combat/Components/CombatComponentBase.h"


UHitBoxComponent::UHitBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 기본적으로 비활성화

	// 기본 바디파트 데미지 배율 설정
	BodyPartDamageMultipliers.Add(TEXT("Head"), 2.0f);
	BodyPartDamageMultipliers.Add(TEXT("Chest"), 1.0f);
	BodyPartDamageMultipliers.Add(TEXT("Arm"), 0.8f);
	BodyPartDamageMultipliers.Add(TEXT("Leg"), 0.6f);
}

void UHitBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	// Owner 캐릭터 캐싱
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerMesh = OwnerCharacter->GetMesh();
	}

	// 쿼리 파라미터 초기화
	UpdateQueryParams();

	// 초기 상태 설정
	SetHitBoxState(EHitBoxState::Inactive);
}

void UHitBoxComponent::BeginDestroy()
{
	// 델리게이트 정리
	OnHitBoxActivated.Clear();
	OnHitBoxDeactivated.Clear();
	OnHitBoxHit.Clear();
	OnHitBoxStateChanged.Clear();

	// 타이머 정리
	if (GetWorld())
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.ClearTimer(MultiHitTimer);
		TimerManager.ClearTimer(CooldownTimer);
	}

	Super::BeginDestroy();
}

void UHitBoxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 활성 상태일 때만 트레이스 수행
	if (CurrentState == EHitBoxState::Active)
	{
		PerformTrace();
	}
}

#pragma region Control_Functions

void UHitBoxComponent::ActivateHitBox()
{
	if (CurrentState != EHitBoxState::Inactive)
	{
		return;
	}

	SetHitBoxState(EHitBoxState::Active);

	// 히트 기록 초기화
	if (!bAllowMultiHit)
	{
		ClearHitActors();
	}
	CurrentHitCount = 0;

	// 틱 활성화
	SetComponentTickEnabled(true);

	// 트레이스 위치 캐시
	CacheTracePositions();

	// 델리게이트 브로드캐스트
	OnHitBoxActivated.Broadcast(this);
}

void UHitBoxComponent::DeactivateHitBox()
{
	if (CurrentState == EHitBoxState::Inactive)
	{
		return;
	}

	SetHitBoxState(EHitBoxState::Inactive);

	// 틱 비활성화
	SetComponentTickEnabled(false);

	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MultiHitTimer);
	}

	// 델리게이트 브로드캐스트
	OnHitBoxDeactivated.Broadcast(this);
}

void UHitBoxComponent::ResetCombo()
{
	CurrentComboCount = 0;
	ClearHitActors();
	CurrentHitCount = 0;
	SetHitBoxState(EHitBoxState::Inactive);
}

void UHitBoxComponent::SetDamageMultiplier(float Multiplier)
{
	CurrentDamageMultiplier = FMath::Max(0.0f, Multiplier);
}

void UHitBoxComponent::ClearHitActors()
{
	HitActors.Empty();
	LastHitTimes.Empty();
}

#pragma endregion

#pragma region Trace_Functions

void UHitBoxComponent::PerformTrace()
{
	if (!OwnerMesh || HitBoxSettings.TraceSocketNames.Num() == 0)
	{
		return;
	}

	TArray<FHitResult> HitResults;
	bool bHitDetected = false;

	// 소켓별로 트레이스 수행
	for (int32 i = 0; i < HitBoxSettings.TraceSocketNames.Num() - 1; ++i)
	{
		FName StartSocket = HitBoxSettings.TraceSocketNames[i];
		FName EndSocket = HitBoxSettings.TraceSocketNames[i + 1];

		if (!OwnerMesh->DoesSocketExist(StartSocket) || !OwnerMesh->DoesSocketExist(EndSocket))
		{
			continue;
		}

		FVector StartLocation = OwnerMesh->GetSocketLocation(StartSocket);
		FVector EndLocation = OwnerMesh->GetSocketLocation(EndSocket);

		TArray<FHitResult> CurrentHits;
		bool bCurrentHit = false;

		// 트레이스 타입에 따른 처리
		switch (HitBoxSettings.TraceType)
		{
		case ETraceType::LineTrace:
			bCurrentHit = PerformLineTrace(StartLocation, EndLocation, CurrentHits);
			break;
		case ETraceType::SphereTrace:
			bCurrentHit = PerformSphereTrace(StartLocation, EndLocation, CurrentHits);
			break;
		case ETraceType::BoxTrace:
			bCurrentHit = PerformBoxTrace(StartLocation, EndLocation, CurrentHits);
			break;
		case ETraceType::CapsuleTrace:
			bCurrentHit = PerformCapsuleTrace(StartLocation, EndLocation, CurrentHits);
			break;
		}

		if (bCurrentHit)
		{
			bHitDetected = true;
			HitResults.Append(CurrentHits);
		}
	}

	// 히트 결과 처리
	for (const FHitResult& Hit : HitResults)
	{
		ProcessHit(Hit);
	}
}

bool UHitBoxComponent::PerformLineTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits)
{
	return GetWorld()->LineTraceMultiByChannel(
		OutHits,
		Start,
		End,
		HitBoxSettings.TraceChannel,
		CachedQueryParams
	);
}

bool UHitBoxComponent::PerformSphereTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits)
{
	return GetWorld()->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		HitBoxSettings.TraceChannel,
		FCollisionShape::MakeSphere(HitBoxSettings.TraceRadius),
		CachedQueryParams
	);
}

bool UHitBoxComponent::PerformBoxTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits)
{
	return GetWorld()->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		HitBoxSettings.TraceChannel,
		FCollisionShape::MakeBox(HitBoxSettings.BoxExtent),
		CachedQueryParams
	);
}

bool UHitBoxComponent::PerformCapsuleTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits)
{
	return GetWorld()->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		HitBoxSettings.TraceChannel,
		FCollisionShape::MakeCapsule(HitBoxSettings.CapsuleRadius, HitBoxSettings.CapsuleHalfHeight),
		CachedQueryParams
	);
}

FHitResult UHitBoxComponent::MainTrace(const TArray<FName>& SocketsName,ETraceType TraceTypeOverride)
{
	FHitResult OutHit;
	
	if (!OwnerMesh)
	{
		return OutHit;
	}

	const TArray<FName>& SocketNames = SocketsName.Num() > 0 ? SocketsName : HitBoxSettings.TraceSocketNames;
	
	if (SocketNames.Num() < 2)
	{
		return OutHit;
	}

	FVector StartLocation = OwnerMesh->GetSocketLocation(SocketNames[0]);
	FVector EndLocation = OwnerMesh->GetSocketLocation(SocketNames[1]);

	TArray<FHitResult> HitResults;
	bool bHit = false;

	switch (TraceTypeOverride)
	{
	case ETraceType::LineTrace:
		bHit = PerformLineTrace(StartLocation, EndLocation, HitResults);
		break;
	case ETraceType::SphereTrace:
		bHit = PerformSphereTrace(StartLocation, EndLocation, HitResults);
		break;
	case ETraceType::BoxTrace:
		bHit = PerformBoxTrace(StartLocation, EndLocation, HitResults);
		break;
	case ETraceType::CapsuleTrace:
		bHit = PerformCapsuleTrace(StartLocation, EndLocation, HitResults);
		break;
	}

	if (bHit && HitResults.Num() > 0)
	{
		OutHit = HitResults[0];
	}

	return OutHit;
}

#pragma endregion

#pragma region Hit_Processing

void UHitBoxComponent::ProcessHit(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (!CanHitActor(HitActor))
	{
		return;
	}

	// 다단히트 체크
	if (!bAllowMultiHit && HitActors.Contains(HitActor))
	{
		return;
	}

	// 다단히트 딜레이 체크
	if (bAllowMultiHit)
	{
		const float* LastHitTime = LastHitTimes.Find(HitActor);
		if (LastHitTime && GetWorld()->GetTimeSeconds() - *LastHitTime < MultiHitDelay)
		{
			return;
		}
	}

	// 최대 히트 수 체크
	if (CurrentHitCount >= MaxHitsPerActivation)
	{
		return;
	}

	// 데미지 계산
	bool bWasCritical = false;
	float FinalDamage = CalculateFinalDamage(Hit, bWasCritical);

	// 히트 이벤트 생성
	FHitBoxDamageEvent HitEvent;
	HitEvent.HitActor = HitActor;
	HitEvent.HitLocation = Hit.Location;
	HitEvent.HitNormal = Hit.Normal;
	HitEvent.BoneName = Hit.BoneName;
	HitEvent.FinalDamage = FinalDamage;
	HitEvent.bWasCritical = bWasCritical;
	HitEvent.DamageType = DamageType;
	HitEvent.DamageMultiplier = CurrentDamageMultiplier;
	HitEvent.ComboIndex = CurrentComboCount;

	// 히트 기록 업데이트
	if (!HitActors.Contains(HitActor))
	{
		HitActors.Add(HitActor);
	}
	LastHitTimes.Add(HitActor, GetWorld()->GetTimeSeconds());
	CurrentHitCount++;

	// 데미지 적용
	ApplyDamageToActor(HitActor, Hit, FinalDamage, bWasCritical);

	// 이펙트 재생
	PlayHitEffects(Hit.Location, Hit.Normal);

	// 물리 효과 적용
	if (bApplyKnockback)
	{
		FVector KnockbackDirection = (Hit.Location - OwnerCharacter->GetActorLocation()).GetSafeNormal();
		ApplyKnockback(HitActor, KnockbackDirection);
	}

	// 히트스톱 적용
	if (HitStopDuration > 0.0f)
	{
		ApplyHitStop();
	}

	// 델리게이트 브로드캐스트
	OnHitBoxHit.Broadcast(HitEvent);

	// 최대 히트 수에 도달하면 비활성화
	if (CurrentHitCount >= MaxHitsPerActivation)
	{
		DeactivateHitBox();
	}
}

bool UHitBoxComponent::CanHitActor(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// Owner 무시
	if (HitBoxSettings.bIgnoreOwner && Actor == GetOwner())
	{
		return false;
	}

	// 무시 목록 체크
	if (IgnoreActors.Contains(Actor))
	{
		return false;
	}

	// 클래스 필터 체크
	if (TargetClasses.Num() > 0)
	{
		bool bValidClass = false;
		for (const TSubclassOf<AActor>& TargetClass : TargetClasses)
		{
			if (Actor->IsA(TargetClass))
			{
				bValidClass = true;
				break;
			}
		}
		if (!bValidClass)
		{
			return false;
		}
	}

	// 태그 필터 체크 (BaseCharacter인 경우)
	// TODO: 태그 시스템 연동 시 RequiredTags/IgnoreTags 실제 비교 구현

	return true;
}

float UHitBoxComponent::CalculateFinalDamage(const FHitResult& Hit, bool& bOutWasCritical)
{
	float FinalDamage = BaseDamage * CurrentDamageMultiplier;

	// 바디파트 배율 적용
	if (Hit.BoneName != NAME_None)
	{
		const float* DamageMultiplier = BodyPartDamageMultipliers.Find(Hit.BoneName);
		if (DamageMultiplier)
		{
			FinalDamage *= *DamageMultiplier;
		}
	}

	// 크리티컬 체크
	bOutWasCritical = FMath::RandRange(0.0f, 1.0f) <= CriticalChance;
	if (bOutWasCritical)
	{
		FinalDamage *= CriticalMultiplier;
	}

	return FinalDamage;
}

void UHitBoxComponent::ApplyDamageToActor(AActor* HitActor, const FHitResult& Hit, float FinalDamage, bool bWasCritical)
{
	if (!HitActor || !OwnerCharacter)
	{
		return;
	}

	float ActualDamage = 0.0f;

	FDamageInfo DamageInfo;
	DamageInfo.BaseDamage = FinalDamage;
	DamageInfo.SourceActor = OwnerCharacter;

	if (ICombatable* Combatable = Cast<ICombatable>(HitActor))
	{
		ActualDamage = Combatable->ReceiveDamage(DamageInfo);
	}
	else if (UCombatComponentBase* CombatComp = HitActor->FindComponentByClass<UCombatComponentBase>())
	{
		ActualDamage = CombatComp->ReceiveDamage(DamageInfo);
	}
	else
	{
		ActualDamage = UGameplayStatics::ApplyDamage(
			HitActor,
			FinalDamage,
			OwnerCharacter->GetInstigatorController(),
			OwnerCharacter,
			UDamageType::StaticClass());
	}

	if (ActualDamage > 0.0f)
	{
		// ShowDamageNumber(Hit.Location, ActualDamage, bWasCritical);
	}

	if (bApplyHitStun && HitStunDuration > 0.0f)
	{
		// TODO: target stun interface/component integration
	}
}


#pragma endregion

#pragma region Effects

void UHitBoxComponent::PlayHitEffects(const FVector& Location, const FVector& Normal)
{
	// 파티클 이펙트
	if (HitEffect)
	{
		FRotator EffectRotation = Normal.Rotation();
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitEffect,
			Location,
			EffectRotation
		);
	}

	// 사운드 이펙트
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			HitSound,
			Location
		);
	}
}

void UHitBoxComponent::ApplyHitStop()
{
	if (OwnerCharacter)
	{
		// OwnerCharacter->ApplyHitStop(HitStopDuration);
	}
}

void UHitBoxComponent::ApplyKnockback(AActor* HitActor, const FVector& Direction)
{
	if (!HitActor || KnockbackForce <= 0.0f)
	{
		return;
	}

	// Character Movement Component를 통한 넉백
	if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
	{
		if (UCharacterMovementComponent* MovementComp = HitCharacter->GetCharacterMovement())
		{
			FVector KnockbackVelocity = Direction * KnockbackForce;
			KnockbackVelocity.Z = FMath::Max(KnockbackVelocity.Z, KnockbackForce * 0.3f); // 최소 상승력
			MovementComp->AddImpulse(KnockbackVelocity, true);
		}
	}
}

#pragma endregion

#pragma region Settings

void UHitBoxComponent::SetTraceSettings(const FHitBoxSettings& NewSettings)
{
	HitBoxSettings = NewSettings;
	UpdateQueryParams();
}

void UHitBoxComponent::SetCriticalSettings(float NewCritChance, float NewCritMultiplier)
{
	CriticalChance = FMath::Clamp(NewCritChance, 0.0f, 1.0f);
	CriticalMultiplier = FMath::Max(1.0f, NewCritMultiplier);
}

void UHitBoxComponent::AddBodyPartMultiplier(FName BodyPart, float Multiplier)
{
	BodyPartDamageMultipliers.Add(BodyPart, Multiplier);
}

void UHitBoxComponent::RemoveBodyPartMultiplier(FName BodyPart)
{
	BodyPartDamageMultipliers.Remove(BodyPart);
}

void UHitBoxComponent::SetDebugDraw(bool bEnabled, float Duration)
{
	bDebugDraw = bEnabled;
	if (Duration > 0.0f)
	{
		DebugDrawDuration = Duration;
	}
}

#pragma endregion

#pragma region Utility

TArray<AActor*> UHitBoxComponent::GetHitActors() const
{
	TArray<AActor*> Result;
	for (const TObjectPtr<AActor>& Actor : HitActors)
	{
		if (IsValid(Actor))
		{
			Result.Add(Actor.Get());
		}
	}
	return Result;
}

void UHitBoxComponent::AddIgnoreActor(AActor* ActorToIgnore)
{
	if (ActorToIgnore)
	{
		IgnoreActors.AddUnique(ActorToIgnore);
		UpdateQueryParams();
	}
}

void UHitBoxComponent::RemoveIgnoreActor(AActor* ActorToRemove)
{
	if (ActorToRemove)
	{
		IgnoreActors.Remove(ActorToRemove);
		UpdateQueryParams();
	}
}

#pragma endregion

#pragma region Internal_Functions

void UHitBoxComponent::SetHitBoxState(EHitBoxState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EHitBoxState OldState = CurrentState;
	CurrentState = NewState;

	OnHitBoxStateChanged.Broadcast(OldState, NewState);
}

void UHitBoxComponent::UpdateQueryParams()
{
	CachedQueryParams.ClearIgnoredSourceObjects();
	CachedQueryParams.bTraceComplex = HitBoxSettings.bTraceComplex;
	CachedQueryParams.bReturnPhysicalMaterial = true;

	// Owner 무시
	if (HitBoxSettings.bIgnoreOwner && GetOwner())
	{
		CachedQueryParams.AddIgnoredActor(GetOwner());
	}

	// 추가 무시 액터들
	for (const TObjectPtr<AActor>& IgnoreActor : IgnoreActors)
	{
		if (IsValid(IgnoreActor))
		{
			CachedQueryParams.AddIgnoredActor(IgnoreActor.Get());
		}
	}
}

void UHitBoxComponent::CacheTracePositions()
{
	PreviousTracePositions.Empty();
	
	if (!OwnerMesh)
	{
		return;
	}

	for (const FName& SocketName : HitBoxSettings.TraceSocketNames)
	{
		if (OwnerMesh->DoesSocketExist(SocketName))
		{
			PreviousTracePositions.Add(OwnerMesh->GetSocketLocation(SocketName));
		}
	}
}

void UHitBoxComponent::OnMultiHitTimerComplete()
{
	// 다단히트 타이머 완료 - 특별한 처리가 필요한 경우 여기에 구현
}

void UHitBoxComponent::OnCooldownComplete()
{
	SetHitBoxState(EHitBoxState::Inactive);
}

#pragma endregion


