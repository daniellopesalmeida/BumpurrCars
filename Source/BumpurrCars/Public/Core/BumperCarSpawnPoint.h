// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BumperCarSpawnPoint.generated.h"

class UArrowComponent;

UCLASS()
class BUMPURRCARS_API ABumperCarSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ABumperCarSpawnPoint();
	virtual void Tick(float DeltaTime) override;

	[[nodiscard]] int32 GetVisualID() const { return VisualsID; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VisualsID{ 0 };

private:


};
