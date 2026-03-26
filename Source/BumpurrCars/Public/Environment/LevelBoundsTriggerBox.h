// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "LevelBoundsTriggerBox.generated.h"

/**
 * 
 */
UCLASS()
class BUMPURRCARS_API ALevelBoundsTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:
	ALevelBoundsTriggerBox();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerEnter(AActor* OverlappedActor, AActor* OtherActor);

private:
	
};
