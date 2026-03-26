// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerUps/BumpStrengthActor.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

ABumpStrengthActor::ABumpStrengthActor()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardTriggerComponent->TargetComponent = PowerUpMesh;
}

void ABumpStrengthActor::BeginPlay()
{
	Super::BeginPlay();
	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &ABumpStrengthActor::OnBumpStrenghtHit);
}

void ABumpStrengthActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABumpStrengthActor::OnBumpStrenghtHit(FHazardTriggerEventData const& Data)
{
	Data.InstigatorActor->BumperStrengthPowerUp();

	if (CollectEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollectEffect,
			Data.HitResult.ImpactPoint
		);
	}

	if (CollectSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CollectSound, Data.HitResult.ImpactPoint);
	}

	Destroy();
}

