// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Hazards/BaseHazard.h"
#include "SpikesHazardActor.generated.h"

UCLASS()
class BUMPURRCARS_API ASpikesHazardActor : public ABaseHazard
{
	GENERATED_BODY()
	
public:	
	ASpikesHazardActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* SpikeHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spikes", meta = (AllowPrivateAccess = "true"))
	float EmergeDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spikes", meta = (AllowPrivateAccess = "true"))
	float RetractDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spikes", meta = (AllowPrivateAccess = "true"))
	float StayEmergedDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spikes", meta = (AllowPrivateAccess = "true"))
	float StayRetractedDuration = 1.0f;

	// Spike animation Settings
	UPROPERTY(EditAnywhere, Category = "Spikes", meta = (AllowPrivateAccess = "true"))
	float SpikeHeight = 50.f;

	UPROPERTY(EditAnywhere, Category = "Spikes", meta = (AllowPrivateAccess = "true"))
	float RetractSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category = "Spikes", meta = (AllowPrivateAccess = "true"))
	float EmergeSpeed = 100.f;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SpikesMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float SpeedReducePercentage = 0.5f;
	
	UFUNCTION()
	void OnSpikesHit(FHazardTriggerEventData const& Data);

	
	FTimerHandle EmergeTimerHandle;
	FTimerHandle StayEmergedTimerHandle;
	FTimerHandle StayRetractedTimerHandle;

	FVector StartLocation;

	bool bIsEmerging = false;
	bool bIsRetracting = false;

	void StartEmerge();
	void StopEmerge();

	void StartRetract();
	void StopRetract();

};
