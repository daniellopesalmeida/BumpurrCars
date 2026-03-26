// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PowerUpBase.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "QuestionMarkPowerUpActor.generated.h"

UCLASS()
class BUMPURRCARS_API AQuestionMarkPowerUpActor : public APowerUpBase
{
	GENERATED_BODY()
	
public:	
	AQuestionMarkPowerUpActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	virtual void OnTrigger(FHazardTriggerEventData const& Data) override;
};
