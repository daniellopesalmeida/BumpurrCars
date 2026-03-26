// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HazardSpawnActor.generated.h"

UENUM(BlueprintType)
enum class EHazardSpawnPointType : uint8
{
	Ground			UMETA(DisplayName = "Ground"),
	Wall			UMETA(DisplayName = "Wall"),
	GroundAndWall   UMETA(DisplayName = "GroundAndWall"),
	Other			UMETA(DisplayName = "Other")
};


UCLASS()
class BUMPURRCARS_API AHazardSpawnActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AHazardSpawnActor();

	[[nodiscard]] FVector GetRandomPointInVolume() const;
	[[nodiscard]] EHazardSpawnPointType GetSpawnPointType() const { return HazardSpawnType; }
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHazardSpawnPointType HazardSpawnType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* SpawnBox;

private:

};
