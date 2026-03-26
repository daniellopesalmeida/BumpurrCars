// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/TextBlock.h>
#include <Characters/BumperCarPawn.h>
#include <Components/Image.h>
#include "GameHud.generated.h"


/**
 * 
 */
UCLASS()
class BUMPURRCARS_API UGameHud : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializeFromManager(class UScoreManagerWorldSubsystem* Manager);

	UFUNCTION()
	void UpdatePlayerScore(ABumperCarPawn* Car, int32 NewScore, bool IsLoss);

	UFUNCTION()
	UImage* GetPlayerScoreMultiplierImage(int32 PlayerIndex) const;
	UFUNCTION()
	UImage* GetPlayerBumperStrengthImage(int32 PlayerIndex) const;
	UFUNCTION()
	UImage* GetPlayerShieldImage(int32 PlayerIndex) const;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player1ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player2ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player3ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player4ScoreText;

	TMap<ABumperCarPawn*, UTextBlock*> PlayerScoreMap;

	TMap<int32, UImage*> PlayerScoreMultiplierImages;
	TMap<int32, UImage*> PlayerBumperStrengthImages;
	TMap<int32, UImage*> PlayerShieldImages;

	// Player 1
	UPROPERTY(meta = (BindWidget))
	UImage* P1ScoreMultImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P1BumperStrengthImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P1ShieldImage;

	// Player 2 
	UPROPERTY(meta = (BindWidget))
	UImage* P2ScoreMultImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P2BumperStrengthImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P2ShieldImage;

	// Player 3
	UPROPERTY(meta = (BindWidget))
	UImage* P3ScoreMultImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P3BumperStrengthImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P3ShieldImage;

	// Player 4
	UPROPERTY(meta = (BindWidget))
	UImage* P4ScoreMultImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P4BumperStrengthImage;
	UPROPERTY(meta = (BindWidget))
	UImage* P4ShieldImage;
};
