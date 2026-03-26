// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "BumperCarPlayerController.generated.h"

struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogBumperCarController, Log, All);

/**
 * 
 */
// The connection between whatever the player does & what happens in the game
UCLASS()
class BUMPURRCARS_API ABumperCarPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Set the input mapping for this controller, can not be done in BeginPlay since it is necessary that the player is "possesed" first.
	void SetupInput();

	UFUNCTION(BlueprintCallable, Category = "Controler")
	void SetInputModeToGameplay();

	UFUNCTION(BlueprintCallable, Category = "Controler")
	void SetInputModeToUI();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	// Mapping contexts
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> GameplayMappingContext { nullptr };

	//UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UInputMappingContext> UIMappingContext { nullptr };


	// Input Actions
	UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AccelerateAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ReverseAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TurnAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> PauseAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> BoostAction;

	//UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UInputAction> UINavigationAction;

	//UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UInputAction> UIBackAction;

	//UPROPERTY(EditDefaultsOnly, Category = "Input|GameplayActions", meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UInputAction> UIAcceptAction;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> WBP_PauseScreen;

	// Input action handling
	void HandleBoostCar();
	void HandlePauseGame();
	void HandleAccelerateCar(const FInputActionValue& Value);
	void HandleReverseCar(const FInputActionValue& Value);
	void HandleTurnCar(const FInputActionValue& Value);

	void HandleUINavigation(const FInputActionValue& Value);

	void HandleUIDown();
	void HandleUIUp();

	void HandleUIBack();
	void HandleUIAccept();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> WidgetClass;
};
