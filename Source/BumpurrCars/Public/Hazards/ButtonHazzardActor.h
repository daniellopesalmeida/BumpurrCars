// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Hazards/BaseHazard.h"

#include "ButtonHazzardActor.generated.h"


UCLASS()
class BUMPURRCARS_API AButtonHazzardActor : public ABaseHazard
{
	GENERATED_BODY()
	
public:	
	AButtonHazzardActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* ButtonActivateSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	class USoundBase* ButtonFailSound;

	float PlayedErrorSound{.2f};

private:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ButtonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> IsOnMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> IsOffMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float ReactivationTime = 5.0f; //time still TBD, just for testing

	bool bIsActive{ true };

	FTimerHandle ReactivationTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AActor>> SpawnAbleHazards;

	UFUNCTION()
	void OnButtonHit(FHazardTriggerEventData const& Data);

	UFUNCTION()
	void SetButtonState(bool bActive);

	UFUNCTION()
	void ReactivateButton();
};
