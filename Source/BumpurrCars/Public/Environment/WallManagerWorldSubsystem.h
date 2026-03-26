// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WallManagerWorldSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWallManager, Log, All);

class AWallPieceActor;

/**
 * 
 */
UCLASS(Blueprintable)
class BUMPURRCARS_API UWallManagerWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void StartRandomWallOpener();
	void OpenWallFromButton();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;



private:
	void OpenRandomWall();
	void CloseWall(AWallPieceActor* Wall);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Management", meta = (AllowPrivateAccess = "true"))
	float MinTimeOpenBeforeClose{ 3.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Management", meta = (AllowPrivateAccess = "true"))
	float MaxTimeOpenBeforeClose{ 7.f };


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Management", meta = (AllowPrivateAccess = "true"))
	float MinTimeNextWallOpen{ 3.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Management", meta = (AllowPrivateAccess = "true"))
	float MaxTimeNextWallOpen{ 7.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Management", meta = (AllowPrivateAccess = "true"))
	int32 MaxOpenWallsAtOnce{ 1 };

	// Timer handle to schedule random wall openings
	FTimerHandle WallOpenTimerHandle;

	bool bAllWallsOpenedByButton{false};

	struct FOpenWall
	{
		AWallPieceActor* Wall; // Reference to the wall that is open
		FTimerHandle TimerHandle; // Timer handle for closing the wall
	};

	TArray<FOpenWall> CurrentlyOpenWalls{};
	TArray<AWallPieceActor*> CurrentlyClosedWalls{};
};
