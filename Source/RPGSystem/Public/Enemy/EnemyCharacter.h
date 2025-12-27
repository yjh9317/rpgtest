// EnemyCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "Combat/Action/ActionOwner.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class RPGSYSTEM_API AEnemyCharacter : public ACharacter, public IActionOwner
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 이 캐릭터가 AI 감각의 대상(시각적/청각적 자극원)이 되도록 함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionStimuliSourceComponent* StimuliSourceComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UActionComponent> ActionComponent;

	virtual UActionComponent* GetActionComponent() const override
	{
		return ActionComponent;
	}
};