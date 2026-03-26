// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerUps/ShieldPowerUpActor.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AShieldPowerUpActor::AShieldPowerUpActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShieldPowerUpActor::BeginPlay()
{
	Super::BeginPlay();
}

void AShieldPowerUpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShieldPowerUpActor::OnTrigger(FHazardTriggerEventData const& Data)
{
	Super::OnTrigger(Data);

	Data.InstigatorActor->ShieldPowerUp();

	Destroy();
}

