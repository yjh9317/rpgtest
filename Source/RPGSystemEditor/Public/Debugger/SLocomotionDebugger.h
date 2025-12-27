// Source/RPGSystemEditor/Public/Debugger/SLocomotionDebugger.h

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ARPGPlayerCharacter;
class UPlayerAnimInstance;

/**
 * PIE에서 ARPGPlayerCharacter를 자동으로 찾아 Locomotion 상태를 보여주는 디버거
 */
class RPGSYSTEMEDITOR_API SLocomotionDebugger : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLocomotionDebugger) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** PIE 월드에서 PlayerCharacter를 찾습니다. */
	void FindTargetCharacter();

	/** UI 헬퍼 함수 */
	TSharedRef<SWidget> MakeSectionHeader(const FText& HeaderText, FLinearColor Color);
	TSharedRef<SWidget> MakeDebugRow(const FText& Label, const TAttribute<FText>& ValueAttribute);

	/** --- Data Getters (AnimInstance의 Property를 읽어옴) --- */
	
	// Actor Info
	FText GetTargetName() const;

	// [New] Combat & Overlay Data
	FText GetAnimOverlayState() const;
	FText GetAnimWeaponStyle() const;
	FText GetAnimCombatState() const; // bIsInCombat

	// Location Data
	FText GetAnimWorldLocation() const;
	FText GetAnimDisplacementSpeed() const;

	// Rotation Data
	FText GetAnimWorldRotation() const;
	FText GetAnimYawDeltaSpeed() const;
	FText GetAnimAdditiveLeanAngle() const;
	FText GetAnimRootYawOffset() const;

	// Velocity Data
	FText GetAnimWorldVelocity() const;
	FText GetAnimLocalVelocity2D() const;
	FText GetAnimLocalVelocityDirAngle() const;
	FText GetAnimLocalVelocityDir() const;

	// Acceleration Data
	FText GetAnimLocalAcceleration2D() const;
	FText GetAnimPivotDirection() const;
	FText GetAnimCardinalDirFromAccel() const;

	// Character State Data
	FText GetAnimStateFlags() const;      // 기존 State Flags
	FText GetAnimMovementFlags() const;   // [New] bShouldMove, bIsMovingBackward, etc.
	FText GetAnimTimeSinceFired() const;

	// Jump / Fall Data
	FText GetAnimTimeToJumpApex() const;
	FText GetAnimTimeFalling() const;
	FText GetAnimGroundDistance() const;
	FText GetAnimLandingImpact() const;   // [New] LandingImpactSpeed

	// Aiming Data
	FText GetAnimAiming() const;          // Pitch / Yaw
	FText GetAnimAimingFlags() const;     // [New] IsAiming, IsBowReady
	FText GetAnimAimBlend() const;        // [New] AimBlendWeight
	
	FText GetAnimPrimaryDown() const;
	FText GetAnimPrimaryBlend() const;
	FText GetAnimGuardBlend() const;
	FText GetAnimGuarding() const;
	/** --- Reflection Helpers --- */
	FString GetEnumPropertyAsString(UObject* Obj, FName PropName, const TCHAR* EnumPath) const;
	FString GetBoolPropertyAsString(UObject* Obj, FName PropName) const;
	float GetFloatProperty(UObject* Obj, FName PropName) const;
	int32 GetIntProperty(UObject* Obj, FName PropName) const; // [New] Added for WeaponStyle
	FString GetVectorPropertyAsString(UObject* Obj, FName PropName) const;
	FString GetRotatorPropertyAsString(UObject* Obj, FName PropName) const;

private:
	/** 현재 추적 중인 캐릭터 (약한 참조) */
	TWeakObjectPtr<ARPGPlayerCharacter> TargetCharacter;
	
	/** 캐시된 애님 인스턴스 */
	TWeakObjectPtr<UPlayerAnimInstance> CachedAnimInstance;
};