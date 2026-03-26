// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BumpurrCarsGamemode.h"

#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "Blueprint/UserWidget.h"

#include "Core/BumperCarSpawnPoint.h"
#include "Core/BumperCarPlayerController.h"

#include "Characters/BumperCarPawn.h"
#include "Components/Image.h"
#include "Core/AudioManager.h"
#include "Core/BumpurrCarsGameInstance.h"

#include "Environment/WallManagerWorldSubsystem.h"

#include "Hazards/HazardManagerWorldSubsystem.h"

#include "Core/ScoreManagerWorldSubsystem.h"

#include "PowerUps/PowerUpManagerWorldSubSystem.h"
#include "Core/ScoreComponent.h"

DEFINE_LOG_CATEGORY(LogBumpurrCarsGamemode);


ABumpurrCarsGamemode::ABumpurrCarsGamemode()
{ }

void ABumpurrCarsGamemode::StartGame()
{
    for (int32 PlayerIndex{ 0 }; PlayerIndex < NumPlayers; ++PlayerIndex)
    {
        APlayerController* PlayerController{ UGameplayStatics::GetPlayerControllerFromID(GetWorld(), PlayerIndex)};
        if (ABumperCarPlayerController * PC{ Cast<ABumperCarPlayerController>(PlayerController) })
        {
            PC->SetupInput();
            PC->SetInputModeToGameplay();
        }
    }

    // Start the world subsystems
    if (UHazardManagerWorldSubsystem * HazardManager{ GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>() })
    {
    	HazardManager->StartHazardManager();
        UE_LOG(LogHazardManager, Log, TEXT("HazardManagerWorldSubsystem started!"));
    }
    else
    {
        UE_LOG(LogHazardManager, Error, TEXT("HazardManagerWorldSubsystem not found!"));
    }

    if (UPowerUpManagerWorldSubSystem * PowerUpManager{ GetWorld()->GetSubsystem<UPowerUpManagerWorldSubSystem>() })
    {
        PowerUpManager->StartPowerUpManager();
        UE_LOG(LogPowerUpManager, Log, TEXT("PowerUpManagerWorldSubSystem started!"));
    }
    else
    {
        UE_LOG(LogPowerUpManager, Error, TEXT("PowerUpManagerWorldSubSystem not found!"));
    }

    if (UAudioManager* AudioManager{ GetGameInstance()->GetSubsystem<UAudioManager>() })
    {
        AudioManager->StartLevelMusic();
    }
    else
    {
        UE_LOG(LogAudio, Error, TEXT("AudioManager not found!"));
    }

    GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ABumpurrCarsGamemode::UpdateGameTimer, 1.0f, true);
}

FTransform ABumpurrCarsGamemode::GetRespawnTransform(ABumperCarPawn const * const PlayerActor) const
{
    ensure(IsValid(PlayerActor));
    checkf(!SpawnPoints.IsEmpty(), TEXT("Spawn points are empty on respawn"));

    TArray<AActor*> PlayerPawns{ };

    // Get all player-controlled pawns (not including the player we are trying to respawn)
    for (FConstPlayerControllerIterator It{ GetWorld()->GetPlayerControllerIterator() }; It; ++It)
    {
        APlayerController const* const PC{ It->Get() };
        if (PC && PC->GetPawn())
        {
            if (PC->GetPawn() == Cast<APawn>(PlayerActor))
            {
                continue;
            }
            PlayerPawns.Add(PC->GetPawn());
        }
    }

    float MaxMinDistance{ 0.0f };

    ABumperCarSpawnPoint* FurthestSpawnPoint{ nullptr };

    for (auto const Spawn : SpawnPoints)
    {
        FVector const SpawnLocation{ Spawn->GetActorLocation() };
        float MinDistanceToPlayers{ MAX_flt };

        for (AActor const* const Player : PlayerPawns)
        {
            float const Distance{ static_cast<float>(FVector::Dist(SpawnLocation, Player->GetActorLocation())) };
            MinDistanceToPlayers = FMath::Min(MinDistanceToPlayers, Distance);

        }

        // Keep track of the spawn point with the maximum minimum distance
        if (MinDistanceToPlayers > MaxMinDistance)
        {
            MaxMinDistance = MinDistanceToPlayers;
            FurthestSpawnPoint = Spawn;
        }
    }

    return FurthestSpawnPoint->GetTransform();
}

