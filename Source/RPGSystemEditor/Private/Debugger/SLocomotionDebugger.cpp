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

				// =========================================================
				// 1. TARGET ACTOR (대상 정보)
				// =========================================================
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeSectionHeader(LOCTEXT("TargetHeader", "TARGET ACTOR"), FLinearColor(0.2f, 1.0f, 0.2f))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeDebugRow(LOCTEXT("TargetLabel", "Character Name:"), 
						TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetTargetName)))
				]

				// =========================================================
				// 2. LOCOMOTION DATA (이동 데이터)
				// =========================================================
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("LocomotionHeader", "LOCOMOTION DATA"), FLinearColor(1.0f, 0.8f, 0.2f))
				]
				// Basic Movement
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("GroundSpeed", "Ground Speed:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_GroundSpeed))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("Velocity", "Velocity (Vector):"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_Velocity))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("InputAccel", "Input Acceleration:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_InputAcceleration))) ]
				
				// Direction & Rotation
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("LocoDir", "Locomotion Direction:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_Direction))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("WorldRot", "World Rotation:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_WorldRotation))) ]

				// Booleans (Flags)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)
				[
					MakeDebugRow(LOCTEXT("MoveFlags", "Movement Flags:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_MovementFlags)))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeDebugRow(LOCTEXT("StateFlags", "State Flags:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_StateFlags)))
				]

				// Fall & Jump
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("Landing", "Landing Impact:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetLoco_LandingImpact))) ]


				// =========================================================
				// 3. COMBAT DATA (전투 데이터)
				// =========================================================
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					MakeSectionHeader(LOCTEXT("CombatHeader", "COMBAT DATA"), FLinearColor(1.0f, 0.4f, 0.4f))
				]
				// Status
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("CombatState", "Is In Combat:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_IsInCombat))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("Overlay", "Overlay State:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_OverlayState))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("WpnStyle", "Weapon Style:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_WeaponStyle))) ]

				// Aiming
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("AimOffset", "Aim Offset (Pitch/Yaw):"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_AimOffset))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("AimBlend", "Aim Blend Weight:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_AimBlendWeight))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("AimFlags", "Aiming / BowReady:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_AimFlags))) ]

				// Action & Guard
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("PrimaryAction", "Primary Action:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_PrimaryAction))) ]
				+ SVerticalBox::Slot().AutoHeight() [ MakeDebugRow(LOCTEXT("GuardAction", "Guard Action:"), TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLocomotionDebugger::GetCombat_GuardAction))) ]
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
			.MinDesiredWidth(150.0f)
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

#define CHECK_ANIM_RET_EMPTY() if (!CachedAnimInstance.IsValid()) return FText::GetEmpty();

FText SLocomotionDebugger::GetTargetName() const
{
	if (TargetCharacter.IsValid()) return FText::FromString(TargetCharacter->GetName());
	return LOCTEXT("Waiting", "Waiting for PIE...");
}

// ------------------------------------------------------------------------------------
// [LOCOMOTION DATA] Getters
// ------------------------------------------------------------------------------------

FText SLocomotionDebugger::GetLoco_GroundSpeed() const
{
	CHECK_ANIM_RET_EMPTY();
	return FText::AsNumber(CachedAnimInstance->GetLocomotionData().GroundSpeed);
}

FText SLocomotionDebugger::GetLoco_Velocity() const
{
	CHECK_ANIM_RET_EMPTY();
	const FVector& Vel = CachedAnimInstance->GetLocomotionData().Velocity;
	return FText::FromString(FString::Printf(TEXT("X:%.1f Y:%.1f Z:%.1f"), Vel.X, Vel.Y, Vel.Z));
}

FText SLocomotionDebugger::GetLoco_InputAcceleration() const
{
	CHECK_ANIM_RET_EMPTY();
	const FVector& Acc = CachedAnimInstance->GetLocomotionData().InputAcceleration;
	return FText::FromString(FString::Printf(TEXT("X:%.1f Y:%.1f"), Acc.X, Acc.Y));
}

FText SLocomotionDebugger::GetLoco_Direction() const
{
	CHECK_ANIM_RET_EMPTY();
	const auto& Data = CachedAnimInstance->GetLocomotionData();
	return FText::FromString(FString::Printf(TEXT("Cur: %.1f  (Last: %.1f)"), 
		Data.LocomotionDirection, Data.LastLocomotionDirection));
}

FText SLocomotionDebugger::GetLoco_WorldRotation() const
{
	CHECK_ANIM_RET_EMPTY();
	return FText::FromString(CachedAnimInstance->GetLocomotionData().WorldRotation.ToString());
}

FText SLocomotionDebugger::GetLoco_MovementFlags() const
{
	CHECK_ANIM_RET_EMPTY();
	const auto& Data = CachedAnimInstance->GetLocomotionData();
	
	FString S_Move = Data.bShouldMove ? TEXT("SHOULD MOVE") : TEXT("-");
	FString S_Accel = Data.bHasAcceleration ? TEXT("HAS ACCEL") : TEXT("-");
	FString S_Back = Data.bIsMovingBackward ? TEXT("BACKWARD") : TEXT("-");

	return FText::FromString(FString::Printf(TEXT("[%s] [%s] [%s]"), *S_Move, *S_Accel, *S_Back));
}

