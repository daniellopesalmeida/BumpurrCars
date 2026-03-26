// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorActor.generated.h"

UCLASS()
class BUMPURRCARS_API AFloorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFloorActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* FloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor|Ice", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> IceFloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor|Ice", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPhysicalMaterial> IceFloorPhysicsMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor|Normal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> FloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor|Normal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> FloorMaterialEndGame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor|Normal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPhysicalMaterial> FloorPhysicsMaterial;

	bool bIsIceFloor{ false };

	UFUNCTION()
	void OnIceFloorActivated();
	UFUNCTION()
	void OnIceFloorDeactivated();

	UFUNCTION()
	void OnEndGameEventActivated();
};
