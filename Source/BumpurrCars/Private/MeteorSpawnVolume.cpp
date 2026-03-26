// Fill out your copyright notice in the Description page of Project Settings.


#include "MeteorSpawnVolume.h"
#include "Components/BoxComponent.h"

AMeteorSpawnVolume::AMeteorSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	HazardSpawnType = EHazardSpawnPointType::Other;
}

void AMeteorSpawnVolume::BeginPlay()
{
	Super::BeginPlay();

}

void AMeteorSpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


