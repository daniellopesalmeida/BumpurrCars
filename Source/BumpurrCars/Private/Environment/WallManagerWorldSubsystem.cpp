// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/WallManagerWorldSubsystem.h"
#include "Environment/WallPieceActor.h"

#include <Kismet/GameplayStatics.h>

DEFINE_LOG_CATEGORY(LogWallManager);

void UWallManagerWorldSubsystem::StartRandomWallOpener()
{
    CurrentlyClosedWalls.Empty();
    CurrentlyOpenWalls.Empty();
    bAllWallsOpenedByButton = false;

    // Fill the array with all openable walls
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWallPieceActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!IsValid(Actor) || !Actor->ActorHasTag("OpenableWall")) continue;

        if (AWallPieceActor* WallPiece = Cast<AWallPieceActor>(Actor))
        {
            CurrentlyClosedWalls.Add(WallPiece);
        }
    }

    UE_LOG(LogWallManager, Warning, TEXT("Found %d openable walls"), CurrentlyClosedWalls.Num());

    if (!GetWorld()->GetTimerManager().IsTimerActive(WallOpenTimerHandle))
    {
        float RandomInterval = FMath::RandRange(MinTimeNextWallOpen, MaxTimeNextWallOpen);

        GetWorld()->GetTimerManager().SetTimer(
            WallOpenTimerHandle,
            this,
            &UWallManagerWorldSubsystem::OpenRandomWall,
            RandomInterval,
            false
        );
    }
}

void UWallManagerWorldSubsystem::OpenWallFromButton()
{
    if (CurrentlyOpenWalls.Num() > 0 && CurrentlyClosedWalls.Num() == 0)
    {
        UE_LOG(LogWallManager, Log, TEXT("All walls already open. Doing nothing."));
        return;
    }

    UE_LOG(LogWallManager, Log, TEXT("Button pressed: opening all walls!"));

    GetWorld()->GetTimerManager().ClearTimer(WallOpenTimerHandle);

    // Close all currently open walls and clear their timers
    TArray<FOpenWall> OpenWallsCopy = CurrentlyOpenWalls;
    for (FOpenWall& OpenedWall : OpenWallsCopy)
    {
        if (!IsValid(OpenedWall.Wall)) continue;

        GetWorld()->GetTimerManager().ClearTimer(OpenedWall.TimerHandle);

        if (OpenedWall.Wall->IsWallOpen())
        {
            OpenedWall.Wall->CloseWall();
            CurrentlyClosedWalls.Add(OpenedWall.Wall);
        }
    }

    CurrentlyOpenWalls.Empty();

    // Open all previously closed walls
    TArray<AWallPieceActor*> ClosedWallsCopy = CurrentlyClosedWalls;
    CurrentlyClosedWalls.Empty();

    for (AWallPieceActor* Wall : ClosedWallsCopy)
    {
        if (!IsValid(Wall)) continue;

        Wall->OpenWall();

        FOpenWall NewOpenedWall;
        NewOpenedWall.Wall = Wall;

        GetWorld()->GetTimerManager().SetTimer(
            NewOpenedWall.TimerHandle,
            [this, Wall]()
            {
                CloseWall(Wall);
            },
            FMath::RandRange(MinTimeOpenBeforeClose, MaxTimeOpenBeforeClose),
            false
        );

        CurrentlyOpenWalls.Add(NewOpenedWall);
    }

    bAllWallsOpenedByButton = true;
}

bool UWallManagerWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only trigger initialization code once (the blueprint version)
    return Super::ShouldCreateSubsystem(Outer) && GetClass()->IsInBlueprint();
}

void UWallManagerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection); 
}

void UWallManagerWorldSubsystem::Deinitialize()
{
	GetWorld()->GetTimerManager().ClearTimer(WallOpenTimerHandle);

    for (FOpenWall& OpenedWall : CurrentlyOpenWalls)
    {
        GetWorld()->GetTimerManager().ClearTimer(OpenedWall.TimerHandle);
    }
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

    Super::Deinitialize();
}

void UWallManagerWorldSubsystem::OpenRandomWall()
{
    if (bAllWallsOpenedByButton)
    {
        UE_LOG(LogWallManager, Log, TEXT("Random opening skipped: walls were opened by button."));
        return;
    }

    if (CurrentlyOpenWalls.Num() >= MaxOpenWallsAtOnce)
    {
        UE_LOG(LogWallManager, Log, TEXT("Max open walls reached. Waiting..."));

        GetWorld()->GetTimerManager().SetTimer(
            WallOpenTimerHandle,
            this,
            &UWallManagerWorldSubsystem::OpenRandomWall,
            MinTimeNextWallOpen,
            false
        );

        return;
    }

    if (CurrentlyClosedWalls.Num() > 0)
    {
        AWallPieceActor* RandomWall = CurrentlyClosedWalls[FMath::RandRange(0, CurrentlyClosedWalls.Num() - 1)];

        RandomWall->OpenWall();
        CurrentlyClosedWalls.Remove(RandomWall);

        FOpenWall OpenedWall;
        OpenedWall.Wall = RandomWall;

        GetWorld()->GetTimerManager().SetTimer(
            OpenedWall.TimerHandle,
            [this, RandomWall]()
            {
                CloseWall(RandomWall);
            },
            FMath::RandRange(MinTimeOpenBeforeClose, MaxTimeOpenBeforeClose),
            false
        );

        CurrentlyOpenWalls.Add(OpenedWall);

        float RandomInterval = FMath::RandRange(MinTimeNextWallOpen, MaxTimeNextWallOpen);

        GetWorld()->GetTimerManager().SetTimer(
            WallOpenTimerHandle,
            this,
            &UWallManagerWorldSubsystem::OpenRandomWall,
            RandomInterval,
            false
        );
    }
    else
    {
        UE_LOG(LogWallManager, Log, TEXT("No closed walls to open."));

        GetWorld()->GetTimerManager().SetTimer(
            WallOpenTimerHandle,
            this,
            &UWallManagerWorldSubsystem::OpenRandomWall,
            MinTimeNextWallOpen,
            false
        );
    }
}

void UWallManagerWorldSubsystem::CloseWall(AWallPieceActor* Wall)
{
    if (!Wall || !Wall->IsWallOpen()) return;

    Wall->CloseWall();

    UE_LOG(LogWallManager, Log, TEXT("Wall %s closed after being open for timer."), *Wall->GetName());

    CurrentlyOpenWalls.RemoveAll([Wall](const FOpenWall& OpenWall) { return OpenWall.Wall == Wall; });
    CurrentlyClosedWalls.Add(Wall);

    if (bAllWallsOpenedByButton && CurrentlyOpenWalls.Num() == 0)
    {
        bAllWallsOpenedByButton = false;

        float RandomInterval = FMath::RandRange(MinTimeNextWallOpen, MaxTimeNextWallOpen);

        GetWorld()->GetTimerManager().SetTimer(
            WallOpenTimerHandle,
            this,
            &UWallManagerWorldSubsystem::OpenRandomWall,
            RandomInterval,
            false
        );
    }
}
