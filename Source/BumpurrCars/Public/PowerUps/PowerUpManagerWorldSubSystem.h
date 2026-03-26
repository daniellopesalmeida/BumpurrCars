// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PowerUpManagerWorldSubSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPowerUpManager, Log, All);

USTRUCT(BlueprintType)
struct FPowerUpEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> PowerUpClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// Give PowerUps more or less weight to spawn
	float SpawnWeight = 1.0f;
};

/**
 * 
 */
UCLASS(Blueprintable)
class BUMPURRCARS_API UPowerUpManagerWorldSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void StartPowerUpManager();

protected:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
	TArray<FPowerUpEntry> SpawnAblePowerUps;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUp Spawn")
	FVector2D SpawnIntervalRange{ 30.0f, 40.0f };

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* SpawnEffect;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* SpawnIndicator;

	UPROPERTY(EditDefaultsOnly)
	float SpawnDelay{ 2.f };

private:
	void SpawnPowerUp();
	void StartSpawnTimer();

	FTimerHandle PowerUpSpawnTimerHandle;

	float TotalSpawnWeight = 0.f;


	UPROPERTY()
	TArray<class APowerUpSpawnAreaActor*> PowerUpSpawnAreas;
};