TArray<ABumperCarPawn*> ABumpurrCarsGamemode::GetAllCarsInRadius(FVector const& Location, float Radius) const
{
    TArray<ABumperCarPawn*> FoundCars{};

    for (TActorIterator<ABumperCarPawn> It(GetWorld()); It; ++It)
    {
        if (auto Car{ *It }; Car)
        {
            ensure(IsValid(Car));

            float const Distance{ static_cast<float>(FVector::Dist(Location, Car->GetActorLocation())) };
            if (Distance <= Radius)
            {
				FoundCars.Add(Car);
            }
        }
    }

    return FoundCars;
}

void ABumpurrCarsGamemode::BeginPlay()
{
    Super::BeginPlay();
}

void ABumpurrCarsGamemode::StartPlay()
{
    Super::StartPlay();

    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto const bInitializedSpawnPoints{ InitSpawnPoints() };
    check(bInitializedSpawnPoints);

    SpawnPlayers();
    SetupCamera();

    if (UScoreManagerWorldSubsystem * ScoreManager{ GetWorld()->GetSubsystem<UScoreManagerWorldSubsystem>() })
    {
        ScoreManager->StartScoreManager(NumPlayers);
        UE_LOG(LogScoreManager, Log, TEXT("ScoreManagerWorldSubsystem started!"));
    }
    else
    {
        UE_LOG(LogScoreManager, Error, TEXT("ScoreManagerWorldSubsystem not found!"));
    }

    check(WBP_HUDClass);

    if (WBP_HUDClass)
    {
        HUDWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_HUDClass);

        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
        }
    }

    check(CountdownWidgetClass);

    CountdownWidget = CreateWidget<UUserWidget>(GetWorld(), CountdownWidgetClass);

    if (CountdownWidget)
    {
        CountdownWidget->AddToViewport();
        UImage* CountdownImage{ Cast<UImage>(CountdownWidget->GetWidgetFromName(TEXT("CountdownImage"))) };
        if (CountdownImage)
        {
            CountdownImage->SetBrushFromTexture(CountdownTextures[0]);
        }
    }
    RemainingTime = GameDuration;
    OnTimerUpdate.Broadcast(RemainingTime);

    RemainingCountdownTime = CountDownDuration;
    GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ABumpurrCarsGamemode::OnCountDownTimerHandle, 1.f, true);

    if (CountDownSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CountDownSound, {});
    }

    

    if (UWallManagerWorldSubsystem * WallManager{ GetWorld()->GetSubsystem<UWallManagerWorldSubsystem>() })
    {
        WallManager->StartRandomWallOpener();
        UE_LOG(LogWallManager, Log, TEXT("WallManagerWorldSubsystem started!"));
    }
    else
    {
        UE_LOG(LogWallManager, Error, TEXT("WallManagerWorldSubsystem not found!"));
    }
}

APawn* ABumpurrCarsGamemode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    // Prevent Unreal from automatically spawning a pawn

    return nullptr;
}

AActor* ABumpurrCarsGamemode::ChoosePlayerStart_Implementation(AController* Player)
{
    return nullptr;
}

bool ABumpurrCarsGamemode::InitSpawnPoints()
{
    SpawnPoints.Empty();

    if (UWorld const* const World{ GetWorld() })
    {
        for (TActorIterator<ABumperCarSpawnPoint> It{ World }; It; ++It)
        {
            UE_LOG(LogBumpurrCarsGamemode, Warning, TEXT("Found spawn point at location: %s"), *It->GetActorLocation().ToString());
            check(*It);
            SpawnPoints.Add((*It));
        }
    }

    if (SpawnPoints.Num() < NumPlayers)
    {
        return false;
    }

    return true;
}

