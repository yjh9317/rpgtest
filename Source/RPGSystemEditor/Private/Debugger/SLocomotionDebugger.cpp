// Source/RPGSystemEditor/Private/Debugger/SLocomotionDebugger.cpp

#include "Debugger/SLocomotionDebugger.h"

// Game Classes
#include "Player/RPGPlayerCharacter.h"
#include "Player/PlayerAnimInstance.h"

// Engine / Editor Headers
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "EngineUtils.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SLocomotionDebugger"

void SLocomotionDebugger::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.Padding(4.0f)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(5.0f)
			[
				SNew(SVerticalBox)

				// === 1. Target Info ===
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeSectionHeader(LOCTEXT("TargetHeader", "TARGET ACTOR"), FLinearColor(0.2f, 1.0f, 0.2f))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeDebugRow(LOCTEXT("TargetLabel", "Selected Character:"), 
						TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetTargetName)))
				]

				// === 2. Combat & Overlay ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("CombatHeader", "COMBAT & OVERLAY"), FLinearColor(1.0f, 0.4f, 0.4f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("Overlay", "Overlay State:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimOverlayState))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("WpnStyle", "Weapon Style (Int):"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimWeaponStyle))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("CombatState", "Is In Combat:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimCombatState))) ]

				// === 3. Location Data ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("LocationHeader", "LOCATION DATA"), FLinearColor(1.0f, 0.8f, 0.2f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("WorldLoc", "World Location:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimWorldLocation))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("DispSpeed", "Displacement Speed:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimDisplacementSpeed))) ]

				// === 4. Rotation Data ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("RotationHeader", "ROTATION DATA"), FLinearColor(1.0f, 0.8f, 0.2f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("WorldRot", "World Rotation:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimWorldRotation))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("YawDelta", "Yaw Delta Speed:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimYawDeltaSpeed))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("LeanAngle", "Additive Lean Angle:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimAdditiveLeanAngle))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("RootYaw", "Root Yaw Offset:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimRootYawOffset))) ]

				// === 5. Velocity Data ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("VelocityHeader", "VELOCITY DATA"), FLinearColor(1.0f, 0.8f, 0.2f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("WorldVel", "World Velocity:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimWorldVelocity))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("LocalVel", "Local Velocity 2D:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimLocalVelocity2D))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("VelAngle", "Dir Angle (Off/NoOff):"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimLocalVelocityDirAngle))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("VelDir", "Cardinal Direction:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimLocalVelocityDir))) ]

				// === 6. Acceleration Data ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("AccelHeader", "ACCELERATION DATA"), FLinearColor(1.0f, 0.8f, 0.2f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("LocalAccel", "Local Accel 2D:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimLocalAcceleration2D))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("PivotDir", "Pivot Direction:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimPivotDirection))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("AccelCard", "Accel Cardinal:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimCardinalDirFromAccel))) ]

				// === 7. State Data ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("StateHeader", "CHARACTER STATE"), FLinearColor(0.2f, 0.8f, 1.0f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("MoveFlags", "Movement Flags:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimMovementFlags))) ]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SLocomotionDebugger::GetAnimStateFlags)
					.ColorAndOpacity(FLinearColor::White)
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("FireTime", "Time Since Fired:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimTimeSinceFired))) ]

				// === 8. Jump / Fall Data ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("JumpHeader", "JUMP / FALL / LAND"), FLinearColor(1.0f, 0.5f, 0.5f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("JumpApex", "Time To Jump Apex:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimTimeToJumpApex))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("TimeFall", "Time Falling:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimTimeFalling))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("GroundDist", "Ground Distance:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimGroundDistance))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("LandImp", "Landing Speed:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimLandingImpact))) ]

				// === 9. Aiming Data ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("AimHeader", "AIMING DATA"), FLinearColor(0.5f, 1.0f, 0.5f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("AimFlags", "Aim Flags:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimAimingFlags))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("AimData", "Pitch / Yaw:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimAiming))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("AimBlend", "Aim Blend Weight:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimAimBlend))) ]

				// === 10. Action & Guard Data (New!) ===
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("ActionGuardHeader", "ACTION & GUARD"), FLinearColor(1.0f, 0.2f, 0.2f))
				]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("PrimaryDown", "Primary Down:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimPrimaryDown))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("PrimaryBlend", "Primary Blend:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimPrimaryBlend))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("IsGuarding", "Is Guarding:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimGuarding))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("GuardBlend", "Guard Blend:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetAnimGuardBlend))) ]
			]
		]
	];
}

