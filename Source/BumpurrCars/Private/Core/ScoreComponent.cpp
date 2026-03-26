// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ScoreComponent.h"

#include "Characters/BumperCarPawn.h"
#include "Core/ScoreManagerWorldSubsystem.h"

UScoreComponent::UScoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScoreComponent::ReducePlayersHit()
{
	--PlayersHit;
	
	if (0 == PlayersHit)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(AddScoreHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(AddScoreHandle);
		}
		GetWorld()->GetTimerManager().SetTimer(AddScoreHandle, this, &UScoreComponent::AddScore, ComboTime);
	}
}

void UScoreComponent::IncreasePlayersHit()
{
	++PlayersHit;
	GetWorld()->GetTimerManager().ClearTimer(AddScoreHandle);
}

void UScoreComponent::OnSpikesReduceScore()
{
	RemoveScore(LossOnSpikesHit);
}

void UScoreComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind to all the score related events
	if (ABumperCarPawn* CarPawn{ Cast<ABumperCarPawn>(GetOwner()) })
	{
		CarPawn->OnPlayerHeadOnDraw.AddDynamic(this, &UScoreComponent::OnPlayerHeadOnDraw);
		CarPawn->OnPlayerWonCollision.AddDynamic(this, &UScoreComponent::OnPlayerWonCollision);
		CarPawn->OnPlayerOffMap.AddDynamic(this, &UScoreComponent::OnPlayerOffMap);
		CarPawn->OnControlRestore.AddDynamic(this, &UScoreComponent::OnControlRestore);
		CarPawn->OnPlayerHitByMeteor.AddDynamic(this, &UScoreComponent::OnMeteorHit);
	}
	else
	{
		UE_LOG(LogScoreManager, Error, TEXT("Invalid pawn in BeginPlay of ScoreComponent"));
	}
}

void UScoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UScoreComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (ABumperCarPawn* CarPawn{ Cast<ABumperCarPawn>(GetOwner()) })
	{
		CarPawn->OnPlayerHeadOnDraw.RemoveDynamic(this, &UScoreComponent::UScoreComponent::OnPlayerHeadOnDraw);
		CarPawn->OnPlayerWonCollision.RemoveDynamic(this, &UScoreComponent::OnPlayerWonCollision);
		CarPawn->OnPlayerOffMap.RemoveDynamic(this, &UScoreComponent::OnPlayerOffMap);
		CarPawn->OnControlRestore.RemoveDynamic(this, &UScoreComponent::OnControlRestore);
		CarPawn->OnPlayerHitByMeteor.RemoveDynamic(this, &UScoreComponent::OnMeteorHit);
	}
	else
	{
		UE_LOG(LogScoreManager, Error, TEXT("Invalid pawn in EndPlay of ScoreComponent"));
	}

	Super::EndPlay(EndPlayReason);
}

void UScoreComponent::OnPlayerWonCollision(EImpactLevel ImpactLevel, ABumperCarPawn* Winner, ABumperCarPawn* Loser, FVector const& ImpactPoint)
{
	if (!Winner || Winner != Cast<ABumperCarPawn>( GetOwner()))
	{
		UE_LOG(LogScoreManager, Error, TEXT("Invalid winning car in ScoreComponent - OnPlayerWonCollision"))
	}

	// temp logging until the score display is hooked up
	//UE_LOG(LogScoreManager, Warning, TEXT("OnPlayerWonCollision triggered! Impact Level: %d"), static_cast<int32>(ImpactLevel));
	//UE_LOG(LogScoreManager, Warning, TEXT("Winning Car: %s"), *Winner->GetName());
	//UE_LOG(LogScoreManager, Warning, TEXT("Losing Car: %s"), *Loser->GetName());

	// The other car was pushed against us by someone else -> they are the winner
	if (LastHitByPlayer)
	{

		if (Loser->GetScoreComponent()->OverwriteLastHitPlayer(LastHitByPlayer))
		{
			//UE_LOG(LogScoreManager, Warning, TEXT("Last hit was by: %s"), *LastHitByPlayer->GetName());
			LastHitByPlayer->GetScoreComponent()->AddBaseScore(ImpactLevel);
		}
	}
	else
	{
		if (Loser->GetScoreComponent()->OverwriteLastHitPlayer(Winner))
		{
			//UE_LOG(LogScoreManager, Warning, TEXT("No last hit player, assigning score to: %s"), *Winner->GetName());
			AddBaseScore(ImpactLevel);
		}
	}
}

void UScoreComponent::OnPlayerHeadOnDraw(EImpactLevel ImpactLevel, ABumperCarPawn* Other, FVector const& ImpactLocation)
{
	//UE_LOG(LogScoreManager, Log, TEXT("ScoreComponent: Head-On Draw! Impact Level: %d"), static_cast<int32>(ImpactLevel));
	AddBaseScore(ImpactLevel);
}

void UScoreComponent::OnPlayerOffMap()
{
	RemoveScore(PercentageLossOnOffMap, true);

	if (LastHitByPlayer)
	{
		LastHitByPlayer->GetScoreComponent()->AddMultiplier(MultiplierGainOnOffMap);
	}
}

