// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScoreManagerWorldSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogScoreManager, Log, All);

class ABumperCarPawn;

/**
 * 
 */
UCLASS(Blueprintable)
class BUMPURRCARS_API UScoreManagerWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void StartScoreManager(int32 NumPlayers);

	const TArray<ABumperCarPawn*>& GetAllPlayers() const { return Players; }

	UFUNCTION(BlueprintCallable, Category = "Score")
	TArray<ABumperCarPawn*> GetPlayersSortedByScore() const;

protected:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// Array of players we track the score for
	TArray<ABumperCarPawn*> Players;

	int32 HighScorePlayerID{ -1 };

	UFUNCTION()
	void OnScoreChanged(ABumperCarPawn* Car, int32 NewScore, bool IsLoss);

};
