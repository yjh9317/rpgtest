// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NiagaraSystem.h"
#include "ANS_RPGTrail.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UANS_RPGTrail : public UAnimNotifyState
{
	GENERATED_BODY()
public:
    	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    	TObjectPtr<UNiagaraSystem> NiagaraTemplate;

    	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    	FName SocketName;
	
    	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail Parameters")
    	FLinearColor TrailColor = FLinearColor::Red;
    
    	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail Parameters")
    	float TrailWidth = 10.0f;
    
    	// 나이아가라 내부 변수 이름 (고정해두면 편함)
    	//const FName ParamName_Color = FName("User.Color");
    	const FName ParamName_Width = FName("User.Width");
    
public:
    	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
