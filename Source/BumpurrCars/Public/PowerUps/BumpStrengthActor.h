// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PowerUpBase.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"

#include "BumpStrengthActor.generated.h"

UCLASS()
class BUMPURRCARS_API ABumpStrengthActor : public APowerUpBase
{
	GENERATED_BODY()
	
public:	
	ABumpStrengthActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnBumpStrenghtHit(FHazardTriggerEventData const& Data);
};
