// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardSpawnActor.h"
#include "MeteorSpawnVolume.generated.h"

UCLASS()
class BUMPURRCARS_API AMeteorSpawnVolume : public AHazardSpawnActor
{
	GENERATED_BODY()
	
public:	
	AMeteorSpawnVolume();
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
private:
};
