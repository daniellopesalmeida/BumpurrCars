// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/HazardManagerWorldSubsystem.h"
#include "Components/BoxComponent.h"
#include "Hazards/MeteorHazardActor.h"
#include "Kismet/GameplayStatics.h"
#include "MeteorSpawnVolume.h"
#include "EngineUtils.h"
#include <Environment/WallManagerWorldSubsystem.h>

#include "NiagaraFunctionLibrary.h"
#include "Core/BumpurrCarsGamemode.h"
#include "Hazards/HazardSpawnActor.h"
DEFINE_LOG_CATEGORY(LogHazardManager);


void UHazardManagerWorldSubsystem::StartHazardManager()
{
    // Bind all necessary events
    OnIceCubeHit.AddDynamic(this, &UHazardManagerWorldSubsystem::OnIceCubeHitEvent);
    OnButtonHit.AddDynamic(this, &UHazardManagerWorldSubsystem::OnButtonHitEvent);

    // get spawnpoints
    for (TActorIterator<AHazardSpawnActor> It(GetWorld()); It; ++It)
    {
	    switch (It->GetSpawnPointType())
    	{
	    case EHazardSpawnPointType::Ground:
            GroundHazardSpawnActors.Add(*It);
		    break;
	    case EHazardSpawnPointType::Wall:
            WallHazardSpawnActors.Add(*It);
		    break;
	    case EHazardSpawnPointType::GroundAndWall:
            WallHazardSpawnActors.Add(*It);
            GroundHazardSpawnActors.Add(*It);
		    break;
	    case EHazardSpawnPointType::Other:
		    break;
	    }
    }

    //Setup meteor spawn vol
    for (TActorIterator<AMeteorSpawnVolume> It(GetWorld()); It; ++It)
    {
        MeteorSpawnVolume = *It;
        break; // Only grab the first one found
    }

    if (!MeteorSpawnVolume)
    {
        UE_LOG(LogHazardManager, Error, TEXT("No MeteorSpawnVolume found in the level!"));
    }

    //Set the weights
    for (auto W : FirstSpawnEvent.Weights)
    {
        FirstSpawnEvent.TotalHazardWeight += W;
    }
    ensureMsgf(FirstSpawnEvent.Weights.Num() == FirstSpawnEvent.HazardClasses.Num(), TEXT("Mismatch in weights and hazard classes for event %s"), *FirstSpawnEvent.Name);

    for (auto& Event : SpawnEvents)
    {
	    for (auto W : Event.Weights)
	    {
            Event.TotalHazardWeight += W;
	    }

        TotalEventWeight += Event.Weight;
        ensureMsgf(Event.Weights.Num() == Event.HazardClasses.Num(), TEXT("Mismatch in weights and hazard classes for event %s"), *Event.Name);
    }

    for (auto W : LastSpawnEvent.Weights)
    {
        LastSpawnEvent.TotalHazardWeight += W;
    }
    ensureMsgf(LastSpawnEvent.Weights.Num() == LastSpawnEvent.HazardClasses.Num(), TEXT("Mismatch in weights and hazard classes for event %s"), *LastSpawnEvent.Name);

    ensure(GroundHazardSpawnActors.Num() > 0);
    ensure(WallHazardSpawnActors.Num() > 0);

    // Sppawn the first wave
    SpawnHazards(FirstSpawnEvent);
}

bool UHazardManagerWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only trigger initialization code once (the blueprint version)
    return Super::ShouldCreateSubsystem(Outer) && GetClass()->IsInBlueprint();
}

void UHazardManagerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UHazardManagerWorldSubsystem::Deinitialize()
{
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Super::Deinitialize();
}

void UHazardManagerWorldSubsystem::OnIceCubeHitEvent(FVector HitLocation)
{
	auto TimerDuration{ IceEventDuration };

    // In case event is already ongoing, just add more time to the event
	if (GetWorld()->GetTimerManager().IsTimerActive(IceEventTimer))
	{
        TimerDuration += GetWorld()->GetTimerManager().GetTimerRemaining(IceEventTimer);
        GetWorld()->GetTimerManager().ClearTimer(IceEventTimer);
	}
	else
	{
        // Broadcast ice floor begin
        OnIceFloorActivated.Broadcast();
	}

    // Default params are used
	FTimerManagerTimerParameters constexpr TimerParams{};

    GetWorld()->GetTimerManager().SetTimer(IceEventTimer, 
        [this]()
        {
	        OnIceFloorDeactivated.Broadcast();
        }, 
        TimerDuration, TimerParams);
}

