// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActionEventHandler.generated.h"

UENUM(BlueprintType)
enum class EActionEvent : uint8
{
	None = 0,

	// 실행/수명 주기
	Started,           // 액션 시작
	Active,            // 본격 동작 구간 진입
	Finished,          // 정상 종료
	Interrupted,       // 피격/캔슬 등으로 중단
	Reset,             // 상태 강제 리셋

	// 공격/히트 관련
	HitCheckStart,     // 히트박스 활성화 시작
	HitCheckEnd,       // 히트박스 비활성화
	HitConfirmed,      // 실제 히트 확정(피해 적용 타이밍)
	BlockedHit,        // 가드/막기 판정
	Parried,           // 패리에 막힘

	// 콤보/입력 창
	ComboWindowOpen,   // 콤보 입력 가능
	ComboWindowClose,  // 콤보 입력 불가
	InputBufferOpen,   // 입력 버퍼 허용
	InputBufferClose,  // 입력 버퍼 닫힘

	// 이동/회전 제어
	MovementLockOn,    // 이동 잠금 (루트모션 등)
	MovementLockOff,   // 이동 잠금 해제
	RotationLockOn,    // 회전 잠금
	RotationLockOff,   // 회전 잠금 해제
	RootMotionOn,      // 루트모션 사용 시작
	RootMotionOff,     // 루트모션 사용 종료

	// 방어/슈퍼아머·무적 등
	SuperArmorOn,      // 경직/히트스턴 무시
	SuperArmorOff,
	GuardOn,           // 가드 판정 활성
	GuardOff,
	IFrameOn,          // 무적 프레임 시작
	IFrameOff,         // 무적 프레임 종료,

	// 리소스/상태
	StaminaRegenBlockOn,   // 스태미나 회복 정지
	StaminaRegenBlockOff,
	PoiseGain,             // 자세/포이즈 상승
	PoiseBreak,            // 포이즈 붕괴(경직 발생)

	// 이펙트/사운드
	WeaponTrailOn,     // 무기 트레일 시작
	WeaponTrailOff,
	PlayImpactVFX,     // 피격/충돌 이펙트
	PlayAttackSFX,     // 공격 사운드
	PlayFootstepSFX    // 발소리 등 공용
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UActionEventHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IActionEventHandler
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void HandleActionEvent(EActionEvent EventType, float Value) {};
	virtual bool CanHandleActionEvent(EActionEvent EventType) const { return true;}
};
