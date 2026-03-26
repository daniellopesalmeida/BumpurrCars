// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ui/ScorePopup.h"
#include "BumperCarPawn.generated.h"


UENUM(BlueprintType)
enum class EImpactLevel : uint8
{
	Light		UMETA(DisplayName = "Light"),
	Medium      UMETA(DisplayName = "Medium"),
	Heavy		UMETA(DisplayName = "Heavy"),
	VeryHeavy	UMETA(DisplayName = "VeryHeavy"),

	Count		UMETA(Hidden)
};

DECLARE_LOG_CATEGORY_EXTERN(LogBumperCar, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnControlRestore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerOffMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerHitByMeteor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHitBySpikes, float, reduceSpeedPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerHeadOnDraw, EImpactLevel, ImpactLevel, ABumperCarPawn*, Other, FVector const&, HitLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPlayerWonCollision, EImpactLevel, ImpactLevel, class ABumperCarPawn*, Winner, class ABumperCarPawn*, Loser, FVector const&, HitLocation);

class UScoreComponent;


UCLASS(Blueprintable)
class BUMPURRCARS_API ABumperCarPawn : public APawn
{
	GENERATED_BODY()

public:
	ABumperCarPawn();
	void SetUserIndex(uint32 ID);
	[[nodiscard]] int32 GetUserIndex() const noexcept { return UserIndex; }
	[[nodiscard]] UScoreComponent* GetScoreComponent() const noexcept;

	[[nodiscard]] class UScorePopup* GetScorePopup() const noexcept;

	virtual FVector GetVelocity() const override;
	FVector const& GetPreviousVelocity() const noexcept;
	FVector const& GetEstimatedPreCollisionVelocity() const noexcept;
	void ActivateCrown();
	void DeActivateCrown();

	void ResetAcceleration();

	bool HasControl() const noexcept;

	void PlayDirectionalHapticFeedbackOnHit(EImpactLevel ImpactLevel, FVector const& Hit, float DurationOverride = 0.f);
	void PlayHapticFeedbackOnHit(EImpactLevel impactLevel, float DurationOverride = 0.f, bool bIsDirectional = false, bool bIsRightSide = false);

#pragma region Events
	// Movement
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBoost OnBoost;

	// Collisions
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerHeadOnDraw OnPlayerHeadOnDraw;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerWonCollision OnPlayerWonCollision;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnControlRestore OnControlRestore;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnControlRestore OnControlLoset;

	// "death" & "damaged"
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerOffMap OnPlayerOffMap;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerHitByMeteor OnPlayerHitByMeteor;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerHitBySpikes OnPlayerHitBySpikes;

#pragma endregion

#pragma region MovementFunctions
	void AccelerateCar(float Value);
	void ReverseCar(float Value);
	void TurnCar(FVector2D Value);
	void Boost();
	void ReduceSpeed(float Percentage);

	// Launch the car
	void Launch(FVector const& Impulse, bool bApplyAtLocation = false, FVector const& HitLocation = { }, bool bResetAcceleration = true);

	//shield 
	void ShieldPowerUp();
	bool HasShield();

	//score mult
	void ScoreMultPowerUp();
	bool HasScoreMult();
	float GetScoreMultiplier();

	//strength pu
	void BumperStrengthPowerUp();
	bool HasBumperStrength() const;
	float GetBumperStrength() const;

	//question mark
	void QuestionPowerUp();

	//player image
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetPlayerImage(UTexture2D* NewImage);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UTexture2D* GetPlayerImage() const { return PlayerImage; }

#pragma endregion

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void SubStepTick(float DeltaTime, FBodyInstance* BodyInstance);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Set the car position & reset all necessary properties to effictively "respawn" the car at a given transform.
	void ResetPosition(float ResetTime);


	UFUNCTION()
	void OnPlayerWonCollisionEvent(EImpactLevel ImpactLevel, class ABumperCarPawn* Winner, class ABumperCarPawn* Loser, FVector const& HitLocation);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> ScorePopupWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FVector ScorePopupOffset{ 0, 0, 250.f };
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FVector ScorePopupDrawSize{ 300.f, 800.f, 0.f };

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* LossOfControlEffect;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* CrownEffect;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* BumperStrenghtEffect;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* ScoreMultiplierEffect;

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* OffMapEffect;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* BoostEffect;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* OffMapSound;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* SpawnSound;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* BoostSound;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* CarBumpingSound;

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* RespawnForceFeedback;

	UPROPERTY(EditAnywhere)
	class UForceFeedbackEffect* OffMapForceFeedback;

