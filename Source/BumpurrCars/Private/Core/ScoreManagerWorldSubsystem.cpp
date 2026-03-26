// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ScoreManagerWorldSubsystem.h"
#include "Characters/BumperCarPawn.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Core/ScoreComponent.h"

DEFINE_LOG_CATEGORY(LogScoreManager);

void UScoreManagerWorldSubsystem::StartScoreManager(int32 NumPlayers)
{
    Players.SetNum(NumPlayers);

    int32 FoundActors{};

    for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
    {
        auto const Pawn{ (*It)->GetPawn() };
        check(Pawn);

        auto const BumperCar{ Cast<ABumperCarPawn>(Pawn)};
        if (not BumperCar)
        {
            UE_LOG(LogScoreManager, Warning, TEXT("A controlled pawn is not a bumper car"))
            continue;
        }

        auto const UserIndex{ BumperCar->GetUserIndex() };

        check(UserIndex >= 0 && UserIndex <= NumPlayers);

        Players[UserIndex] = BumperCar;
        Players[UserIndex]->GetScoreComponent()->OnScoreChanged.AddDynamic(this, &UScoreManagerWorldSubsystem::OnScoreChanged);

    	++FoundActors;
    }

    check(FoundActors == NumPlayers)
}



TArray<ABumperCarPawn*> UScoreManagerWorldSubsystem::GetPlayersSortedByScore() const
{
    TArray<ABumperCarPawn*> SortedPlayers = Players;

    SortedPlayers.Sort([](const ABumperCarPawn& A, const ABumperCarPawn& B)
        {
            int32 ScoreA = A.GetScoreComponent()->GetScore();
            int32 ScoreB = B.GetScoreComponent()->GetScore();
            return ScoreA > ScoreB; 
        });

    return SortedPlayers;
}

bool UScoreManagerWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only trigger initialization code once (the blueprint version)
    return Super::ShouldCreateSubsystem(Outer) && GetClass()->IsInBlueprint();
}


void UScoreManagerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UScoreManagerWorldSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UScoreManagerWorldSubsystem::OnScoreChanged(ABumperCarPawn* Car, int32 NewScore, bool IsLoss)
{
    auto const ID{ Car->GetUserIndex() };
    ensure(Players[ID] == Car);

    if (HighScorePlayerID == -1)
    {
        int32 MaxScore{ 0 };
        int32 NewHighID{ -1 };

        for (int32 i = 0; i < Players.Num(); ++i)
        {
            if (Players[i]->GetScoreComponent()->GetScore() >= MaxScore)
            {
                MaxScore = Players[i]->GetScoreComponent()->GetScore();
                NewHighID = i;
            }
        }

        if (MaxScore == 0 || NewHighID == -1)
        {
            return;
        }

        HighScorePlayerID = NewHighID;
        Players[HighScorePlayerID]->ActivateCrown();
        UE_LOG(LogScoreManager, Warning, TEXT("New high score id: %d"), HighScorePlayerID);

        return;
    }

    int32 MaxScore{ 0 };
    int32 NewHighID{ -1 };

    for (int32 i = 0; i < Players.Num(); ++i)
    {
        if (Players[i]->GetScoreComponent()->GetScore() >= MaxScore)
        {
            MaxScore = Players[i]->GetScoreComponent()->GetScore();
            NewHighID = i;
        }
    }

    if (HighScorePlayerID == NewHighID)
    {
        return;
    }

    // Deactivate crown for ID
    Players[HighScorePlayerID]->DeActivateCrown();
    HighScorePlayerID = NewHighID;
    // Activate for new high
    Players[HighScorePlayerID]->ActivateCrown();

    UE_LOG(LogScoreManager, Warning, TEXT("New high score id: %d"), HighScorePlayerID);
}

