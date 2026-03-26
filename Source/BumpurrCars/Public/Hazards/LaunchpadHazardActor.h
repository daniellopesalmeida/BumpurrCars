// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Hazards/BaseHazard.h"
#include "LaunchpadHazardActor.generated.h"

UCLASS()
class BUMPURRCARS_API ALaunchpadHazardActor : public ABaseHazard
{
	GENERATED_BODY()

public:
	ALaunchpadHazardActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* LaunchBumpPadHitEffect;

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* HitForceFeedbackEffect;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* LaunchPadHitSound;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LaunchpadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaunchpadData", meta = (AllowPrivateAccess = "true"))
	bool bApplyAtLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaunchpadData", meta = (AllowPrivateAccess = "true"))
	float LaunchBoostModifier = 1.f;
	UFUNCTION()
	void OnLaunchpadHit(FHazardTriggerEventData const& Data);
};
