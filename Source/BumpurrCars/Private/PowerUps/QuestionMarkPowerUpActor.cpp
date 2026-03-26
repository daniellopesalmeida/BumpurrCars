// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerUps/QuestionMarkPowerUpActor.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"


AQuestionMarkPowerUpActor::AQuestionMarkPowerUpActor()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardTriggerComponent->TargetComponent = PowerUpMesh;
}

void AQuestionMarkPowerUpActor::BeginPlay()
{
	Super::BeginPlay();
}

void AQuestionMarkPowerUpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AQuestionMarkPowerUpActor::OnTrigger(FHazardTriggerEventData const& Data)
{
	Super::OnTrigger(Data);

	Data.InstigatorActor->QuestionPowerUp();
	Destroy();
}

