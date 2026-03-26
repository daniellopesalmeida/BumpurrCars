// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Hazards/BaseHazard.h"
#include "IceCubeHazardActor.generated.h"


UCLASS()
class BUMPURRCARS_API AIceCubeHazardActor : public ABaseHazard
{
	GENERATED_BODY()
	
public:	
	AIceCubeHazardActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* IceBreakEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* IceBreakSound;

	UPROPERTY(EditDefaultsOnly)
	float MinRotSpeed;

	UPROPERTY(EditDefaultsOnly)
	float MaxRotSpeed;

	float RotSpeed;

private:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* IceCubeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;


	UFUNCTION()
	void OnIceCubeHit(FHazardTriggerEventData const& Data);


};
