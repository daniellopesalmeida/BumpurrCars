// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BumperCars_StartScreen.h"
#include "Core/BumperCarPlayerController.h"

#include "Blueprint/UserWidget.h"
#include <Kismet/GameplayStatics.h>

void ABumperCars_StartScreen::StartGame()
{
    //UE_LOG(LogTemp, Warning, TEXT("Getting triggerd"));

    check(WBP_StartScreen);

    //if (WBP_StartScreen)
    {
        StartScreenWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_StartScreen);

        if (StartScreenWidget)
        {
            StartScreenWidget->AddToViewport();
        }
    }

    //conecting controler
    if(UWorld const* const World{ GetWorld() })
    {
        APlayerController* PlayerController{ UGameplayStatics::GetPlayerControllerFromID(World, 0) };
        if (!IsValid(PlayerController))
        {
            UE_LOG(LogTemp, Warning, TEXT("Created controller: %d"), 0);
            PlayerController = UGameplayStatics::CreatePlayer(World, 0, true);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not make new controller"));
        }

        checkf(PlayerController, TEXT("Invalid player controller when trying to spawn"));

        auto Pawn{ SpawnDefaultPawnAtTransform(PlayerController, FTransform({1,0,0},{0,1,0},{0,0,1},{})) };
        ensureMsgf(Pawn, TEXT("Invalid pawn when trying to spawn"));
        if (Pawn)
        {
            PlayerController->Possess(Pawn);

            if (ABumperCarPlayerController * PC{ Cast<ABumperCarPlayerController>(PlayerController) })
            {
                //Cast<ABumperCarPawn>(Pawn)->SetUserIndex(0);
                PC->SetupInput();
                //PC->SetInputModeToUI();
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not spawn pawn"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("could not get world"));
    }
}

void ABumperCars_StartScreen::BeginPlay()
{
    Super::BeginPlay();
}

void ABumperCars_StartScreen::StartPlay()
{
    Super::StartPlay();

    StartGame();

    if (auto controler = Cast<ABumperCarPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        controler->SetInputModeToUI();
    }
}