void ABumpurrCarsGamemode::SpawnPlayers()
{
    if (UWorld const* const World{ GetWorld() })
    {
        check(World->IsInitialized())
        ensureMsgf(SpawnPoints.Num() >= NumPlayers, TEXT("Not enough spawn points created for num players"));

        for (int32 PlayerIndex{ 0 }; PlayerIndex < NumPlayers; ++PlayerIndex)
        {
            APlayerController* PlayerController{ UGameplayStatics::GetPlayerControllerFromID(World, PlayerIndex) };
            if (not IsValid(PlayerController))
            {
                UE_LOG(LogBumpurrCarsGamemode, Warning, TEXT("Created controller: %d"), PlayerIndex);
                PlayerController = UGameplayStatics::CreatePlayer(World, PlayerIndex, true);
            }
            else
            {
                UE_LOG(LogBumpurrCarsGamemode, Warning, TEXT("Using previously created controller: %d"), PlayerIndex);
            }

            if (!IsValid(PlayerController))
            {
                UE_LOG(LogBumpurrCarsGamemode, Error, TEXT("Could not create player controller"));
                return;
            }

            checkf(PlayerController, TEXT("Invalid player controller when trying to spawn"));
            auto const VisIDToFind{ Cast<UBumpurrCarsGameInstance>(GetGameInstance())->GetVisualID(PlayerIndex) };
            ABumperCarSpawnPoint* SpawnPoint{ nullptr };
            for (auto& s : SpawnPoints)
            {
	            if (VisIDToFind == s->GetVisualID())
	            {
                    SpawnPoint = s;
	            }
            }
            ensure(SpawnPoint);

            auto Pawn{ SpawnDefaultPawnAtTransform(PlayerController, SpawnPoint->GetTransform()) };
        	ensureMsgf(Pawn, TEXT("Invalid pawn when trying to spawn"));
            if (Pawn)
            {
                PlayerController->Possess(Pawn);

                if (ABumperCarPlayerController * PC{ Cast<ABumperCarPlayerController>(PlayerController) })
                {
                    ABumperCarPawn* BumperCar = Cast<ABumperCarPawn>(Pawn);
                    if (BumperCar)
                    {
                        int32 VisualID = SpawnPoint->GetVisualID();
                        BumperCar->SetUserIndex(VisualID);

                        if (PlayerImages.IsValidIndex(VisualID))
                        {
                            UTexture2D* PlayerImage = PlayerImages[VisualID];
                            BumperCar->SetPlayerImage(PlayerImage);

                            FString ImageName = PlayerImage ? PlayerImage->GetName() : TEXT("None");
                            UE_LOG(LogBumpurrCarsGamemode, Log, TEXT("Assigned image '%s' to Player ID %d"), *ImageName, VisualID);
                        }
                    }
                }
            }   
            else
            {
                UE_LOG(LogBumpurrCarsGamemode, Error, TEXT("Could not spawn pawn"));
            }
        }
    }
}

void ABumpurrCarsGamemode::SetupCamera()
{
	checkf(BP_CameraActorClass, TEXT("BP_CameraActorClass must be set in the GameMode"));
    if (!BP_CameraActorClass)
    {
        return;
    }

    TArray<AActor*> FoundCameras{};
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), BP_CameraActorClass, FoundCameras);

	checkf(FoundCameras.Num() > 0, TEXT("Need at least one gameplay camera in the level"));
    UE_LOG(LogBumpurrCarsGamemode, Log, TEXT("Found %d cameras"), FoundCameras.Num())
        


	ACameraActor* GameplayCamera{ Cast<ACameraActor>(FoundCameras[0]) };
	checkf(GameplayCamera, TEXT("Spawned GameplayCamera is null"));

    UE_LOG(LogBumpurrCarsGamemode, Log, TEXT("Using %s as camera"), *GameplayCamera->GetName())
	UE_LOG(LogBumpurrCarsGamemode, Log, TEXT("Setting the camera up for all player controllers"));



    // Iterate through all player controllers and set the view target to the level camera
    for (FConstPlayerControllerIterator It{ GetWorld()->GetPlayerControllerIterator() }; It; ++It)
    {
        if (APlayerController * PC{ It->Get() })
        {
            PC->SetViewTarget(Cast<AActor>(GameplayCamera));
        }
    }
}