void UScoreComponent::AddBaseScore(EImpactLevel ImpactLevel)
{
	ABumperCarPawn* OwnerCar = Cast<ABumperCarPawn>(GetOwner());
	// Get the current score multiplier from the pawn
	float PowerUpMultiplier = OwnerCar ? OwnerCar->GetScoreMultiplier() : 1.f;

	UE_LOG(LogTemp, Warning, TEXT("Score Multiplier for %s: %f"), *GetOwner()->GetName(), PowerUpMultiplier);

	switch (ImpactLevel)
	{
	case EImpactLevel::Light:
	{
		CurrentGainedScoreBase += GainOnLight* PowerUpMultiplier;
		break;
	}
	case EImpactLevel::Medium:
	{
		CurrentGainedScoreBase += GainOnMedium * PowerUpMultiplier;
		break;
	}
	case EImpactLevel::Heavy:
	{
		CurrentGainedScoreBase += GainOnHeavy * PowerUpMultiplier;
		break;
	}
	case EImpactLevel::VeryHeavy:	
	{
		CurrentGainedScoreBase += GainOnVeryHeavy * PowerUpMultiplier;
		break;
	}
	default:
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s: CurrentGainedScoreBase = %d"), *GetOwner()->GetName(), CurrentGainedScoreBase);
	Cast<ABumperCarPawn>(GetOwner())->GetScorePopup()->SetScoreGain(CurrentGainedScoreBase * CurrentGainedScoreMultiplier);
}

void UScoreComponent::AddMultiplier(int32 multiplier)
{
	ABumperCarPawn* OwnerCar{ Cast<ABumperCarPawn>(GetOwner()) };
	float const PowerUpMultiplier = OwnerCar ? OwnerCar->GetScoreMultiplier() : 1.f;
	multiplier *= PowerUpMultiplier;

	CurrentGainedScoreMultiplier *= multiplier;
	Cast<ABumperCarPawn>(GetOwner())->GetScorePopup()->SetScoreGain(CurrentGainedScoreBase * CurrentGainedScoreMultiplier);
}


void UScoreComponent::AddScore()
{
	if (CurrentGainedScoreBase == 0)
	{
		return;
	}

	PlayerScore += CurrentGainedScoreBase * CurrentGainedScoreMultiplier;

	CurrentGainedScoreBase = 0;
	CurrentGainedScoreMultiplier = 1;

	// Temp log until UI is hooked up with the on score changed
	//UE_LOG(LogScoreManager, Warning, TEXT("Added score, score now is: %d for player: %d"), PlayerScore, Cast<ABumperCarPawn>(GetOwner())->GetUserIndex());

	Cast<ABumperCarPawn>(GetOwner())->GetScorePopup()->SetScoreGain(0);
	OnScoreChanged.Broadcast(Cast<ABumperCarPawn>(GetOwner()), PlayerScore, false);
}

void UScoreComponent::RemoveScore(uint32 Amount, bool bIsPercentage)
{
	uint32 const PrevScore{ static_cast<uint32>(PlayerScore) };

	if (bIsPercentage)
	{
		PlayerScore = FMath::Max(0, PlayerScore - (PlayerScore * (Amount / 100.f)));
	}
	else
	{
		PlayerScore = FMath::Max(0, PlayerScore - static_cast<int32>(Amount));
	}

	//UE_LOG(LogScoreManager, Warning, TEXT("Removed score, score now is: %d for player: %d"), PlayerScore, Cast<ABumperCarPawn>(GetOwner())->GetUserIndex());
	auto const Loss{ PrevScore - PlayerScore };
	if (0 != Loss)
	{
		Cast<ABumperCarPawn>(GetOwner())->GetScorePopup()->SetScoreLoss(Loss);
	}

	//Cast<ABumperCarPawn>(GetOwner())->GetScorePopup()->SetScoreLoss(0);
	OnScoreChanged.Broadcast(Cast<ABumperCarPawn>(GetOwner()), PlayerScore, true);
}

void UScoreComponent::OnControlRestore()
{
	ResetLastHitPlayer();

	if (PlayersHit == 0)
	{
		AddScore();
	}
}

void UScoreComponent::OnMeteorHit()
{
	if (ABumperCarPawn * CarPawn{ Cast<ABumperCarPawn>(GetOwner()) })
	{
		if (CarPawn->HasShield())
			return;
	}

	RemoveScore(LossOnMeteorHit);

	if (LastHitByPlayer)
	{
		LastHitByPlayer->GetScoreComponent()->AddMultiplier(MultiplierOnMeteorHit);
	}
}

void UScoreComponent::ResetLastHitPlayer()
{
	if (LastHitByPlayer)
	{
		LastHitByPlayer->GetScoreComponent()->ReducePlayersHit();
	}

	LastHitByPlayer = nullptr;
}

bool UScoreComponent::OverwriteLastHitPlayer(ABumperCarPawn* NewLastHit)
{
	check(NewLastHit);

	ABumperCarPawn* OwningPawn{ Cast<ABumperCarPawn>(GetOwner()) };
	if (not OwningPawn)
	{
		return false;
	}

	if (NewLastHit == OwningPawn)
	{
		UE_LOG(LogBumperCar, Error, TEXT("Trying to collide with self!"));
		return false;
	}

	if (LastHitByPlayer == NewLastHit)
	{
		return true;
	}

	if (LastHitByPlayer)
	{
		LastHitByPlayer->GetScoreComponent()->ReducePlayersHit();
	}

	LastHitByPlayer = NewLastHit;
	LastHitByPlayer->GetScoreComponent()->IncreasePlayersHit();

	return true;
}