void UHazardManagerWorldSubsystem::OnButtonHitEvent(TSubclassOf<AActor> TriggeredHazard, FVector HitLocation)
{
    if (!TriggeredHazard) // Ensure the class is valid
    {
        UE_LOG(LogTemp, Warning, TEXT("OnButtonHitEvent: Invalid hazard class!"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (TriggeredHazard->IsChildOf(AMeteorHazardActor::StaticClass()))
    {
        FRotator const SpawnRotation{ -90.0, 0.0, 0.0 }; // Faces downward
        AActor* SpawnedHazard{ GetWorld()->SpawnActor<AActor>(TriggeredHazard, MeteorSpawnVolume ? MeteorSpawnVolume->GetRandomPointInVolume() : HitLocation , SpawnRotation, SpawnParams)};
    }
    else
    {
		AActor* SpawnedHazard{ GetWorld()->SpawnActor<AActor>(TriggeredHazard, HitLocation, FRotator::ZeroRotator, SpawnParams) };
    }

    if (GetWorld()->GetSubsystem<UWallManagerWorldSubsystem>())
    {
        GetWorld()->GetSubsystem<UWallManagerWorldSubsystem>()->OpenWallFromButton();
        UE_LOG(LogHazardManager, Warning, TEXT("Triggered wall opening from button press."));
    }
    else
    {
        UE_LOG(LogHazardManager, Warning, TEXT("No wall event trrigered."));
    }
}

void UHazardManagerWorldSubsystem::SpawnHazards(FHazardSpawnEvent& event)
{
    // Clears hazards & sets the next spawn

    for (auto& Actor : SpawnedPermanentHazards)
    {
	    if (IsValid(Actor))
	    {
			Actor->Destroy();
	    }
    }


    SpawnedPermanentHazards.Empty();
    PendingSpawnEvent = event;

    GetWorld()->GetTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &UHazardManagerWorldSubsystem::ExecuteSpawnLogic,
        DelayBetweenEvents,
        false
    );
}

void UHazardManagerWorldSubsystem::ExecuteSpawnLogic()
{
    float const Duration{ FMath::RandRange(PendingSpawnEvent.MinDuration, PendingSpawnEvent.MaxDuration) };
    GetWorld()->GetTimerManager().ClearTimer(OngoingEventTimer);
    GetWorld()->GetTimerManager().SetTimer(OngoingEventTimer, this, &UHazardManagerWorldSubsystem::SelectNewEventAtEventEnd, Duration);

    UE_LOG(LogHazardManager, Warning, TEXT("new event Started: %s | Duration: %f"), *PendingSpawnEvent.Name, Duration)

    GetWorld()->GetTimerManager().SetTimer(FlickerHandle, this, &UHazardManagerWorldSubsystem::FlickerAllHazardsOff, Duration - TimeBeforeEventEndForFlickerStart);

    // Make copies so we can ensure one spawn area is only used once.
    TArray<AHazardSpawnActor*> GroundCopy{ GroundHazardSpawnActors };
    TArray<AHazardSpawnActor*> WallCopy{ WallHazardSpawnActors };

    int32 AmountOfHazardsToSpawn{ FMath::RandRange(PendingSpawnEvent.MinHazards, PendingSpawnEvent.MaxHazards) };
    for (int32 HazToSpawn{ 0 }; HazToSpawn < AmountOfHazardsToSpawn; ++HazToSpawn)
    {
        float const Roll{ FMath::RandRange(0.f, PendingSpawnEvent.TotalHazardWeight) };
        float AccumulatedWeight{ 0.f };
        TSubclassOf<ABaseHazard> SelectedHazard{ nullptr };

        EHazardType SpawnType{ EHazardType::Other };

        while (not SelectedHazard)
        {
            for (int32 i{ 0 }; i < PendingSpawnEvent.Weights.Num(); ++i)
            {
                AccumulatedWeight += PendingSpawnEvent.Weights[i];
                if (Roll <= AccumulatedWeight)
                {
                    SelectedHazard = PendingSpawnEvent.HazardClasses[i];

                    ABaseHazard const* DefaultHazard{ SelectedHazard->GetDefaultObject<ABaseHazard>() };
                    SpawnType = DefaultHazard->GetHazardType();

                    if (not (SpawnType == EHazardType::Ground 
                        or SpawnType == EHazardType::Wall
                        or SpawnType == EHazardType::GroundAndWall))
                    {
                        UE_LOG(LogHazardManager, Error, TEXT("Invalid hazard type"))
                    }

                    if (GroundCopy.IsEmpty() and WallCopy.IsEmpty())
                    {
                        return;
                    }

                    if (SpawnType == EHazardType::Ground && GroundCopy.IsEmpty())
                    {
                        SelectedHazard = nullptr;
                    }

                    if (SpawnType == EHazardType::Wall && WallCopy.IsEmpty())
                    {
                        SelectedHazard = nullptr;
                    }

                    break;
                }
            }
        }

        switch (SpawnType)
        {
        case EHazardType::Ground:
        {
            if (GroundCopy.IsEmpty())
            {
                UE_LOG(LogHazardManager, Error, TEXT("No ground hazard spawn points available!"));
            	continue;
            }
            int32 const RandomIndex{ FMath::RandRange(0, GroundCopy.Num() - 1) };
            AHazardSpawnActor* SelectedSpawnPoint = GroundCopy[RandomIndex];
            SpawnHazardActor(SelectedSpawnPoint, SelectedHazard, false);

            GroundCopy.Swap(RandomIndex, GroundCopy.Num() - 1);
            GroundCopy.Pop();
        }
        break;
        case EHazardType::Wall:
        {
            if (WallCopy.IsEmpty())
            {
                UE_LOG(LogHazardManager, Error, TEXT("No wall hazard spawn points available!"));
            	continue;
            }

            int32 const RandomIndex{ FMath::RandRange(0, WallCopy.Num() - 1) };
            AHazardSpawnActor* SelectedSpawnPoint{ WallCopy[RandomIndex] };
            SpawnHazardActor(SelectedSpawnPoint, SelectedHazard, true);

            WallCopy.Swap(RandomIndex, WallCopy.Num() - 1);
            WallCopy.Pop();
        }
        break;
        case EHazardType::GroundAndWall:
        {
            if (GroundCopy.IsEmpty() and WallCopy.IsEmpty())
            {
                UE_LOG(LogHazardManager, Error, TEXT("No ground and wall hazard spawn points available!"));
                return;
            }

            bool const PreferGround{ FMath::RandBool() };
            if (not GroundCopy.IsEmpty() and (WallCopy.IsEmpty() or PreferGround))
            {
                int32 const RandomIndex{ FMath::RandRange(0, GroundCopy.Num() - 1) };
                AHazardSpawnActor* SelectedSpawnPoint{ GroundCopy[RandomIndex] };
                SpawnHazardActor(SelectedSpawnPoint, SelectedHazard, false);

                GroundCopy.Swap(RandomIndex, GroundCopy.Num() - 1);
                GroundCopy.Pop();
            }
            else
            {
                int32 const RandomIndex{ FMath::RandRange(0, WallCopy.Num() - 1) };
                AHazardSpawnActor* SelectedSpawnPoint{ WallCopy[RandomIndex] };
                SpawnHazardActor(SelectedSpawnPoint, SelectedHazard, true);

                WallCopy.Swap(RandomIndex, WallCopy.Num() - 1);
                WallCopy.Pop();
            }
        }
        break;
        case EHazardType::Other:
            break;
        default:
            break;
        }
    }
}

void UHazardManagerWorldSubsystem::SpawnHazardActor(AHazardSpawnActor* pSpawnPoint, TSubclassOf<ABaseHazard> SelectedHazard, bool bIsWall)
{
    ensure(pSpawnPoint);

    FVector const SpawnLocation{ pSpawnPoint->GetRandomPointInVolume() };
    FRotator SpawnRotation{ pSpawnPoint->GetActorRotation() };

    if (not bIsWall and bRotateGroundHazardsRandomlyAtSpawn)
    {
        SpawnRotation += FRotator{ 0.f, FMath::RandRange(MinRandomRotation, MaxRandomRotation), 0.f };
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    FVector SpawnScale{
    	bScaleWallHazardsRandomlyAtSpawn and bIsWall ? FMath::RandRange(MinWallScale.X, MaxWallScale.X) : 1.f,
    	bScaleWallHazardsRandomlyAtSpawn and bIsWall ? FMath::RandRange(MinWallScale.Y, MaxWallScale.Y) : 1.f,
    	bScaleWallHazardsRandomlyAtSpawn and bIsWall ? FMath::RandRange(MinWallScale.Z, MaxWallScale.Z) : 1.f
    };

    if (bIsWall and bFlipWallHazardsXAxisRandomly)
    {
        if (FMath::RandBool())
        {
            SpawnScale.Y *= -1.f;
        }
    }

    FTransform const SpawnTransform{ SpawnRotation, SpawnLocation, SpawnScale };

    if ( (bIsWall ? HazardSpawnIndicatorEffect_Wall : HazardSpawnIndicatorEffect_Ground) )
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            bIsWall ? HazardSpawnIndicatorEffect_Wall : HazardSpawnIndicatorEffect_Ground,
            SpawnTransform.GetLocation(),
            SpawnTransform.GetRotation().Rotator(),
            SpawnTransform.GetScale3D()
        );
    }

    FTimerHandle TimerHandle;
    FTimerDelegate TimerDel;
    TWeakObjectPtr<UHazardManagerWorldSubsystem> WeakThis(this);
    TimerDel.BindLambda([WeakThis, SelectedHazard, SpawnTransform, SpawnParams, bIsWall]()
        {
            if (!WeakThis.IsValid()) return;

            UWorld* World{ WeakThis->GetWorld() };
            if (!World) return;

            auto* Haz{ World->SpawnActor<ABaseHazard>(SelectedHazard, SpawnTransform, SpawnParams) };
            ensure(Haz);
            if (IsValid(Haz))
            {
                WeakThis->SpawnedPermanentHazards.Add(Haz);

                if ((bIsWall ? WeakThis->HazardSpawnEffect_Wall : WeakThis->HazardSpawnEffect_Ground))
                {
                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                        World,
                        bIsWall ? WeakThis->HazardSpawnEffect_Wall : WeakThis->HazardSpawnEffect_Ground,
                        SpawnTransform.GetLocation(),
                        SpawnTransform.GetRotation().Rotator(),
                        SpawnTransform.GetScale3D()
                    );
                }
            }
            else
            {
                UE_LOG(LogHazardManager, Error, TEXT("Invalid spawned hazard!"));
            }
        });

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, HazardSpawnDelay, false);
}

