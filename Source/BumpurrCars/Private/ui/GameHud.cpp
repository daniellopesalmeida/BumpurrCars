// Fill out your copyright notice in the Description page of Project Settings.


#include "ui/GameHud.h"
#include <Kismet/GameplayStatics.h>
#include "Core/ScoreManagerWorldSubsystem.h"
#include "Core/ScoreComponent.h"
#include "Characters/BumperCarPawn.h"

void UGameHud::NativeConstruct()
{
	Super::NativeConstruct();

	if(UWorld * World = GetWorld())
	{
		if (UScoreManagerWorldSubsystem* Manager = World->GetSubsystem<UScoreManagerWorldSubsystem>())
		{
			InitializeFromManager(Manager);
		}
	}
}

void UGameHud::NativeDestruct()
{
	Super::NativeDestruct();

	if (UWorld* World = GetWorld())
	{
		if (UScoreManagerWorldSubsystem* Manager = World->GetSubsystem<UScoreManagerWorldSubsystem>())
		{
			for (ABumperCarPawn* Car : Manager->GetAllPlayers())
			{
				if (Car && Car->GetScoreComponent())
				{
					Car->GetScoreComponent()->OnScoreChanged.RemoveDynamic(this, &UGameHud::UpdatePlayerScore);
				}
			}
		}
	}
}

void UGameHud::InitializeFromManager(UScoreManagerWorldSubsystem* Manager)
{
	if (!Manager)
	{
		UE_LOG(LogTemp, Warning, TEXT("no manager detected!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Initialize GameHud"));

	const TArray<ABumperCarPawn*>& Players = Manager->GetAllPlayers();

	for (int32 Index = 0; Index < Players.Num(); ++Index)
	{
		ABumperCarPawn* Car = Players[Index];
		if (!Car || !Car->GetScoreComponent()) continue;

		Car->GetScoreComponent()->OnScoreChanged.AddDynamic(this, &UGameHud::UpdatePlayerScore);
		UE_LOG(LogTemp, Warning, TEXT("Delegate bound for Player %d"), Car->GetUserIndex());

		UTextBlock* ScoreText = nullptr;

		switch (Index)
		{
		case 0: ScoreText = Player1ScoreText; break;
		case 1: ScoreText = Player2ScoreText; break;
		case 2: ScoreText = Player3ScoreText; break;
		case 3: ScoreText = Player4ScoreText; break;
		}

		if (ScoreText)
		{
			ScoreText->SetText(FText::AsNumber(Car->GetScoreComponent()->GetScore()));
			PlayerScoreMap.Add(Car, ScoreText);
		}
	}

	if (P1ScoreMultImage) P1ScoreMultImage->SetVisibility(ESlateVisibility::Hidden);
	if (P1BumperStrengthImage) P1BumperStrengthImage->SetVisibility(ESlateVisibility::Hidden);
	if (P1ShieldImage) P1ShieldImage->SetVisibility(ESlateVisibility::Hidden);

	if (P2ScoreMultImage) P2ScoreMultImage->SetVisibility(ESlateVisibility::Hidden);
	if (P2BumperStrengthImage) P2BumperStrengthImage->SetVisibility(ESlateVisibility::Hidden);
	if (P2ShieldImage) P2ShieldImage->SetVisibility(ESlateVisibility::Hidden);

	if (P3ScoreMultImage) P3ScoreMultImage->SetVisibility(ESlateVisibility::Hidden);
	if (P3BumperStrengthImage) P3BumperStrengthImage->SetVisibility(ESlateVisibility::Hidden);
	if (P3ShieldImage) P3ShieldImage->SetVisibility(ESlateVisibility::Hidden);

	if (P4ScoreMultImage) P4ScoreMultImage->SetVisibility(ESlateVisibility::Hidden);
	if (P4BumperStrengthImage) P4BumperStrengthImage->SetVisibility(ESlateVisibility::Hidden);
	if (P4ShieldImage) P4ShieldImage->SetVisibility(ESlateVisibility::Hidden);

	PlayerScoreMultiplierImages.Add(0, P1ScoreMultImage);
	PlayerScoreMultiplierImages.Add(1, P2ScoreMultImage);
	PlayerScoreMultiplierImages.Add(2, P3ScoreMultImage);
	PlayerScoreMultiplierImages.Add(3, P4ScoreMultImage);

	
	PlayerBumperStrengthImages.Add(0, P1BumperStrengthImage);
	PlayerBumperStrengthImages.Add(1, P2BumperStrengthImage);
	PlayerBumperStrengthImages.Add(2, P3BumperStrengthImage);
	PlayerBumperStrengthImages.Add(3, P4BumperStrengthImage);

	PlayerShieldImages.Add(0, P1ShieldImage);
	PlayerShieldImages.Add(1, P2ShieldImage);
	PlayerShieldImages.Add(2, P3ShieldImage);
	PlayerShieldImages.Add(3, P4ShieldImage);
}

void UGameHud::UpdatePlayerScore(ABumperCarPawn* Car, int32 NewScore, bool IsLoss)
{
	UE_LOG(LogTemp, Warning, TEXT("UpdatePlayerScore called for Player %d with new score %d"), Car->GetUserIndex(), NewScore);

	if (UTextBlock** TextPtr = PlayerScoreMap.Find(Car))
	{
		if (UTextBlock* ScoreText = *TextPtr)
		{
			ScoreText->SetText(FText::AsNumber(NewScore));
		}
	}
}

UImage* UGameHud::GetPlayerScoreMultiplierImage(int32 PlayerIndex) const
{
	if (PlayerScoreMultiplierImages.Contains(PlayerIndex))
	{
		return PlayerScoreMultiplierImages[PlayerIndex];
	}
	return nullptr;
}

UImage* UGameHud::GetPlayerBumperStrengthImage(int32 PlayerIndex) const
{
	if (PlayerBumperStrengthImages.Contains(PlayerIndex))
	{
		return PlayerBumperStrengthImages[PlayerIndex];
	}
	return nullptr;
}

UImage* UGameHud::GetPlayerShieldImage(int32 PlayerIndex) const
{
	if (PlayerShieldImages.Contains(PlayerIndex))
	{
		return PlayerShieldImages[PlayerIndex];
	}
	return nullptr;
}
