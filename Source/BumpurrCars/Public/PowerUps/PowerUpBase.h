// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "PowerUpBase.generated.h"

UCLASS()
class BUMPURRCARS_API APowerUpBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APowerUpBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnTrigger(FHazardTriggerEventData const& Data);

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* CollectEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* CollectSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* PowerUpMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHazardTriggerComponent* HazardTriggerComponent;

	UPROPERTY(EditDefaultsOnly)
	float RotationSpeed = 90.f; // Degrees per second

	UPROPERTY(EditDefaultsOnly)
	float TimeAliveBeforeDeSpawn{ 8.f };

	UPROPERTY(EditDefaultsOnly)
	float StartFlickeringTime{ 6.5f };

	UPROPERTY(EditDefaultsOnly)
	float FlickerOffTime{ .2f };

	UPROPERTY(EditDefaultsOnly)
	float FlickerOnTime{ .3f };

	UPROPERTY(EditDefaultsOnly)
	float FlickerOnReductionModifier{ .85f };
private:
	FTimerHandle DeSpawnHandle;
	FTimerHandle FlickerHandle;

	UFUNCTION()
	void DeSpawnPowerUp();
	UFUNCTION()
	void FlickerOn();
	UFUNCTION()
	void FlickerOff();
};
