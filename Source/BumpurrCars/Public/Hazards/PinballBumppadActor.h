// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Hazards/BaseHazard.h"
#include "PinballBumppadActor.generated.h"

UCLASS()
class BUMPURRCARS_API APinballBumppadActor : public ABaseHazard
{
	GENERATED_BODY()
	
public:	
	APinballBumppadActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* PinballBumpPadHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* LaunchBumpPadHitEffect;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* PinballBumpingSound;

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* HitForceFeedbackEffect;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PinballBumpPadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BumppadData", meta = (AllowPrivateAccess = "true"))
	float LaunchBoostModifier = 1.62f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BumppadData", meta = (AllowPrivateAccess = "true"))
	float RandDeg = 30.f;

	UFUNCTION()
	void OnBumpPadHit(FHazardTriggerEventData const& Data);

};