void SLocomotionDebugger::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	FindTargetCharacter();
}

void SLocomotionDebugger::FindTargetCharacter()
{
	UWorld* PlayWorld = GEditor->PlayWorld;

	if (!PlayWorld)
	{
		TargetCharacter.Reset();
		CachedAnimInstance.Reset();
		return;
	}

	if (TargetCharacter.IsValid() && IsValid(TargetCharacter.Get()))
	{
		if (TargetCharacter->GetMesh())
		{
			UAnimInstance* CurrentAnim = TargetCharacter->GetMesh()->GetAnimInstance();
			if (CachedAnimInstance.Get() != CurrentAnim)
			{
				CachedAnimInstance = Cast<UPlayerAnimInstance>(CurrentAnim);
			}
		}
		return;
	}

	for (TActorIterator<ARPGPlayerCharacter> It(PlayWorld); It; ++It)
	{
		ARPGPlayerCharacter* FoundChar = *It;
		if (IsValid(FoundChar))
		{
			TargetCharacter = FoundChar;
			if (FoundChar->GetMesh())
			{
				CachedAnimInstance = Cast<UPlayerAnimInstance>(FoundChar->GetMesh()->GetAnimInstance());
			}
			return;
		}
	}
}

TSharedRef<SWidget> SLocomotionDebugger::MakeSectionHeader(const FText& HeaderText, FLinearColor Color)
{
	return SNew(SBorder)
		.Padding(FMargin(5.0f, 2.0f))
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(STextBlock)
			.Text(HeaderText)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			.ColorAndOpacity(Color)
		];
}

TSharedRef<SWidget> SLocomotionDebugger::MakeDebugRow(const FText& Label, const TAttribute<FText>& ValueAttribute)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 2)
		[
			SNew(STextBlock)
			.Text(Label)
			.MinDesiredWidth(160.0f)
			.ColorAndOpacity(FLinearColor::Gray)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(10, 2, 0, 2)
		[
			SNew(STextBlock)
			.Text(ValueAttribute)
			.ColorAndOpacity(FLinearColor::White)
		];
}

// ====================================================================================
// Data Getters implementation
// ====================================================================================

FText SLocomotionDebugger::GetTargetName() const
{
	if (TargetCharacter.IsValid()) return FText::FromString(TargetCharacter->GetName());
	return LOCTEXT("Waiting", "Waiting for PIE...");
}

// --- Combat & Overlay ---
FText SLocomotionDebugger::GetAnimOverlayState() const
{
	if (CachedAnimInstance.IsValid())
	{
		return FText::FromString(GetEnumPropertyAsString(CachedAnimInstance.Get(), TEXT("OverlayState"), TEXT("/Script/RPGSystem.ECharacterOverlayState")));
	}
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimWeaponStyle() const
{
	if (CachedAnimInstance.IsValid())
	{
		int32 Val = GetIntProperty(CachedAnimInstance.Get(), TEXT("WeaponStyle"));
		return FText::AsNumber(Val);
	}
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimCombatState() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bIsInCombat")));
	return FText::GetEmpty();
}

// --- Location ---
FText SLocomotionDebugger::GetAnimWorldLocation() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetVectorPropertyAsString(CachedAnimInstance.Get(), TEXT("WorldLocation")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimDisplacementSpeed() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("DisplacementSpeed")));
	return FText::GetEmpty();
}

// --- Rotation ---
FText SLocomotionDebugger::GetAnimWorldRotation() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetRotatorPropertyAsString(CachedAnimInstance.Get(), TEXT("WorldRotation")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimYawDeltaSpeed() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("YawDeltaSpeed")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimAdditiveLeanAngle() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("AdditiveLeanAngle")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimRootYawOffset() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("RootYawOffset")));
	return FText::GetEmpty();
}

