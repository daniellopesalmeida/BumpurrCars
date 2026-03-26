// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreComponent.generated.h"

enum class EImpactLevel : uint8;
class ABumperCarPawn;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreChanged, ABumperCarPawn*, Car, int32, NewScore, bool, IsLoss);


// Score component takes care of all score related logic.
// This class is tightly coupled with the HUD elements, which will respond to the OnScoreChanged event.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUMPURRCARS_API UScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UScoreComponent();

	// The players hit logic has to be dealth with separately, this effectively decides when the player is, or is not in the "gaining score state"
	void ReducePlayersHit();
	void IncreasePlayersHit();

	// On score changed event, hook the UI up to this
	UPROPERTY(BlueprintAssignable, Category = "Score")
	FOnScoreChanged OnScoreChanged;

	[[nodiscard]] int32 GetScore() const noexcept { return PlayerScore; }

	UFUNCTION()
	void OnSpikesReduceScore();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	// How many players did this person currently register as being "hit"
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score|Logic", meta = (AllowPrivateAccess = "true"))
	int32 PlayersHit{ 0 };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score|Logic", meta = (AllowPrivateAccess = "true"))
	ABumperCarPawn* LastHitByPlayer{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score|Logic", meta = (AllowPrivateAccess = "true"))
	// The base score the player currently has from the last interactions
	int32 CurrentGainedScoreBase{ 0 };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score|Logic", meta = (AllowPrivateAccess = "true"))
	// The score mutlipleir the player currently has gained from the last interactions
	int32 CurrentGainedScoreMultiplier{ 1 };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = "true"))
	// The score the player currently has
	int32 PlayerScore{ 0 };

#pragma region Collisions
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 GainOnLight{ 300 };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 GainOnMedium{ 500 };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 GainOnHeavy{ 1000 };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 GainOnVeryHeavy{ 10000 };
#pragma endregion

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 PercentageLossOnOffMap{ 25 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 MultiplierGainOnOffMap{ 10 };

#pragma region Hazards
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 LossOnMeteorHit{ 2000 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 MultiplierOnMeteorHit{ 2 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 LossOnSpikesHit{ 1000 };
#pragma endregion

	UFUNCTION()
	void OnPlayerWonCollision(EImpactLevel ImpactLevel, ABumperCarPawn* Winner, ABumperCarPawn* Loser, FVector const& ImpactPoint);

	UFUNCTION()
	void OnPlayerHeadOnDraw(EImpactLevel ImpactLevel, ABumperCarPawn* Other, FVector const& ImpactLocation);

	UFUNCTION()
	void OnPlayerOffMap();

	UFUNCTION()
	void OnControlRestore();

	UFUNCTION()
	void OnMeteorHit();

	void AddBaseScore(EImpactLevel ImpactLevel);
	void AddMultiplier(int32 multiplier);

	// Whenever the player is not in the "gaining score state", we reset all the gained score variables and add the score.
	UFUNCTION()
	void AddScore();
	// Whenever a player hits certain obstacles, they lose score, this is removed immediately.
	void RemoveScore(uint32 Amount, bool bIsPercentage = false);

	void ResetLastHitPlayer();

	// Set the last hit player to a new one and do all the necessary checks & updates
	bool OverwriteLastHitPlayer(ABumperCarPawn* NewLastHit);

	FTimerHandle AddScoreHandle;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	float ComboTime{ .4f };
};


