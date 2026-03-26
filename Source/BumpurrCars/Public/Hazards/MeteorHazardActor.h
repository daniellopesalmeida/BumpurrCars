// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hazards/BaseHazard.h"
#include "MeteorHazardActor.generated.h"

UCLASS()
class BUMPURRCARS_API AMeteorHazardActor : public ABaseHazard
{
	GENERATED_BODY()
	
public:	
	AMeteorHazardActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* MeteorIndicatorEffect;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* MeteorFallingEffect;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* ImpactSound;
	
	UPROPERTY(EditDefaultsOnly)
	class USoundBase* MeteorFallSound;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> ImpactShake;

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* ImpactForceFeedbackEffect;

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* PushForceFeedbackEffect;

private:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeteorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHazardTriggerComponent* HazardTriggerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float FallDelay = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float InitialFallSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float MeteorDestroyDelay = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float LaunchRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float LaunchForce = 100.f;

	FTimerHandle DestroyTimerHandle;

	FTimerHandle StartFallingTimerHandle;

	bool bHit{ false };

	UFUNCTION()
	void OnMeteorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnMeteorOverlap(FHazardTriggerEventData const& TriggerData);
};