// --- Velocity ---
FText SLocomotionDebugger::GetAnimWorldVelocity() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetVectorPropertyAsString(CachedAnimInstance.Get(), TEXT("WorldVelocity")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimLocalVelocity2D() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetVectorPropertyAsString(CachedAnimInstance.Get(), TEXT("LocalVelocity2D")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimLocalVelocityDirAngle() const
{
	if (CachedAnimInstance.IsValid())
	{
		float Angle = GetFloatProperty(CachedAnimInstance.Get(), TEXT("LocalVelocityDirectionAngle"));
		float AngleWithOffset = GetFloatProperty(CachedAnimInstance.Get(), TEXT("LocalVelocityDirectionAngleWithOffset"));
		return FText::FromString(FString::Printf(TEXT("%.1f / %.1f"), AngleWithOffset, Angle));
	}
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimLocalVelocityDir() const
{
	if (CachedAnimInstance.IsValid())
	{
		FString Dir = GetEnumPropertyAsString(CachedAnimInstance.Get(), TEXT("LocalVelocityDirection"), TEXT("/Script/RPGSystem.ECardinalDirection"));
		FString DirNoOffset = GetEnumPropertyAsString(CachedAnimInstance.Get(), TEXT("LocalVelocityDirectionNoOffset"), TEXT("/Script/RPGSystem.ECardinalDirection"));
		return FText::FromString(FString::Printf(TEXT("%s (NoOff: %s)"), *Dir, *DirNoOffset));
	}
	return FText::GetEmpty();
}

// --- Acceleration ---
FText SLocomotionDebugger::GetAnimLocalAcceleration2D() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetVectorPropertyAsString(CachedAnimInstance.Get(), TEXT("LocalAcceleration2D")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimPivotDirection() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetVectorPropertyAsString(CachedAnimInstance.Get(), TEXT("PivotDirection2D")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimCardinalDirFromAccel() const
{
	if (CachedAnimInstance.IsValid())
	{
		return FText::FromString(GetEnumPropertyAsString(CachedAnimInstance.Get(), TEXT("CardinalDirectionFromAcceleration"), TEXT("/Script/RPGSystem.ECardinalDirection")));
	}
	return FText::GetEmpty();
}

// --- Character State ---
FText SLocomotionDebugger::GetAnimMovementFlags() const
{
	if (CachedAnimInstance.IsValid())
	{
		FString Move = GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bShouldMove"));
		FString Fall = GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bIsFalling"));
		FString Back = GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bIsMovingBackward"));
		
		return FText::FromString(FString::Printf(TEXT("ShouldMove: %s | Fall: %s | Back: %s"), *Move, *Fall, *Back));
	}
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimStateFlags() const
{
	if (CachedAnimInstance.IsValid())
	{
		TArray<FString> States;
		auto CheckState = [&](const TCHAR* PropName, const TCHAR* Display)
		{
			if (GetBoolPropertyAsString(CachedAnimInstance.Get(), PropName) == TEXT("True"))
			{
				States.Add(Display);
			}
		};

		CheckState(TEXT("IsOnGround"), TEXT("[Ground]"));
		CheckState(TEXT("IsCrouching"), TEXT("[Crouch]"));
		CheckState(TEXT("IsJumping"), TEXT("[Jump]"));
		CheckState(TEXT("bIsSprint"), TEXT("[Sprint]"));
		CheckState(TEXT("HasVelocity"), TEXT("[HasVel]"));
		CheckState(TEXT("bHasAcceleration"), TEXT("[HasAccel]"));
		CheckState(TEXT("IsRunningIntoWall"), TEXT("[Wall]"));
		CheckState(TEXT("CrouchStateChange"), TEXT("[CrouchChg]"));

		if (States.Num() == 0) return FText::FromString(TEXT("None"));
		return FText::FromString(FString::Join(States, TEXT(" ")));
	}
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimTimeSinceFired() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("TimeSinceFiredWeapon")));
	return FText::GetEmpty();
}

// --- Jump / Fall ---
FText SLocomotionDebugger::GetAnimTimeToJumpApex() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("TimeToJumpApex")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimTimeFalling() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("TimeFalling")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimGroundDistance() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("GroundDistance")));
	return FText::GetEmpty();
}
FText SLocomotionDebugger::GetAnimLandingImpact() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("LandingImpactSpeed")));
	return FText::GetEmpty();
}

// --- Aiming ---
FText SLocomotionDebugger::GetAnimAiming() const
{
	if (CachedAnimInstance.IsValid())
	{
		float Pitch = GetFloatProperty(CachedAnimInstance.Get(), TEXT("AimPitch"));
		float Yaw = GetFloatProperty(CachedAnimInstance.Get(), TEXT("AimYaw"));
		return FText::FromString(FString::Printf(TEXT("Pitch: %.1f / Yaw: %.1f"), Pitch, Yaw));
	}
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimAimingFlags() const
{
	if (CachedAnimInstance.IsValid())
	{
		FString Aim = GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bIsAiming"));
		FString Bow = GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bIsBowReady"));
		return FText::FromString(FString::Printf(TEXT("Aiming: %s | BowReady: %s"), *Aim, *Bow));
	}
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimAimBlend() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("AimBlendWeight")));
	return FText::GetEmpty();
}

