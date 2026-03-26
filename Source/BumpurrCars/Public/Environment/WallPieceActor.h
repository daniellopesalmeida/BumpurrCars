// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WallPieceActor.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogWallPiece, Log, All);

UCLASS()
// Class to encapsulate a single wall piece, pieces that can be opened will have the "openable" tag.
class BUMPURRCARS_API AWallPieceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AWallPieceActor();

	void OpenWall();
	void CloseWall();

	bool IsWallOpen();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* OnWallHitNiagaraSystem;

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* ForceFeedbackEffect;

private:
	UFUNCTION()
	void FinishOpenWall();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Wall", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WallMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* PreOpenMaterial;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* WallBumpSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall", meta = (AllowPrivateAccess = "true"))
	bool bIsWallOpen{ false };

	UPROPERTY(EditAnywhere)
	float PreOpenDelay = 1.0f;

	float LastBumpSoundTime = 0.0f;

	//CD for sound 
	UPROPERTY(EditAnywhere, Category = "Audio Cooldown")
	float BumpSoundCooldown = .2f;

	UPROPERTY(EditDefaultsOnly)
	float VelThresholdForPushback{ 10.f };
	UPROPERTY(EditDefaultsOnly)
	float PushBackModifier{ 2.f };
	UPROPERTY(EditDefaultsOnly)
	float PushBackFacingThreshold{.6f};

	FTimerHandle DelayHandle;
	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
