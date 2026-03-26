// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "AudioManager.generated.h"
/**
 * 
 */
UCLASS(Blueprintable)
class BUMPURRCARS_API UAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void StartLevelMusic();
	void StopLevelMusic();

protected:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* GameMusic;

private:
	UPROPERTY()
	class UAudioComponent* MusicComponent;

	void HandleMapLoad(UWorld* LoadedWorld, UWorld::InitializationValues init);
};
