// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include <Engine/SkyLight.h>
#include "Hazards/BaseHazard.h"
#include "LightSwitchHazardActor.generated.h"

UCLASS()
class BUMPURRCARS_API ALightSwitchHazardActor : public ABaseHazard
{
	GENERATED_BODY()
	
public:	
	ALightSwitchHazardActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	ASkyLight* Light;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* LightSwitchActivateSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* LightSwitchFailSound;

	UPROPERTY(EditDefaultsOnly)
	float LightsOffLightIntensity{ .1f };

	UPROPERTY(EditDefaultsOnly)
	float LightsOnIntensity{ 1.f };
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LightSwitchMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float ReactivationTime = 5.0f; //time still TBD, just for testing

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> IsOnMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> IsOffMaterial;

	FTimerHandle SwitchReactivationTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float CooldownTime = 20.f;
	FTimerHandle CooldownTimerHandle;
	float PlayedErrorSound{ 0.f };

	bool bIsOn{ true };

	UFUNCTION()
	void OnLightSwitchHit(FHazardTriggerEventData const& Data);

	UFUNCTION()
	void SetLightSwitchState(bool bActive);

	UFUNCTION()
	void ReactivateLightSwitch();

	UFUNCTION()
	void StartCooldown();
};
