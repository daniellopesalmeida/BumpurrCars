// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "BumpurrCarsGamemode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBumpurrCarsGamemode, Log, All);


class ABumperCarPawn;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimerUpdateDelagate, int32, RemainingTimeSeconds);

/**
 * 
 */
UCLASS()
class BUMPURRCARS_API ABumpurrCarsGamemode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABumpurrCarsGamemode();

	UPROPERTY(BlueprintAssignable)
	FTimerUpdateDelagate OnTimerUpdate;

	void StartGame();

	// Get the transform to respawn at, since our game just "resets" the player position we only need a transform
	// And do not need to actually destroy & recreate the player.
	[[nodiscard]] FTransform GetRespawnTransform(ABumperCarPawn const * const PlayerActor) const;

	// Returns the cars in radius & their distance from the center.
	[[nodiscard]] TArray<ABumperCarPawn*> GetAllCarsInRadius(FVector const& Location, float Radius) const;

	[[nodiscard]] int32 GetRemainingTime() const { return RemainingTime; }


	[[nodiscard]] UUserWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	virtual void BeginPlay() override;
	virtual void StartPlay() override;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
#pragma region HUD
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	// the class of the HUD we want to spawn
	TSubclassOf<UUserWidget> WBP_HUDClass;
	UPROPERTY()
	// the instance of the HUD
	TObjectPtr<UUserWidget> HUDWidget;
#pragma endregion
#pragma region SettingsAndSetup
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	// The class of the camera we're looking for
	TSubclassOf<ACameraActor> BP_CameraActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameSettings", meta = (AllowPrivateAccess = "true"))
	// Amount of local player playing the game
	int32 NumPlayers{ 4 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameSettings", meta = (AllowPrivateAccess = "true", ToolTip = "In seconds"))
	// Duration of the game in seconds
	int32 GameDuration{ 120 };

	// Remaining time of the game in seconds
	int32 RemainingTime{ 0 };
	// Timer handle to count down the game time (update RemainingTime)

	FTimerHandle GameTimerHandle;

	TArray<class ABumperCarSpawnPoint*> SpawnPoints;
#pragma endregion
#pragma region Countdown
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> CountdownWidgetClass;

	UPROPERTY();
	TObjectPtr<UUserWidget> CountdownWidget;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TArray<UTexture2D*> CountdownTextures;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameSettings", meta = (AllowPrivateAccess = "true", ToolTip = "In seconds"))
	int32 CountDownDuration{ 3 };
	int32 RemainingCountdownTime{ CountDownDuration };
 
	FTimerHandle CountdownTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class USoundBase* CountDownSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class TSubclassOf<UUserWidget> EndUI;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TArray<UTexture2D*> PlayerImages;

#pragma endregion

	// Setup spawn points at the start of the game
	bool InitSpawnPoints();
	// Spawn the players at the start of the game
	void SpawnPlayers();
	// Set the camera up at the start of the game
	void SetupCamera();

	void UpdateGameTimer();

	void EndGame();

	UFUNCTION()
	void OnCountDownTimerHandle();
};
