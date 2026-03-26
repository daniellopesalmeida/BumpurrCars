// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Hazards/BaseHazard.h"
#include "HazardManagerWorldSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHazardManager, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndGameActivated);

// Ice Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FIceFloorActvated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FIceFloorDeactvated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIceCubeHit, FVector, HitLocation);

// Launchpad Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLaunchpadHit, FVector, HitLocation);

// Speed booster Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpeedBoosterHit, FVector, HitLocation);

// Button events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FButtonHit, TSubclassOf<AActor>, TriggeredHazard, FVector, HitLocation);

// BumpPad events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBumpPadHit, FVector, HitLocation);

USTRUCT(BlueprintType)
struct FHazardSpawnEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDuration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<ABaseHazard>> HazardClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> Weights;
	float TotalHazardWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinHazards{ 1 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHazards{ 1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxOccurence{ 1 };

	int32 TimesOccured;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight;
};

/**
 * 
 */
UCLASS(Blueprintable)
class BUMPURRCARS_API UHazardManagerWorldSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void StartHazardManager();

#pragma region Events
	UPROPERTY(BlueprintAssignable)
	FEndGameActivated OnEndGameActivated;

	// Ice Events
	UPROPERTY(BlueprintAssignable, Category = "IceFloorEvent")
	FIceFloorActvated OnIceFloorActivated;
	UPROPERTY(BlueprintAssignable, Category = "IceFloorEvent")
	FIceFloorDeactvated OnIceFloorDeactivated;
	UPROPERTY(BlueprintAssignable, Category = "IceFloorEvent")
	FIceCubeHit OnIceCubeHit;

	UPROPERTY(BlueprintAssignable, Category = "LaunchpadEvent")
	FLaunchpadHit OnLaunchpadHit;

	//speed booster event
	UPROPERTY(BlueprintAssignable, Category = "SpeedBoosterEvent")
	FSpeedBoosterHit OnSpeedBoosterHit;
	//button event
	UPROPERTY(BlueprintAssignable, Category = "ButtonEvent")
	FButtonHit OnButtonHit;

	UPROPERTY(BlueprintAssignable, Category = "BumpPadEvent")
	FBumpPadHit OnBumpPadHit;
#pragma endregion

protected:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UPROPERTY(EditDefaultsOnly)
	class USoundBase* TransitionSound;
private:
	// Ice Events
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IceFloorEvent", meta = (AllowPrivateAccess = "true"))
	// Effictively is a const value, but can be changed in the editor
	float IceEventDuration{ 5.f };
	FTimerHandle IceEventTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))

	float DelayBetweenEvents{ 1.f };

#pragma region Spawning

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* HazardSpawnIndicatorEffect_Wall;
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* HazardSpawnIndicatorEffect_Ground;
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* HazardSpawnEffect_Wall;
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* HazardSpawnEffect_Ground;

	UPROPERTY()
	TArray<class AHazardSpawnActor*> WallHazardSpawnActors;
	UPROPERTY()

	TArray<class AHazardSpawnActor*> GroundHazardSpawnActors;

	UPROPERTY()
	class AHazardSpawnActor* MeteorSpawnVolume;

	UPROPERTY(EditDefaultsOnly)
	FHazardSpawnEvent FirstSpawnEvent;

	UPROPERTY(EditDefaultsOnly)
	FHazardSpawnEvent LastSpawnEvent;

	UPROPERTY(EditDefaultsOnly)
	bool bRotateGroundHazardsRandomlyAtSpawn{ true };

	UPROPERTY(EditDefaultsOnly)
	float MinRandomRotation{ 0.f };
	UPROPERTY(EditDefaultsOnly)
	float MaxRandomRotation{ 90.f };

	UPROPERTY(EditDefaultsOnly)
	bool bScaleWallHazardsRandomlyAtSpawn{ true };
	UPROPERTY(EditDefaultsOnly)
	FVector MinWallScale{ .80f, .80f, .80f };
	UPROPERTY(EditDefaultsOnly)
	FVector MaxWallScale{ 1.20f, 1.20f, 1.20f };

	UPROPERTY(EditDefaultsOnly)
	bool bFlipWallHazardsXAxisRandomly{ true };

	UPROPERTY(EditDefaultsOnly)
	TArray<FHazardSpawnEvent> SpawnEvents;

	UPROPERTY()
	TArray<ABaseHazard*> SpawnedPermanentHazards;

	FTimerHandle SpawnTimerHandle;
	FHazardSpawnEvent PendingSpawnEvent;

	FTimerHandle OngoingEventTimer;

	float TotalEventWeight{ 0.f };

	UPROPERTY(EditDefaultsOnly)
	float TimeBeforeEventEndForFlickerStart{ 2.f };
	FTimerHandle FlickerHandle;

	UPROPERTY(EditDefaultsOnly)
	float FlickerOnTime{ .3f };
	UPROPERTY(EditDefaultsOnly)
	float FlickerOffTime{ .1f };

	UPROPERTY(EditDefaultsOnly)
	float FlickerOnReductionModifier{ .85f };

	UPROPERTY(EditDefaultsOnly)

	float HazardSpawnDelay{ 1.f };

#pragma endregion

	UFUNCTION()
	void OnIceCubeHitEvent(FVector HitLocation);

	UFUNCTION()
	void OnButtonHitEvent(TSubclassOf<AActor> TriggeredHazard, FVector HitLocation);

	void SpawnHazards(FHazardSpawnEvent& event);
	void ExecuteSpawnLogic();

	void SpawnHazardActor(AHazardSpawnActor* pSpawnPoint, TSubclassOf<ABaseHazard> SelectedHazard, bool bIsWall);

	void SelectNewEventAtEventEnd();

	UFUNCTION()
	void FlickerAllHazardsOff();
	UFUNCTION()
	void FlickerAllHazardsOn();
};
