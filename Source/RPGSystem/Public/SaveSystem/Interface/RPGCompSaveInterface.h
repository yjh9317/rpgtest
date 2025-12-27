// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RPGCompSaveInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Category = "RPG Save", BlueprintType, meta = (DisplayName = "RPG Component Save Interface"))
class URPGCompSaveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IRPGCompSaveInterface
{
	GENERATED_BODY()

public:
	
	/**Executed before the Component is saved.*/
	UFUNCTION(BlueprintNativeEvent, Category = "RPG Save")
	void ComponentPreSave();
	virtual void ComponentPreSave_Implementation() {}

	/**Executed after the Component has been saved.*/
	UFUNCTION(BlueprintNativeEvent, Category = "RPG Save")
	void ComponentSaved();
	virtual void ComponentSaved_Implementation() {}

	/**Executed right before the Component is loaded.*/
	UFUNCTION(BlueprintNativeEvent, Category = "RPG Save")
	void ComponentPreLoad();
	virtual void ComponentPreLoad_Implementation() {}

	/**Executed after the Component has been loaded.*/
	UFUNCTION(BlueprintNativeEvent, Category = "RPG Save")
	void ComponentLoaded();
	virtual void ComponentLoaded_Implementation() {}
};
