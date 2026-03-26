// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseHazard.generated.h"

UENUM(BlueprintType)
enum class EHazardType : uint8
{
	Ground			UMETA(DisplayName = "Ground"),
	Wall			UMETA(DisplayName = "Wall"),
	GroundAndWall   UMETA(DisplayName = "GroundAndWall"),
	Other			UMETA(DisplayName = "Other") // E.g meteor falling from sky
};

UCLASS()
class ABaseHazard : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseHazard();

	UFUNCTION(BlueprintCallable, Category = "Hazard")
	EHazardType GetHazardType() const { return HazardType; }
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hazard")
	EHazardType HazardType { EHazardType::Ground };

private:
};
