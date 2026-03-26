// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerUps/ScoreMultPowerUp.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AScoreMultPowerUp::AScoreMultPowerUp()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardTriggerComponent->TargetComponent = PowerUpMesh;
}

void AScoreMultPowerUp::BeginPlay()
{
	Super::BeginPlay();
}

void AScoreMultPowerUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AScoreMultPowerUp::OnTrigger(FHazardTriggerEventData const& Data)
{
	Super::OnTrigger(Data);

	Data.InstigatorActor->ScoreMultPowerUp();

	Destroy();
}
