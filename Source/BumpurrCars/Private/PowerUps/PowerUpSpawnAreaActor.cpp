// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerUps/PowerUpSpawnAreaActor.h"
#include "Components/BoxComponent.h"

APowerUpSpawnAreaActor::APowerUpSpawnAreaActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = SpawnBox;
}

FVector APowerUpSpawnAreaActor::GetRandomPointInArea() const
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

void APowerUpSpawnAreaActor::BeginPlay()
{
	Super::BeginPlay();
}

void APowerUpSpawnAreaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
