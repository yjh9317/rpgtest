// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/InteractableDefinitions.h"
#include "InteractionPromptWidget.generated.h"

class UInteractableComponent;
class UTextBlock;
class UImage;
/**
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UInteractionPromptWidget(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setup", meta=(ExposeOnSpawn="true"))
	EInteractionInputType InputType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setup", meta=(ExposeOnSpawn="true"))
	TObjectPtr<UInteractableComponent> InteractableComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", meta=(ExposeOnSpawn="true"))
	FText Description = NSLOCTEXT("InteractionPromptWidget", "Description", "Interaction");

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void BorderFill(float Value);
	UFUNCTION()
	void SetFillDecimalValue(float Value);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Input;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_FillBorder;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Description;
    
private:
	static TObjectPtr<UMaterialInterface> SharedBorderMaterial;
};
