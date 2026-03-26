// Fill out your copyright notice in the Description page of Project Settings.

#include "Environment/LevelBoundsTriggerBox.h"

#include "Characters/BumperCarPawn.h"
#include "Components/ShapeComponent.h"
#include "Core/BumpurrCarsGamemode.h"

ALevelBoundsTriggerBox::ALevelBoundsTriggerBox()
{
	if (UShapeComponent * ShapeComponent{ GetCollisionComponent() })
	{
		ShapeComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
		ShapeComponent->SetGenerateOverlapEvents(true);
		ShapeComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

		// Only interested in the players (pawns)
		ShapeComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}

void ALevelBoundsTriggerBox::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &ALevelBoundsTriggerBox::OnTriggerEnter);
}

void ALevelBoundsTriggerBox::OnTriggerEnter(AActor* OverlappedActor, AActor* OtherActor)
{
	APawn* Pawn{ Cast<APawn>(OtherActor) };

	if (Pawn && Pawn->IsPlayerControlled())
	{
		if (auto PlayerPawn{ Cast<ABumperCarPawn>(Pawn) })
		{
			PlayerPawn->OnPlayerOffMap.Broadcast();
		}
	}
}