	// player avatar image
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HighScore UI")
	UTexture2D* PlayerImage;
private:
#pragma region CoreComponentsAndMaterials
	UPROPERTY(EditDefaultsOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TMap<int32, TSoftObjectPtr<UStaticMesh>> UserMeshMap;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TMap<int32, TSoftObjectPtr<UStaticMesh>> UserMeshMap_Booster;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TMap<int32, TSoftObjectPtr<UMaterialInterface>> UserMaterialMap_Car;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
	TMap<int32, TSoftObjectPtr<UMaterialInterface>> UserMaterialMap_Bumper;


	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true", ToolTip = "The body of the bumper car. This component is physics-enabled and handles collisions."))
	// Body of the bumper car
	UStaticMeshComponent* BumperCarBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PoleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* CatBodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* CarShieldMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UScoreComponent* ScoreComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* ScoreWidget;


	UPROPERTY()
	// Dynamic material, stored because we want to change a parameter for the dash cooldown
	UMaterialInstanceDynamic* CooldownMaterial{ nullptr };

	UPROPERTY()
	// Dynamic material, stored because we want to change a parameter for shield color
	UMaterialInstanceDynamic* ShieldMaterial{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (AllowPrivateAccess = "true"))
	UPhysicalMaterial* PhysicsMaterial;
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UserInfo", meta = (AllowPrivateAccess = "true", ToolTip = "The user & controller ID of the player"))
	int32 UserIndex;

#pragma region PhysicsAndMovement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ToolTip = "Maximum acceleration force applied to the car"))
	bool bHardCapSpeed{ true };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ToolTip = "Maximum acceleration force applied to the car"))
	float HardCappedSpeed{ 10000.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ToolTip = "Maximum acceleration force applied to the car"))
	float AccelerationSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ToolTip = "Maximum acceleration force applied to the car"))
	float ReverseAccelerationSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ToolTip = "Braking force applied to slow down the car"))
	float BrakeForce = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxForwardSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxBackwardSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MinVelocityForTurning = 10.f;
	
	// Radians per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ToolTip = "Radians per second"))
	float TurnSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bReverseOnlyMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bReverseOnlyFromStandingStill = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", EditCondition = "!bReverseOnlyFromStandingStill"))
	float StandingStillToleranceForReverseMovement = 1.f;

	//ice properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|On Ice", meta = (AllowPrivateAccess = "true"))
	float LinearDampeningOnIce = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|On Ice", meta = (AllowPrivateAccess = "true"))
	float AngularDampeningOnIce = 0.2f;

	// Not accessible thro blueprints
	float CurrentThrottle{ 0.f };
	float CurrentBrake{ 0.f };

	FVector PrevVel{};
	FVector EstimatedPreCollisionVel{};
	FVector ImpulsesToApply{};
	TArray<TPair<FVector, FVector>> ImpulsesToApplyWithLocations;

	float PrevLinDampening{};
	float PrevAngDampening{};

	bool bIsReversing{ false };

	FVector2D TurnInput;
	FCalculateCustomPhysics OnCalculateCustomPhysics;
#pragma endregion

