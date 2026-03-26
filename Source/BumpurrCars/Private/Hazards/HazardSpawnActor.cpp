// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/HazardSpawnActor.h"

#include "Components/BoxComponent.h"

AHazardSpawnActor::AHazardSpawnActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = SpawnBox;
}

FVector AHazardSpawnActor::GetRandomPointInVolume() const
{
	if (!SpawnBox)
	{
		return FVector::ZeroVector;
	}

	FVector const BoxCenter{ SpawnBox->GetComponentLocation() };
	FVector const BoxExtent{ SpawnBox->GetScaledBoxExtent() };

	return FVector{
		BoxCenter.X + FMath::RandRange(-BoxExtent.X, BoxExtent.X),
		BoxCenter.Y + FMath::RandRange(-BoxExtent.Y, BoxExtent.Y),
		BoxCenter.Z
	};
}

void AHazardSpawnActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHazardSpawnActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

