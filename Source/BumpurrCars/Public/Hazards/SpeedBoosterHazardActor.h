// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Hazards/BaseHazard.h"
#include "SpeedBoosterHazardActor.generated.h"

UCLASS()
class BUMPURRCARS_API ASpeedBoosterHazardActor : public ABaseHazard
{
	GENERATED_BODY()
	
public:	
	ASpeedBoosterHazardActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	bool bRandomizeImpulseStrengthOnHit{ true };
	UPROPERTY(EditDefaultsOnly)
	float MinImpulseStrength{ 300.0f };
	UPROPERTY(EditDefaultsOnly)
	float MaxImpulseStrength{ 450.0f };

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* SpeedBoosterHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* BumpercarTrailHitEffect;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* SpeedBoostSound;
private:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SpeedBoosterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;

	UFUNCTION()
	void OnSpeedBoosterHit(FHazardTriggerEventData const& Data);
};
