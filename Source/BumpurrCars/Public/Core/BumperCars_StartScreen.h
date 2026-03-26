// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "BumperCars_StartScreen.generated.h"

/**
 * 
 */
UCLASS()
class BUMPURRCARS_API ABumperCars_StartScreen : public AGameModeBase
{
	GENERATED_BODY()
	
	void StartGame();

protected:
	virtual void BeginPlay() override;
	virtual void StartPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> WBP_StartScreen;

	TObjectPtr<UUserWidget> StartScreenWidget;
};
