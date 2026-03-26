// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerUps/PowerUpBase.h"

#include "NiagaraFunctionLibrary.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Kismet/GameplayStatics.h"

APowerUpBase::APowerUpBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PowerUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerUpStaticMesh"));
	RootComponent = PowerUpMesh;

	PowerUpMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	PowerUpMesh->SetNotifyRigidBodyCollision(true);
	PowerUpMesh->SetCollisionObjectType(ECC_WorldStatic);
	PowerUpMesh->SetCollisionResponseToAllChannels(ECR_Block);

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComp"));
}

void APowerUpBase::BeginPlay()
{
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::Hit;
	HazardTriggerComponent->TargetComponent = PowerUpMesh;

	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &APowerUpBase::OnTrigger);
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(DeSpawnHandle, this, &APowerUpBase::DeSpawnPowerUp, TimeAliveBeforeDeSpawn);
	GetWorld()->GetTimerManager().SetTimer(FlickerHandle, this, &APowerUpBase::FlickerOff, StartFlickeringTime);
}

void APowerUpBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void APowerUpBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
}

void APowerUpBase::OnTrigger(FHazardTriggerEventData const& Data)
{
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
}

void APowerUpBase::DeSpawnPowerUp()
{
	Destroy();
}

void APowerUpBase::FlickerOn()
{
	PowerUpMesh->SetVisibility(true);
	GetWorld()->GetTimerManager().SetTimer(FlickerHandle, this, &APowerUpBase::FlickerOff, FlickerOnTime);
	FlickerOnTime *= FlickerOnReductionModifier;
}

void APowerUpBase::FlickerOff()
{
	PowerUpMesh->SetVisibility(false);
	GetWorld()->GetTimerManager().SetTimer(FlickerHandle, this, &APowerUpBase::FlickerOn, FlickerOffTime);
}