// --- Action & Guard (New!) ---
FText SLocomotionDebugger::GetAnimPrimaryDown() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bIsPrimaryDown")));
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimPrimaryBlend() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("PrimaryBlendWeight")));
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimGuarding() const
{
	if (CachedAnimInstance.IsValid()) return FText::FromString(GetBoolPropertyAsString(CachedAnimInstance.Get(), TEXT("bIsGuarding")));
	return FText::GetEmpty();
}

FText SLocomotionDebugger::GetAnimGuardBlend() const
{
	if (CachedAnimInstance.IsValid()) return FText::AsNumber(GetFloatProperty(CachedAnimInstance.Get(), TEXT("GuardBlendWeight")));
	return FText::GetEmpty();
}

// ====================================================================================
// Reflection Helpers
// ====================================================================================

FString SLocomotionDebugger::GetEnumPropertyAsString(UObject* Obj, FName PropName, const TCHAR* EnumPath) const
{
	if (!Obj) return TEXT("None");
	const UEnum* EnumPtr = FindObject<UEnum>(nullptr, EnumPath, true);
	
	FProperty* Prop = Obj->GetClass()->FindPropertyByName(PropName);
	// Handle Byte Property (traditional Enums)
	if (FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
	{
		uint8 Val = ByteProp->GetPropertyValue_InContainer(Obj);
		if (EnumPtr) return EnumPtr->GetNameStringByValue(Val);
		return FString::FromInt(Val);
	}
	// Handle Enum Property (modern enum class)
	else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
	{
		void* ValPtr = EnumProp->ContainerPtrToValuePtr<void>(Obj);
		int64 Val = EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValPtr);
		if (EnumPtr) return EnumPtr->GetNameStringByValue(Val);
	}
	return TEXT("Invalid");
}

FString SLocomotionDebugger::GetBoolPropertyAsString(UObject* Obj, FName PropName) const
{
	if (!Obj) return TEXT("false");
	if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Obj->GetClass()->FindPropertyByName(PropName)))
	{
		return BoolProp->GetPropertyValue_InContainer(Obj) ? TEXT("True") : TEXT("False");
	}
	return TEXT("N/A");
}

float SLocomotionDebugger::GetFloatProperty(UObject* Obj, FName PropName) const
{
	if (!Obj) return 0.0f;
	if (FDoubleProperty* DblProp = CastField<FDoubleProperty>(Obj->GetClass()->FindPropertyByName(PropName)))
	{
		return (float)DblProp->GetPropertyValue_InContainer(Obj);
	}
	else if (FFloatProperty* FltProp = CastField<FFloatProperty>(Obj->GetClass()->FindPropertyByName(PropName)))
	{
		return FltProp->GetPropertyValue_InContainer(Obj);
	}
	return 0.0f;
}

int32 SLocomotionDebugger::GetIntProperty(UObject* Obj, FName PropName) const
{
	if (!Obj) return 0;
	if (FIntProperty* IntProp = CastField<FIntProperty>(Obj->GetClass()->FindPropertyByName(PropName)))
	{
		return IntProp->GetPropertyValue_InContainer(Obj);
	}
	return 0;
}

FString SLocomotionDebugger::GetVectorPropertyAsString(UObject* Obj, FName PropName) const
{
	if (!Obj) return TEXT("0, 0, 0");
	if (FStructProperty* StructProp = CastField<FStructProperty>(Obj->GetClass()->FindPropertyByName(PropName)))
	{
		if (StructProp->Struct == TBaseStructure<FVector>::Get())
		{
			FVector* ValPtr = StructProp->ContainerPtrToValuePtr<FVector>(Obj);
			if (ValPtr) return ValPtr->ToString();
		}
	}
	return TEXT("N/A");
}

FString SLocomotionDebugger::GetRotatorPropertyAsString(UObject* Obj, FName PropName) const
{
	if (!Obj) return TEXT("0, 0, 0");
	if (FStructProperty* StructProp = CastField<FStructProperty>(Obj->GetClass()->FindPropertyByName(PropName)))
	{
		if (StructProp->Struct == TBaseStructure<FRotator>::Get())
		{
			FRotator* ValPtr = StructProp->ContainerPtrToValuePtr<FRotator>(Obj);
			if (ValPtr) return ValPtr->ToString();
		}
	}
	return TEXT("N/A");
}

#undef LOCTEXT_NAMESPACE