// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerUps/PowerUpManagerWorldSubSystem.h"
#include "PowerUps/PowerUpSpawnAreaActor.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "NiagaraFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogPowerUpManager);

void UPowerUpManagerWorldSubSystem::StartPowerUpManager()
{
	for (auto const& Entry : SpawnAblePowerUps)
	{
		TotalSpawnWeight += Entry.SpawnWeight;
	}

	ensure(TotalSpawnWeight > 0.f);
	UE_LOG(LogPowerUpManager, Warning, TEXT("Total spawn weight: %f"), TotalSpawnWeight);

	for (TActorIterator<APowerUpSpawnAreaActor> It(GetWorld()); It; ++It)
	{
		PowerUpSpawnAreas.Add(*It);
	}

	ensure(PowerUpSpawnAreas.Num() > 0);


	SpawnPowerUp();
}

bool UPowerUpManagerWorldSubSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only trigger initialization code once (the blueprint version)
	return Super::ShouldCreateSubsystem(Outer) && GetClass()->IsInBlueprint();
}

void UPowerUpManagerWorldSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPowerUpManagerWorldSubSystem::Deinitialize()
{
	Super::Deinitialize();

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void UPowerUpManagerWorldSubSystem::SpawnPowerUp()
{
	ensure(PowerUpSpawnAreas.Num() > 0);

	int32 const RandomSpawnAreaIndex{ FMath::RandRange(0, PowerUpSpawnAreas.Num() - 1) };
	APowerUpSpawnAreaActor const* const SelectedSpawnArea{ PowerUpSpawnAreas[RandomSpawnAreaIndex] };

	float const Roll{ FMath::FRandRange(0.f, TotalSpawnWeight) };
	float AccumulatedWeight{ 0.f };
	TSubclassOf<AActor> SelectedPowerUp;

	for (auto const& Entry : SpawnAblePowerUps)
	{
		AccumulatedWeight += Entry.SpawnWeight;
		if (Roll <= AccumulatedWeight)
		{
			UE_LOG(LogPowerUpManager, Warning, TEXT("Roll: %f Acc Weight: %f"), Roll, AccumulatedWeight);

			SelectedPowerUp = Entry.PowerUpClass;
			break;
		}
	}

	if (SelectedPowerUp)
	{
		FVector const SpawnLocation{ SelectedSpawnArea->GetRandomPointInArea() };
		FRotator SpawnRotation = FRotator(0.f, 180.f, 0.f);

		FTransform const SpawnTransform{ SpawnRotation, SpawnLocation };

		if (SpawnIndicator)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				SpawnIndicator,
				SpawnTransform.GetLocation(),
				SpawnTransform.GetRotation().Rotator(),
				SpawnTransform.GetScale3D()
			);
		}

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TWeakObjectPtr<UPowerUpManagerWorldSubSystem> WeakThis(this);
		TimerDel.BindLambda([WeakThis, SelectedPowerUp, SpawnTransform]()
			{
				if (!WeakThis.IsValid()) return;

				UWorld* World{ WeakThis->GetWorld() };
				if (!World) return;

				World->SpawnActor<AActor>(SelectedPowerUp, SpawnTransform);

				if (WeakThis->SpawnEffect)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						World,
						WeakThis->SpawnEffect,
						SpawnTransform.GetLocation(),
						SpawnTransform.GetRotation().Rotator()
					);
				}

				WeakThis->StartSpawnTimer();
			});

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, SpawnDelay, false);
	}
	else
	{
		UE_LOG(LogPowerUpManager, Error, TEXT("Failed to select powerup"));
	}
}

void UPowerUpManagerWorldSubSystem::StartSpawnTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(PowerUpSpawnTimerHandle);

	float const RandomInterval{ static_cast<float>(FMath::FRandRange(SpawnIntervalRange.X, SpawnIntervalRange.Y)) };

	GetWorld()->GetTimerManager().SetTimer(
		PowerUpSpawnTimerHandle,
		this,
		&UPowerUpManagerWorldSubSystem::SpawnPowerUp,
		RandomInterval,
		false // Loop the timer
	);
}