FText SLocomotionDebugger::GetLoco_StateFlags() const
{
	CHECK_ANIM_RET_EMPTY();
	const auto& Data = CachedAnimInstance->GetLocomotionData();

	FString S_Fall = Data.bIsFalling ? TEXT("FALLING") : TEXT("GROUND");
	FString S_Crouch = Data.bIsCrouch ? TEXT("CROUCH") : TEXT("STAND");
	FString S_Sprint = Data.bIsSprint ? TEXT("SPRINT") : TEXT("-");
	FString S_Walk = Data.bIsWalking ? TEXT("WALK") : TEXT("RUN");

	return FText::FromString(FString::Printf(TEXT("%s | %s | %s | %s"), *S_Fall, *S_Crouch, *S_Walk, *S_Sprint));
}

FText SLocomotionDebugger::GetLoco_LandingImpact() const
{
	CHECK_ANIM_RET_EMPTY();
	return FText::AsNumber(CachedAnimInstance->GetLocomotionData().LandingImpactSpeed);
}

// ------------------------------------------------------------------------------------
// [COMBAT DATA] Getters
// ------------------------------------------------------------------------------------

FText SLocomotionDebugger::GetCombat_IsInCombat() const
{
	CHECK_ANIM_RET_EMPTY();
	return CachedAnimInstance->GetCombatData().bIsInCombat ? FText::FromString(TEXT("TRUE (Combat Mode)")) : FText::FromString(TEXT("FALSE (Peace Mode)"));
}

FText SLocomotionDebugger::GetCombat_OverlayState() const
{
	CHECK_ANIM_RET_EMPTY();
	const UEnum* EnumPtr = StaticEnum<ECharacterOverlayState>();
	if (EnumPtr)
	{
		return EnumPtr->GetDisplayNameTextByValue((int64)CachedAnimInstance->GetCombatData().OverlayState);
	}
	return FText::FromString("Unknown");
}

FText SLocomotionDebugger::GetCombat_WeaponStyle() const
{
	CHECK_ANIM_RET_EMPTY();
	return FText::AsNumber(CachedAnimInstance->GetCombatData().WeaponStyle);
}

FText SLocomotionDebugger::GetCombat_AimOffset() const
{
	CHECK_ANIM_RET_EMPTY();
	const FRotator& Aim = CachedAnimInstance->GetCombatData().AimOffset;
	return FText::FromString(FString::Printf(TEXT("P: %.1f / Y: %.1f"), Aim.Pitch, Aim.Yaw));
}

FText SLocomotionDebugger::GetCombat_AimBlendWeight() const
{
	CHECK_ANIM_RET_EMPTY();
	return FText::AsNumber(CachedAnimInstance->GetCombatData().AimBlendWeight);
}

FText SLocomotionDebugger::GetCombat_AimFlags() const
{
	CHECK_ANIM_RET_EMPTY();
	const auto& Data = CachedAnimInstance->GetCombatData();
	FString Aim = Data.bIsAiming ? TEXT("AIMING") : TEXT("-");
	FString Bow = Data.bIsBowReady ? TEXT("BOW READY") : TEXT("-");
	return FText::FromString(FString::Printf(TEXT("[%s]  [%s]"), *Aim, *Bow));
}

FText SLocomotionDebugger::GetCombat_PrimaryAction() const
{
	CHECK_ANIM_RET_EMPTY();
	const auto& Data = CachedAnimInstance->GetCombatData();
	FString Down = Data.bIsPrimaryDown ? TEXT("DOWN") : TEXT("UP");
	return FText::FromString(FString::Printf(TEXT("%s (Blend: %.2f)"), *Down, Data.PrimaryBlendWeight));
}

FText SLocomotionDebugger::GetCombat_GuardAction() const
{
	CHECK_ANIM_RET_EMPTY();
	const auto& Data = CachedAnimInstance->GetCombatData();
	FString Guard = Data.bIsGuarding ? TEXT("GUARDING") : TEXT("-");
	return FText::FromString(FString::Printf(TEXT("%s (Blend: %.2f)"), *Guard, Data.GuardBlendWeight));
}

// ------------------------------------------------------------------------------------
// Unused / Deprecated Reflection Helpers (Header compatibility)
// ------------------------------------------------------------------------------------
FString SLocomotionDebugger::GetEnumPropertyAsString(UObject* Obj, FName PropName, const TCHAR* EnumPath) const { return ""; }
FString SLocomotionDebugger::GetBoolPropertyAsString(UObject* Obj, FName PropName) const { return ""; }
float SLocomotionDebugger::GetFloatProperty(UObject* Obj, FName PropName) const { return 0.0f; }
int32 SLocomotionDebugger::GetIntProperty(UObject* Obj, FName PropName) const { return 0; }
FString SLocomotionDebugger::GetVectorPropertyAsString(UObject* Obj, FName PropName) const { return ""; }
FString SLocomotionDebugger::GetRotatorPropertyAsString(UObject* Obj, FName PropName) const { return ""; }

#undef LOCTEXT_NAMESPACE