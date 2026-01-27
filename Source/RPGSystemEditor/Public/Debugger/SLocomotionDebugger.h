// Source/RPGSystemEditor/Public/Debugger/SLocomotionDebugger.h

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ARPGPlayerCharacter;
class UPlayerAnimInstance;

/**
 * PIE에서 ARPGPlayerCharacter를 자동으로 찾아 Locomotion 및 Combat 상태를 보여주는 디버거
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

	/** --- Data Getters (AnimInstance의 데이터를 읽어옴) --- */
	
	// Actor Info
	FText GetTargetName() const;

	// [LOCOMOTION DATA]
	FText GetLoco_GroundSpeed() const;
	FText GetLoco_Velocity() const;
	FText GetLoco_InputAcceleration() const;
	FText GetLoco_Direction() const;
	FText GetLoco_WorldRotation() const;
	FText GetLoco_MovementFlags() const;
	FText GetLoco_StateFlags() const;
	FText GetLoco_LandingImpact() const;

	// [COMBAT DATA]
	FText GetCombat_IsInCombat() const;
	FText GetCombat_OverlayState() const;
	FText GetCombat_WeaponStyle() const;
	FText GetCombat_AimOffset() const;
	FText GetCombat_AimBlendWeight() const;
	FText GetCombat_AimFlags() const;
	FText GetCombat_PrimaryAction() const;
	FText GetCombat_GuardAction() const;

	/** --- Deprecated / Unused Reflection Helpers --- */
	// .cpp와의 호환성을 위해 남겨두었으나, 내부는 비워져 있습니다.
	FString GetEnumPropertyAsString(UObject* Obj, FName PropName, const TCHAR* EnumPath) const;
	FString GetBoolPropertyAsString(UObject* Obj, FName PropName) const;
	float GetFloatProperty(UObject* Obj, FName PropName) const;
	int32 GetIntProperty(UObject* Obj, FName PropName) const;
	FString GetVectorPropertyAsString(UObject* Obj, FName PropName) const;
	FString GetRotatorPropertyAsString(UObject* Obj, FName PropName) const;

private:
	/** 현재 추적 중인 캐릭터 (약한 참조) */
	TWeakObjectPtr<ARPGPlayerCharacter> TargetCharacter;
	
	/** 캐시된 애님 인스턴스 */
	TWeakObjectPtr<UPlayerAnimInstance> CachedAnimInstance;
};