void ABumpurrCarsGamemode::UpdateGameTimer()
{
    --RemainingTime;

    UE_LOG(LogBumpurrCarsGamemode, Log, TEXT("Time Remaining: %d"), RemainingTime);

    OnTimerUpdate.Broadcast(RemainingTime);

    if (RemainingTime <= 0)
    {
        GetWorldTimerManager().ClearTimer(GameTimerHandle);
        EndGame();
    }
}

void ABumpurrCarsGamemode::EndGame()
{
    // TODO broadcast game end event
    // TODO Trigger the game ended UI 

    

    if (UAudioManager * AudioManager{ GetGameInstance()->GetSubsystem<UAudioManager>() })
    {
        AudioManager->StopLevelMusic();
    }
    else
    {
        UE_LOG(LogAudio, Error, TEXT("AudioManager not found!"));
    }


    UE_LOG(LogBumpurrCarsGamemode, Warning, TEXT("Game ended"));

    ////--debug score-- 
    //    if (UScoreManagerWorldSubsystem* ScoreManager = GetWorld()->GetSubsystem<UScoreManagerWorldSubsystem>())
    //{
    //    TArray<ABumperCarPawn*> SortedPlayers = ScoreManager->GetPlayersSortedByScore();
    //
    //    UE_LOG(LogTemp, Warning, TEXT("=== Sorted Players by Score ==="));
    //    for (int32 i = 0; i < SortedPlayers.Num(); ++i)
    //    {
    //        ABumperCarPawn* Player = SortedPlayers[i];
    //        if (Player && Player->GetScoreComponent())
    //        {
    //            int32 Score = Player->GetScoreComponent()->GetScore();
    //            int32 Index = Player->GetUserIndex();
    //            UE_LOG(LogTemp, Warning, TEXT("Rank %d: PlayerIndex %d with Score %d"), i + 1, Index, Score);
    //        }
    //    }
    //}
    //else
    //{
    //    UE_LOG(LogTemp, Error, TEXT("ScoreManagerWorldSubsystem not found!"));
    //}
    ////--debug score-- 


    if(auto controler = Cast<ABumperCarPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        controler->SetInputModeToUI();

        auto pauseMenuInstance = CreateWidget<UUserWidget>(controler, EndUI);
        pauseMenuInstance->AddToViewport();
    }
    for (TActorIterator<ABumperCarPawn> It(GetWorld()); It; ++It)
    {
        ABumperCarPawn* Car = *It;
        if (Car)
        {
            Car->ResetAcceleration();

            if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Car->GetRootComponent()))
            {
                Root->SetSimulatePhysics(false);
            }
        }
    }
    
    //if (UGameInstance * GI{ GetGameInstance() })
    //{
    //    if (UBumpurrCarsGameInstance * BumpGI{ Cast<UBumpurrCarsGameInstance>(GI) })
    //    {
    //        CollectGarbage(RF_NoFlags);
    //        UE_LOG(LogBumpurrCarsGamemode, Warning, TEXT("Force cleaning garbage! "));
    //        BumpGI->ReloadLevel();
    //    }
    //}
}

void ABumpurrCarsGamemode::OnCountDownTimerHandle()
{
    RemainingCountdownTime -= 1;

	UE_LOG(LogBumpurrCarsGamemode, Warning, TEXT("Remaining time: %d"), RemainingCountdownTime);
    if (-1 == RemainingCountdownTime)
    {
        UE_LOG(LogBumpurrCarsGamemode, Warning, TEXT("Start game"));

        GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
        CountdownWidget->RemoveFromParent();
        CountdownWidget = nullptr;
        CountdownTextures.Empty();
        CountDownSound = nullptr;

        StartGame();

        return;
    }

    UImage* CountdownImage{ Cast<UImage>(CountdownWidget->GetWidgetFromName(TEXT("CountdownImage"))) };
    if (CountdownImage)
    {
        CountdownImage->SetBrushFromTexture(CountdownTextures[CountdownTextures.Num() - 1 - RemainingCountdownTime], true);
    }

}
