// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUpSpawnAreaActor.generated.h"

UCLASS()
class BUMPURRCARS_API APowerUpSpawnAreaActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APowerUpSpawnAreaActor();

	[[nodiscard]] FVector GetRandomPointInArea() const;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeteorSpawn")
	class UBoxComponent* SpawnBox;

private:
};