void UHazardManagerWorldSubsystem::SelectNewEventAtEventEnd()
{
    GetWorld()->GetTimerManager().ClearTimer(FlickerHandle);

    if (TransitionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TransitionSound, {});
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Transition sound effect not set"));
    }

    auto const RemainingTime{ Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode())->GetRemainingTime() };
    bool const bIsEndGame{ RemainingTime <= LastSpawnEvent.MaxDuration };
	if (bIsEndGame)
	{
        // Ensure event lasts for rest of game
        LastSpawnEvent.MinDuration = RemainingTime * 2;
        LastSpawnEvent.MaxDuration = RemainingTime * 2;
        SpawnHazards(LastSpawnEvent);

        OnEndGameActivated.Broadcast();

        return;
	}

    if (SpawnEvents.IsEmpty())
    {
        SpawnHazards(FirstSpawnEvent);
        UE_LOG(LogHazardManager, Warning, TEXT("Spawn events is empty, re using first event! "));
        return;
    }

    float const Roll{ FMath::RandRange(0.f, TotalEventWeight) };
    float AccumulatedWeight{ 0.f };

	FHazardSpawnEvent SelectedSpawnEvent{};

    
    for (int32 i{ 0 }; i < SpawnEvents.Num(); ++i)
    {
        AccumulatedWeight += SpawnEvents[i].Weight;
        if (Roll <= AccumulatedWeight)
        {
            SelectedSpawnEvent = SpawnEvents[i];

            SelectedSpawnEvent.TimesOccured++;
            if (SelectedSpawnEvent.TimesOccured >= SelectedSpawnEvent.MaxOccurence)
            {
                TotalEventWeight -= SelectedSpawnEvent.Weight;
            	SpawnEvents.Swap(i, SpawnEvents.Num() - 1);
                SpawnEvents.Pop();
            }
            else
            {
                SpawnEvents[i].Weight /= 2.f;
                TotalEventWeight -= SpawnEvents[i].Weight;
            }

            break;
        }
    }

    SpawnHazards(SelectedSpawnEvent);
}

void UHazardManagerWorldSubsystem::FlickerAllHazardsOff()
{
    for (auto& Actor : SpawnedPermanentHazards)
    {
	    if (IsValid(Actor))
	    {
			Actor->SetActorHiddenInGame(true);
	    }
    }

    GetWorld()->GetTimerManager().SetTimer(FlickerHandle, this, &UHazardManagerWorldSubsystem::FlickerAllHazardsOn, FlickerOffTime);
}

void UHazardManagerWorldSubsystem::FlickerAllHazardsOn()
{
    for (auto& Actor : SpawnedPermanentHazards)
    {
        if (IsValid(Actor))
        {
            Actor->SetActorHiddenInGame(false);
        }
    }
    GetWorld()->GetTimerManager().SetTimer(FlickerHandle, this, &UHazardManagerWorldSubsystem::FlickerAllHazardsOff, FlickerOnTime);
    FlickerOnTime *= FlickerOnReductionModifier;
}