#pragma region Boosting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boosting", meta = (AllowPrivateAccess = "true"))
	float BoostForce{ 500.f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boosting", meta = (AllowPrivateAccess = "true"))
	float BoostCooldown{ 2.f };

	bool bCanBoost{ true };

	FTimerHandle BoostCooldownTimer;
	FTimerHandle BoostCooldownGradientUpdate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boosting", meta = (AllowPrivateAccess = "true"))
	int32 BoostCooldownIndicatorStepSize{ 8 };

	void ResetBoostCooldown();
	void UpdateBoostCooldownGradient();
#pragma endregion

#pragma region Collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	bool ApplyCollisionForcesAtLocation{ true };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float MinVelocityToTriggerBump{ 3.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float MinFacingAngleDegreesForHeadOn{ 30.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float DrawProjectionTolerance{ 30.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float HeadOnDrawProjectionTolerance{ 30.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float PushBackForceMultiplierOnNoCollision{ .2f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float PushBackForceMultiplierOnDraw{ .35f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float PushBackForceMultiplierOnHeadOnDraw{ .55f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float PushBackForceMultiplierOnLoss{ 1.2f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float PushBackForceReductionOnLossWhenControlLoss{ .5f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float PushBackForceMultiplierOnWin{ .6f };

	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* CollisionEffect_1;
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* CollisionEffect_2;
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* CollisionEffect_3;
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* CollisionEffect_4;

	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* CollisionEffect_HeadOnDraw;

	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float DefaultHitFeedbackDuration{ .3f };
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float LightImpactFeedbackIntensity{ .5f};
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float MediumImpactFeedbackIntensity{ .65f };
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float HeavyImpactFeedbackIntensity{ .85f };
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	float VeryHeavyImpactFeedbackIntensity{ 1.f };

	// Collision impact thresholds
	UPROPERTY(EditAnywhere, Category = "Collision|Impact Thresholds")
	float LightImpactThreshold = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Collision|Impact Thresholds")
	float MediumImpactThreshold = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Collision|Impact Thresholds")
	float HeavyImpactThreshold = 10000.f;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	static bool AreCarsFacingEachOther(ABumperCarPawn const* CarA, ABumperCarPawn const* CarB, float MinFacingAngleDegrees);
	EImpactLevel DetermineImpactLevel(float Projection) const;
	FColor GetDebugColor() const;

	void HandleNoCollision(FVector const& NormalImpulse, FHitResult const& Hit);
	void HandleCollisionDraw(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Other);
	void HandleCollisionHeadOnDraw(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Other);
	void HandleCollisionWin(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Loser);
	void HandleCollisionLoss(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Winner);

	void HandleDrawEffects(EImpactLevel ImpactLevel, FVector const& HitLocation);
	void HandleHeadOnDrawEffects(EImpactLevel ImpactLevel, FVector const& HitLocation);
#pragma endregion
#pragma region ControlLoss
	void RestoreControl();
	void LoseControl(bool bIsTimeBased = false, float ControlLossTime = 0.f);
	[[nodiscard]] bool CanRestoreControl() const;

	FTimerHandle ControlLossTimerHandle;
	bool bLostControl{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Loss", meta = (AllowPrivateAccess = "true"))
	float MinControlLosDurationOnLoss{ .2f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Loss", meta = (AllowPrivateAccess = "true"))
	float MinControlLosDurationOnHeadOnDraw{ .2f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Loss", meta = (AllowPrivateAccess = "true"))
	float MaxForwardSpeed_ControlRestore = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Control Loss", meta = (AllowPrivateAccess = "true", ToolTip = "Radians per second"))
	float TurnSpeed_ControlLost{ 100.0f };
#pragma endregion
#pragma region HazardsAndEvents
	//ice floor
	UFUNCTION()
	void IceFloorActivated();
	UFUNCTION()
	void IceFloorDeactivated();
	UFUNCTION()
	void OnPlayerOffMapEvent();

	UFUNCTION()
	void OnPlayerHitByMeteorEvent();

	UFUNCTION()
	void OnSpikesHitEvent(float reduceSpeedPercent);
#pragma endregion
#pragma region PowerUps
	bool bHasShield{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float shieldTimer{ 2.0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float ShieldFlickerTimer{ 1.f };
	FTimerHandle ShieldFlickerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float FlickerDuration{ .2f };
	FTimerHandle UnFlickerHandle;;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float ShieldFlickerBottomOpacity{ .01f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float ShieldFlickerTopOpacity{ .1f };


	UFUNCTION()
	void DisableShield();
	UFUNCTION()
	void FlickerShield();
	UFUNCTION()
	void UnFlickerShield();

	FTimerHandle ShieldTimer;

	bool bHasScoreMult{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float scoreMultTimer{ 3.0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float scoreMult{ 1.0 };

	UFUNCTION()
	void SetScoreMult();

	FTimerHandle ScoreMultTimer;

	//bumper strength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUps", meta = (AllowPrivateAccess = "true"))
	float BumperStrengthTimer = 4.0f;
	FTimerHandle BumperStrengthTimerHandle;

	bool bHasBumperStrength = false;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float BuffedBumperStrength = 2.0f;

	UFUNCTION()
	void ResetBumperStrength();
	UFUNCTION()
	void ActivateBumperStrengthEffect();
	UFUNCTION()
	void DeactivateBumperStrengthEffect();

	//reset delay
	UFUNCTION()
	void FinishResetPosition();

	//Timer handle for reset delay
	FTimerHandle ResetTransformTimerHandle;

	//UPROPERTY(EditDefaultsOnly, Category = "Respawn Effect")
	//UNiagaraSystem* RespawnLocationEffectSystem;

	UPROPERTY(EditDefaultsOnly)
	float ReSpawnDelayOffMap{1.f};

	UPROPERTY(EditDefaultsOnly)
	float ReSpawnDelayMeteorHit{.25f};

	bool bIsDead = false;

	//cat wobble on colision
	UFUNCTION()
	void TriggerCatHeadWobble(EImpactLevel ImpactLevel);
	UFUNCTION()
	void UpdateCatHeadWobble(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, Category = "Cat Wobble")
	bool bShouldWobbleCat = false;
	
	float CatWobbleTimer = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "Cat Wobble")
	float CatWobbleDuration = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category = "Cat Wobble")
	FRotator CatHeadRotationOffset = FRotator(-10.f, 0.f, 0.f);
	UPROPERTY(VisibleAnywhere, Category = "CatWobble")
	FRotator ScaledCatHeadRotationOffset = FRotator::ZeroRotator;

#pragma endregion
};
