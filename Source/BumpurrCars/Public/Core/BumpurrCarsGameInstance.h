// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include "GameFramework/GameModeBase.h"
#include "BumpurrCarsGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameInstance, Log, All);


/**
 * 
 */
UCLASS()
class BUMPURRCARS_API UBumpurrCarsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Game")
    void ReloadLevel();

	//UFUNCTION(BlueprintCallable, Category = "Input")
	//bool IsControllerConnected(int32 playerID);

	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetVisualID(int32 ControllerID) const
	{
		ensure(ControllerID_VisualIDMap.Contains(ControllerID));
		return *ControllerID_VisualIDMap.Find(ControllerID);
	}

	UFUNCTION(BlueprintCallable, Category = "Game")
	void InsertVisualID(int32 ControllerID, int32 VisualID)
	{
		ControllerID_VisualIDMap.Emplace(ControllerID, VisualID);
	}

protected:
	virtual void Init() override;

	UPROPERTY(VisibleAnywhere)
	TMap<int32, int32> ControllerID_VisualIDMap
	{
		{ 0, 0 },
		{ 1, 1 },
		{ 2, 2 },
		{ 3, 3 }
	};

private:
	void OnLevelLoaded(UWorld* World, const UWorld::InitializationValues IVS);
};
