// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BumperCarPawn.h"
#include "EnhancedPlayerInput.h"
#include "Components/WidgetComponent.h"
#include "Core/BumpurrCarsGamemode.h"
#include "Core/ScoreComponent.h"
#include "Hazards/HazardManagerWorldSubsystem.h"
#include "ui/ScorePopup.h"
#include <Kismet/GameplayStatics.h>
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "UI/GameHud.h"

DEFINE_LOG_CATEGORY(LogBumperCar);

ABumperCarPawn::ABumperCarPawn() 
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	// Set root and mesh defaults
	BumperCarBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarBody"));
	PoleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleMesh"));
	CatBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CatBodyMesh"));
	CarShieldMesh= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarShieldMesh"));

	SetRootComponent(BumperCarBody);
	PoleMesh->SetupAttachment(BumperCarBody, TEXT("PoleSocket"));
	PoleMesh->SetSimulatePhysics(false);

	CatBodyMesh->SetupAttachment(BumperCarBody, TEXT("CatBodySocket"));
	CatBodyMesh->SetSimulatePhysics(false);

	CarShieldMesh->SetupAttachment(BumperCarBody, TEXT("ShieldSocket"));
	CarShieldMesh->SetSimulatePhysics(false);
	CarShieldMesh->SetVisibility(false);
	CarShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BumperCarBody->SetSimulatePhysics(true);
	BumperCarBody->SetEnableGravity(true);
	BumperCarBody->SetCollisionObjectType(ECC_Pawn);
	BumperCarBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BumperCarBody->SetCollisionResponseToAllChannels(ECR_Overlap);

	// level bounds trigger box, powerups, hazards,...
	BumperCarBody->SetGenerateOverlapEvents(true);
	// Channel 1 == level bounds channel
	BumperCarBody->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);

	// For the outline
	BumperCarBody->SetRenderCustomDepth(true);
	CatBodyMesh->SetRenderCustomDepth(true);

	BumperCarBody->BodyInstance.bLockXRotation = true;
	BumperCarBody->BodyInstance.bLockYRotation = true;

	BumperCarBody->BodyInstance.bLockZTranslation = true;

	// Spawns with boost enabled / witout cooldown
	bCanBoost = true;

	ScoreComponent = CreateDefaultSubobject<UScoreComponent>(TEXT("ScoreComponent"));
	ScoreWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScoreWidget"));

	ScoreWidget->SetupAttachment(CatBodyMesh);
	ScoreWidget->SetWidgetSpace(EWidgetSpace::World);
	ScoreWidget->SetUsingAbsoluteRotation(true);

	LossOfControlEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LossOfControlVFX"));
	LossOfControlEffect->SetAutoActivate(false);

	CrownEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("CrownVFX"));
	CrownEffect->SetAutoActivate(false);

	BumperStrenghtEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BumperStrenghtVFX"));
	BumperStrenghtEffect->SetAutoActivate(false);
	BumperStrenghtEffect->SetupAttachment(BumperCarBody);
	
	BoostEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BoostVFX"));
	BoostEffect->SetAutoActivate(false);
	BoostEffect->SetupAttachment(BumperCarBody);
	
	ScoreMultiplierEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ScoreMultiplierVFX"));
	ScoreMultiplierEffect->SetAutoActivate(false);
	ScoreMultiplierEffect->SetupAttachment(BumperCarBody);
}	

// Called when the game starts or when spawned
void ABumperCarPawn::BeginPlay()
{
	Super::BeginPlay();

	// Assign the phyics material (if not assigned as override in the BP)
	if (PhysicsMaterial && BumperCarBody)
	{
		BumperCarBody->SetPhysMaterialOverride(PhysicsMaterial);
	}

	BumperCarBody->OnComponentHit.AddDynamic(this, &ABumperCarPawn::OnComponentHit);

	CurrentBrake = 0;
	CurrentThrottle = 0;

	GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>()->OnIceFloorActivated.AddDynamic(this, &ABumperCarPawn::IceFloorActivated);
	GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>()->OnIceFloorDeactivated.AddDynamic(this, &ABumperCarPawn::IceFloorDeactivated);

	OnPlayerHitByMeteor.AddDynamic(this, &ABumperCarPawn::OnPlayerHitByMeteorEvent);
	OnPlayerOffMap.AddDynamic(this, &ABumperCarPawn::OnPlayerOffMapEvent);
	OnPlayerHitBySpikes.AddDynamic(this, &ABumperCarPawn::OnSpikesHitEvent);

	if (ScorePopupWidgetClass)
	{
		ScoreWidget->SetWidgetClass(ScorePopupWidgetClass);
		ScoreWidget->SetRelativeLocation(ScorePopupOffset);
		ScoreWidget->SetDrawSize(FVector2D(ScorePopupDrawSize.X, ScorePopupDrawSize.Y));
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("ScorePopupWidgetClass is null in bumpercar constructor"));
	}

	// Setup physics tick;
	OnCalculateCustomPhysics.BindUObject(this, &ABumperCarPawn::SubStepTick);
	OnPlayerWonCollision.AddDynamic(this, &ABumperCarPawn::OnPlayerWonCollisionEvent);

	ShieldPowerUp();
	
	//need this to spawn these vfx on car location
	BoostEffect->AttachToComponent(BumperCarBody, FAttachmentTransformRules::KeepRelativeTransform);
	BumperStrenghtEffect->AttachToComponent(BumperCarBody, FAttachmentTransformRules::KeepRelativeTransform);
	ScoreMultiplierEffect->AttachToComponent(BumperCarBody, FAttachmentTransformRules::KeepRelativeTransform);
}


void ABumperCarPawn::SubStepTick(float DeltaTime, FBodyInstance* BodyInstance)
{
	auto const CurrVel{ BodyInstance->GetUnrealWorldVelocity() };
	PrevVel = CurrVel;

	BodyInstance->AddImpulse(ImpulsesToApply, false);
	FVector const ExpectedDeltaVel{ ImpulsesToApply / BumperCarBody->GetMass() };
	FVector const ExpectedVel{ PrevVel + ExpectedDeltaVel };
	EstimatedPreCollisionVel = ExpectedVel;

	ImpulsesToApply = FVector::ZeroVector;
	for (auto const& Pair : ImpulsesToApplyWithLocations)
	{
		BumperCarBody->AddImpulseAtLocation(Pair.Key, Pair.Value);
		EstimatedPreCollisionVel += Pair.Key / BumperCarBody->GetMass();
	} 
	ImpulsesToApplyWithLocations.Empty();

	if (bHardCapSpeed)
	{
		FVector const Velocity{ BodyInstance->GetUnrealWorldVelocity() };
		if (Velocity.Size() > HardCappedSpeed * 100)
		{
			UE_LOG(LogBumperCar, Log, TEXT("Hard capped speed"));
			FVector const ClampedVelocity{ Velocity.GetSafeNormal() * HardCappedSpeed * 100 };
			BodyInstance->SetLinearVelocity(ClampedVelocity, false);
		}
	}

	//UE_LOG(LogBumperCar, Warning, TEXT("Current velocity (car: %d): %s"), UserIndex, *CurrVel.ToString());
	//UE_LOG(LogBumperCar, Warning, TEXT("Prev (est) velocity (car: %d): %s"), UserIndex, *EstimatedPreCollisionVel.ToString());

	// Rotation / Turning
	if (not TurnInput.IsNearlyZero() and BodyInstance->GetUnrealWorldVelocity().Size() >= MinVelocityForTurning * 100)  // mps -> cmps
	{
		float const Magnitude{ static_cast<float>(TurnInput.Size()) > 1.f ? 1.f : static_cast<float>(TurnInput.Size()) };
		//float const InputMagnitude{ FVector{ static_cast<float>(TurnInput.X), static_cast<float>(TurnInput.Y).Size()};
		FVector const DesiredDirection{ FVector{TurnInput.X, TurnInput.Y, 0.f}.GetSafeNormal() };
		FVector const CurrentForwardDirection{ GetActorForwardVector() };

		float AngleBetween{ static_cast<float>(FMath::Acos(FVector::DotProduct(CurrentForwardDirection, DesiredDirection))) };
		//UE_LOG(LogBumperCar, Warning, TEXT("Angle between: %f"), AngleBetween);

		// Ensure we always rotate in the shortest direction
		FVector const CrossProduct{ FVector::CrossProduct(CurrentForwardDirection, DesiredDirection) };

		// -1 for counterclockwise, 1 for clockwise
		float RotationDirection{ static_cast<float>(FMath::Sign(CrossProduct.Z)) };

		//UE_LOG(LogBumperCar, Warning, TEXT("Rotation Dir: %f"), RotationDirection);

		if (AngleBetween >= PI)
		{
			AngleBetween = TWO_PI - AngleBetween;
			RotationDirection *= -1; // Invert direction if over 180 degrees
		}


		float const Vel{ (RotationDirection * (bLostControl ? TurnSpeed_ControlLost : TurnSpeed)) * Magnitude };
		//UE_LOG(LogBumperCar, Warning, TEXT("Vel: %f"), Vel);


		//Vel may be so high we overshoot our point -> clamp it
		//E.g: we want to turn 180 degrees to reach the point, but we turn so fast we are turning 190 degrees
		// --> clamp to 180 degrees (or - 180)
		// Velocity to get there this exact frame --> clamp value
		float const MaxRequiredAngularVelocity{ AngleBetween / DeltaTime };
		//UE_LOG(LogBumperCar, Warning, TEXT("Max Req Ang Vel: %f"), MaxRequiredAngularVelocity);
		BodyInstance->SetAngularVelocityInRadians({ 0.f, 0.f, FMath::Clamp(Vel, -MaxRequiredAngularVelocity, MaxRequiredAngularVelocity) }, false);
		//BodyInstance->AddTorqueInRadians({0.f, 0.f, FMath::Clamp(Vel, -MaxRequiredAngularVelocity, MaxRequiredAngularVelocity) }, false, false);
	}

	if (bLostControl)
	{
		return;
	}

	// Forward movement / Acceleration
	if (CurrentThrottle != 0.0f)
	{
		// Only apply acceleration when below max speed
		if (BodyInstance->GetUnrealWorldVelocity().Size() <= MaxForwardSpeed * 100)
		{
			FVector const ForwardDirection{ GetActorForwardVector().GetSafeNormal() };
			// * 100 to convert to correct unit system
			FVector const Acceleration{ ForwardDirection * AccelerationSpeed * 100 * CurrentThrottle * BumperCarBody->GetMass() };
			BodyInstance->AddForce(Acceleration, false);
			EstimatedPreCollisionVel += (Acceleration / BumperCarBody->GetMass()) * DeltaTime;
		}
		else
		{
			UE_LOG(LogBumperCar, Log, TEXT("Clamped acceleration, surpassed max forward speed"));
		
		}
	}

	// Backward movement / Braking
	if (CurrentBrake != 0.f)
	{
		FVector const CurrentVelocityCmPs{ BodyInstance->GetUnrealWorldVelocity() };

		FVector const ForwardDirection{ GetActorForwardVector().GetSafeNormal() };

		float const ProjSpeed{ static_cast<float>(FVector::DotProduct(CurrentVelocityCmPs, ForwardDirection)) };

		// Is the car about standing still
		bool const bIsMovingBackward{ ProjSpeed < -1.f };
		bool const bIsMovingForward = ProjSpeed > 1.f;
		bool const bIsNearlyStill = FMath::Abs(ProjSpeed) <= 1.f;
		if (bIsMovingForward)
		{
			bIsReversing = false;
		}

		if (bReverseOnlyMode)
		{
			if (bReverseOnlyFromStandingStill)
			{
				if (bIsNearlyStill || CurrentVelocityCmPs.IsNearlyZero(StandingStillToleranceForReverseMovement))
				{
					bIsReversing = true;
				}
			}

			if (bReverseOnlyFromStandingStill) // Start reversing only from standstill
			{
				if (bIsReversing)
				{
					// Only apply acceleration when below max speed
					// * 100 to convert to correct unit system
					if (CurrentVelocityCmPs.Size() <= MaxBackwardSpeed * 100.f)
					{
						FVector const RevAcceleration{ -ForwardDirection * ReverseAccelerationSpeed * 100.f * CurrentBrake * BumperCarBody->GetMass() };
						BodyInstance->AddForce(RevAcceleration, false);
						EstimatedPreCollisionVel += (RevAcceleration / BumperCarBody->GetMass()) * DeltaTime;
					}
				}
			}
			else // Can reverse while already moving backward
			{
				if (bIsMovingBackward || CurrentVelocityCmPs.IsNearlyZero(StandingStillToleranceForReverseMovement * 100))
				{
					// Only apply acceleration when below max speed
					// * 100 to convert to correct unit system
					if (CurrentVelocityCmPs.Size() <= MaxBackwardSpeed * 100.f)
					{
						FVector const RevAcceleration{ -ForwardDirection * ReverseAccelerationSpeed * 100.f * CurrentBrake * BumperCarBody->GetMass() };
						BodyInstance->AddForce(RevAcceleration, false);
						EstimatedPreCollisionVel += (RevAcceleration / BumperCarBody->GetMass()) * DeltaTime;
					}
				}
			}
		}
		else // Full brake + reverse behavior
		{
			if (bIsMovingForward)
			{
				// Apply braking force (opposes current forward velocity)
				FVector const FinalBrakeForce{ -ForwardDirection * BrakeForce * 100.f * CurrentBrake * BumperCarBody->GetMass() };
				BodyInstance->AddForce(FinalBrakeForce, false);
				EstimatedPreCollisionVel += (FinalBrakeForce / BumperCarBody->GetMass()) * DeltaTime;
			}
			else if (bIsNearlyStill || bIsMovingBackward)
			{
				// Apply reverse acceleration up to max reverse speed
				if (CurrentVelocityCmPs.Size() <= MaxBackwardSpeed * 100.f)
				{
					FVector const RevAcceleration{ -ForwardDirection * ReverseAccelerationSpeed * 100.f * CurrentBrake * BumperCarBody->GetMass() };
					BodyInstance->AddForce(RevAcceleration, false);
					EstimatedPreCollisionVel += (RevAcceleration / BumperCarBody->GetMass()) * DeltaTime;
				}
			}
		}
	}
}

void ABumperCarPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(BoostCooldownTimer);
	GetWorld()->GetTimerManager().ClearTimer(BoostCooldownGradientUpdate);

	Super::EndPlay(EndPlayReason);
}

void ABumperCarPawn::ResetBoostCooldown()
{
	UE_LOG(LogBumperCar, Log, TEXT("Boost cooldown reset"));

	bCanBoost = true;

	GetWorld()->GetTimerManager().ClearTimer(BoostCooldownGradientUpdate);
	UpdateBoostCooldownGradient();
}

void ABumperCarPawn::UpdateBoostCooldownGradient()
{
	if (CooldownMaterial)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(BoostCooldownTimer))
		{
			CooldownMaterial->SetScalarParameterValue("Gradient_Scale", FMath::Clamp(GetWorld()->GetTimerManager().GetTimerElapsed(BoostCooldownTimer) / BoostCooldown , 0.f, 1.f));
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(BoostCooldownGradientUpdate);
			// possibly swap material here
			CooldownMaterial->SetScalarParameterValue("Gradient_Scale", 1.f);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(BoostCooldownGradientUpdate);
		UE_LOG(LogBumperCar, Error, TEXT("Failed to create th dynamic material"));
	}
}

void ABumperCarPawn::IceFloorActivated()
{
	UE_LOG(LogBumperCar, Warning, TEXT("Ice Floor Activated!"));
	PrevLinDampening = BumperCarBody->GetLinearDamping();
	PrevAngDampening = BumperCarBody->GetAngularDamping();
	BumperCarBody->SetLinearDamping(LinearDampeningOnIce);
	BumperCarBody->SetAngularDamping(AngularDampeningOnIce);
	
}

void ABumperCarPawn::IceFloorDeactivated()
{
	UE_LOG(LogBumperCar, Warning, TEXT("Ice Floor Deactivated!"));
	BumperCarBody->SetLinearDamping(PrevLinDampening);
	BumperCarBody->SetAngularDamping(PrevAngDampening);
}

void ABumperCarPawn::PlayHapticFeedbackOnHit(EImpactLevel ImpactLevel, float DurationOverride, bool bIsDirectional, bool bIsRightSide)
{
	if (APlayerController * PC{ Cast<APlayerController>(GetController()) })
	{
		float Intensity{ LightImpactFeedbackIntensity };

		ensure(DurationOverride >= 0.f);
		float const Duration{ (DurationOverride == 0.f ? DefaultHitFeedbackDuration : DurationOverride) };

		bool bAffectOnlySmall{ false };

		//Set vibration intensity based on impact level
		switch (ImpactLevel)
		{
		case EImpactLevel::Light:
			Intensity = LightImpactFeedbackIntensity;
			bAffectOnlySmall = true;
			break;
		case EImpactLevel::Medium:
			Intensity = MediumImpactFeedbackIntensity;
			break;
		case EImpactLevel::Heavy:
			Intensity = HeavyImpactFeedbackIntensity;
			break;
		case EImpactLevel::VeryHeavy:
			Intensity = VeryHeavyImpactFeedbackIntensity;
			break;
		default:
			break;
		}

		if (bIsDirectional)
		{
			PC->PlayDynamicForceFeedback(Intensity, Duration, not bAffectOnlySmall and not bIsRightSide, not bIsRightSide, not bAffectOnlySmall and bIsRightSide, bIsRightSide);

		}
		else
		{
			PC->PlayDynamicForceFeedback(Intensity, Duration, not bAffectOnlySmall and true, true, not bAffectOnlySmall and true, true);
		}
	}
}

bool ABumperCarPawn::AreCarsFacingEachOther(ABumperCarPawn const* CarA, ABumperCarPawn const* CarB, float MinFacingAngleDegrees)
{
	FVector const DirAToB{ (CarB->GetActorLocation() - CarA->GetActorLocation()).GetSafeNormal2D() };
	FVector const DirBToA{ -DirAToB };

	float const DotA{ static_cast<float>(FVector::DotProduct(CarA->GetActorForwardVector().GetSafeNormal2D(), DirAToB)) };
	float const DotB{ static_cast<float>(FVector::DotProduct(CarB->GetActorForwardVector().GetSafeNormal2D(), DirBToA)) };

	float const AngleA{ FMath::RadiansToDegrees(FMath::Acos(DotA)) };
	float const AngleB{ FMath::RadiansToDegrees(FMath::Acos(DotB)) };

	return AngleA <= MinFacingAngleDegrees and AngleB <= MinFacingAngleDegrees;
}

EImpactLevel ABumperCarPawn::DetermineImpactLevel(float Projection) const
{
	UE_LOG(LogBumperCar, Log, TEXT("Projection: %f"), Projection);

	if (Projection < LightImpactThreshold)
	{
		UE_LOG(LogBumperCar, Log, TEXT("LIGHT IMPACT | Below LightImpactThreshold: %f"), LightImpactThreshold);
		return EImpactLevel::Light;
	}

	if (Projection < MediumImpactThreshold)
	{
		UE_LOG(LogBumperCar, Log, TEXT("MEDIUM IMPACT | Below LightImpactThreshold: %f"), MediumImpactThreshold);
		return EImpactLevel::Medium;
	}

	if (Projection < HeavyImpactThreshold)
	{
		UE_LOG(LogBumperCar, Log, TEXT("HEAVY IMPACT | Below LightImpactThreshold: %f"), HeavyImpactThreshold);
		return EImpactLevel::Heavy;
	}

	UE_LOG(LogBumperCar, Log, TEXT("VERY HEAVY IMPACT | Above HeavyImpactThreshold: %f"), HeavyImpactThreshold);
	return EImpactLevel::VeryHeavy;
}

FColor ABumperCarPawn::GetDebugColor() const
{
	if (0 == UserIndex)
	{
		return FColor::Green;
	}

	if (1 == UserIndex)
	{
		return FColor::Purple;
	}

	if (2 == UserIndex)
	{
		return FColor::Red;
	}

	if (3 == UserIndex)
	{
		return FColor::Blue;
	}

	return FColor::Black;

}

void ABumperCarPawn::HandleNoCollision(FVector const& NormalImpulse, FHitResult const& Hit)
{
	FVector const AdditionalImpulse{ Hit.ImpactNormal * NormalImpulse.Size() * PushBackForceMultiplierOnNoCollision };

	if (ApplyCollisionForcesAtLocation)
	{
		Launch(AdditionalImpulse, true, Hit.ImpactPoint);
	}
	else
	{
		Launch(AdditionalImpulse);
	}

	PlayDirectionalHapticFeedbackOnHit(EImpactLevel::Light, Hit.ImpactPoint);

	// Debug drawing
	//FColor const DebugColor{ GetDebugColor() };
	//float constexpr DebugDuration{ 2.0f };

	//FVector const ArrowEnd{ Hit.ImpactPoint + AdditionalImpulse.GetSafeNormal() * AdditionalImpulse.Size() };

	//DrawDebugDirectionalArrow(
	//	GetWorld(),
	//	Hit.ImpactPoint,
	//	ArrowEnd,
	//	20.f,
	//	DebugColor,
	//	false,
	//	DebugDuration,
	//	0,
	//	2.f
	//);
}

void ABumperCarPawn::HandleCollisionDraw(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Other)
{
	FVector AdditionalImpulse{ Hit.ImpactNormal * NormalImpulse.Size() * PushBackForceMultiplierOnDraw };

	if (Other->HasBumperStrength())
	{
		AdditionalImpulse *= Other->BuffedBumperStrength;
	}

	if (ApplyCollisionForcesAtLocation)
	{
		Launch(AdditionalImpulse, true, Hit.ImpactPoint);
	}
	else
	{
		Launch(AdditionalImpulse);
	}

	// Debug drawing
	//FColor const DebugColor{ GetDebugColor() };
	//float constexpr DebugDuration{ 2.0f };

	//FVector const ArrowEnd{ Hit.ImpactPoint + AdditionalImpulse.GetSafeNormal() * AdditionalImpulse.Size() };

	//DrawDebugDirectionalArrow(
	//	GetWorld(),
	//	Hit.ImpactPoint,
	//	ArrowEnd,
	//	20.f,
	//	DebugColor,
	//	false,
	//	DebugDuration,
	//	0,
	//	2.f
	//);

	OnPlayerHeadOnDraw.Broadcast(ImpactLevel, Other, Hit.ImpactPoint);
	LoseControl(true, MinControlLosDurationOnLoss);
	PlayDirectionalHapticFeedbackOnHit(ImpactLevel, Hit.ImpactPoint);
}

void ABumperCarPawn::HandleCollisionHeadOnDraw(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Other)
{
	FVector AdditionalImpulse{ Hit.ImpactNormal * NormalImpulse.Size() * PushBackForceMultiplierOnHeadOnDraw };

	if (Other->HasBumperStrength())
	{
		AdditionalImpulse *= Other->BuffedBumperStrength;
	}

	if (ApplyCollisionForcesAtLocation)
	{
		Launch(AdditionalImpulse, true, Hit.ImpactPoint);
	}
	else
	{
		Launch(AdditionalImpulse);
	}

	// Debug drawing
	//FColor const DebugColor{ GetDebugColor() };
	//float constexpr DebugDuration{ 2.0f };

	//FVector const ArrowEnd{ Hit.ImpactPoint + AdditionalImpulse.GetSafeNormal() * AdditionalImpulse.Size() };

	//DrawDebugDirectionalArrow(
	//	GetWorld(),
	//	Hit.ImpactPoint,
	//	ArrowEnd,
	//	20.f,
	//	DebugColor,
	//	false,
	//	DebugDuration,
	//	0,
	//	2.f
	//);

	OnPlayerHeadOnDraw.Broadcast(ImpactLevel, Other, Hit.ImpactPoint);
	LoseControl(true, MinControlLosDurationOnHeadOnDraw);
	PlayDirectionalHapticFeedbackOnHit(ImpactLevel, Hit.ImpactPoint);
	ResetAcceleration();
}

void ABumperCarPawn::HandleCollisionWin(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Loser)
{
	FVector AdditionalImpulse{ Hit.ImpactNormal * NormalImpulse.Size() * PushBackForceMultiplierOnWin };

	if (Loser->HasBumperStrength())
	{
		AdditionalImpulse *= Loser->BuffedBumperStrength;
	}

	if (ApplyCollisionForcesAtLocation)
	{
		Launch(AdditionalImpulse, true,Hit.ImpactPoint);
	}
	else
	{
		Launch(AdditionalImpulse);
	}

	// Debug drawing
	//FColor const DebugColor{ GetDebugColor() };
	//float constexpr DebugDuration{ 2.0f };

	//FVector const ArrowEnd{ Hit.ImpactPoint + AdditionalImpulse.GetSafeNormal() * AdditionalImpulse.Size() };

	//DrawDebugDirectionalArrow(
	//	GetWorld(),
	//	Hit.ImpactPoint,
	//	ArrowEnd,
	//	20.f,
	//	DebugColor,
	//	false,
	//	DebugDuration,
	//	0,
	//	2.f
	//);

	OnPlayerWonCollision.Broadcast(ImpactLevel, this, Loser, Hit.ImpactPoint);
	PlayDirectionalHapticFeedbackOnHit(ImpactLevel, Hit.ImpactPoint);

	ResetAcceleration();
}

void ABumperCarPawn::HandleCollisionLoss(EImpactLevel ImpactLevel, FVector const& NormalImpulse, FHitResult const& Hit, ABumperCarPawn* Winner)
{
	FVector AdditionalImpulse{ Hit.ImpactNormal * NormalImpulse.Size() * (bLostControl ? PushBackForceMultiplierOnLoss * PushBackForceReductionOnLossWhenControlLoss :  PushBackForceMultiplierOnLoss) };

	if (Winner->HasBumperStrength())
	{
		AdditionalImpulse *= Winner->BuffedBumperStrength;
	}

	if (ApplyCollisionForcesAtLocation)
	{
		Launch(AdditionalImpulse, true, Hit.ImpactPoint);
	}
	else
	{
		Launch(AdditionalImpulse);
	}

	// Debug drawing
	//FColor const DebugColor{ GetDebugColor() };
	//float constexpr DebugDuration{ 2.0f };

	//FVector const ArrowEnd{ Hit.ImpactPoint + AdditionalImpulse.GetSafeNormal() * AdditionalImpulse.Size() };

	//DrawDebugDirectionalArrow(
	//	GetWorld(),
	//	Hit.ImpactPoint,
	//	ArrowEnd,
	//	20.f,
	//	DebugColor,
	//	false,
	//	DebugDuration,
	//	0,
	//	2.f
	//);

	PlayDirectionalHapticFeedbackOnHit(ImpactLevel, Hit.ImpactPoint);
	LoseControl(true, MinControlLosDurationOnLoss);
	ResetAcceleration();
}

void ABumperCarPawn::HandleDrawEffects(EImpactLevel ImpactLevel, FVector const& HitLocation)
{
	//TODO unique effects
	switch (ImpactLevel)
	{
	case EImpactLevel::Light:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_1,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.25f);
		break;
	case EImpactLevel::Medium:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_2,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.5f);
		break;
	case EImpactLevel::Heavy:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_HeadOnDraw,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.75f);
		break;
	case EImpactLevel::VeryHeavy:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_HeadOnDraw,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 1.0f);
		break;
	case EImpactLevel::Count:
		break;
	}
}

void ABumperCarPawn::HandleHeadOnDrawEffects(EImpactLevel ImpactLevel, FVector const& HitLocation)
{
	//TODO unique effects

	switch (ImpactLevel)
	{
	case EImpactLevel::Light:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_1,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.25f);
		break;
	case EImpactLevel::Medium:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_2,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.5f);
		break;
	case EImpactLevel::Heavy:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_HeadOnDraw,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.75f);
		break;
	case EImpactLevel::VeryHeavy:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_HeadOnDraw,
			HitLocation,
			GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 1.0f);
		break;
	case EImpactLevel::Count:
		break;
	}
}

void ABumperCarPawn::OnPlayerOffMapEvent()
{
	auto const GameMode{ GetWorld()->GetAuthGameMode() };

	if (auto const BumpGameMode{ Cast<ABumpurrCarsGamemode>(GameMode) })
	{
		if (OffMapEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				OffMapEffect,
				GetActorLocation(),
				GetActorRotation()
			);
		}
		else
		{
			UE_LOG(LogBumperCar, Error, TEXT("Off Map Effect is not set in the bumpercar"));
		}

		if (OffMapSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, OffMapSound, GetActorLocation());
		}
		else
		{
			UE_LOG(LogBumperCar, Error, TEXT("Off Map Sound Effect is not set in the bumpercar"));
		}

		APlayerController* PlayerController{ Cast<APlayerController>(GetController()) };
		if (PlayerController && OffMapForceFeedback)
		{
			PlayerController->ClientPlayForceFeedback(OffMapForceFeedback);
		}
		else
		{
			UE_LOG(LogBumperCar, Error, TEXT("Off Map feedback Effect is not set in the bumpercar"));
		}

		ResetPosition(ReSpawnDelayOffMap);
	}
}

void ABumperCarPawn::OnPlayerHitByMeteorEvent()
{
	if (bHasShield)
	{
		return;
	}

	ResetPosition(ReSpawnDelayMeteorHit);
}

void ABumperCarPawn::OnSpikesHitEvent(float reduceSpeedPercent)
{
	ReduceSpeed(reduceSpeedPercent);
	PlayHapticFeedbackOnHit(EImpactLevel::Light);
	//only take damge if does not have shield
	if(!bHasShield)
	{
		ScoreComponent->OnSpikesReduceScore();
	}
}

// Called every frame
void ABumperCarPawn::Tick(float DeltaTime)
{
	if (BumperCarBody && BumperCarBody->GetBodyInstance())
	{
		BumperCarBody->GetBodyInstance()->AddCustomPhysics(OnCalculateCustomPhysics);
	}

	Super::Tick(DeltaTime);

	if (bLostControl)
	{
		if(CanRestoreControl())
		{
			RestoreControl();
		}
	}

	FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
	FVector ToCamera = CameraLocation - ScoreWidget->GetComponentLocation();
	FRotator LookAtRotation = ToCamera.Rotation();
	ScoreWidget->SetWorldRotation(LookAtRotation);

	UpdateCatHeadWobble(DeltaTime);
}
	

void ABumperCarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABumperCarPawn::SetUserIndex(uint32 ID)
{
	check(ID <= 3);
	UE_LOG(LogBumperCar, Warning, TEXT("Set user index in Bumper car to: %d"), ID);

	//Setup the outline 
	BumperCarBody->SetCustomDepthStencilValue(ID);
	CatBodyMesh->SetCustomDepthStencilValue(ID);
	PoleMesh->SetCustomDepthStencilValue(ID);

	UserIndex = static_cast<int32>(ID);

	if (TSoftObjectPtr<UStaticMesh> const* MeshRef{ UserMeshMap.Find(UserIndex) })
	{
		if (UStaticMesh * Mesh{ MeshRef->LoadSynchronous() })
		{
			CatBodyMesh->SetStaticMesh(Mesh);
		}
		else
		{
			UE_LOG(LogBumperCar, Error, TEXT("Failed to load CAT static mesh for UserID %d"), UserIndex);
		}
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("No CAT static mesh assigned for UserID %d"), UserIndex);
	}

	CrownEffect->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	CrownEffect->AttachToComponent(CatBodyMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FName{ "TopHeadSocket" });

	LossOfControlEffect->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	LossOfControlEffect->AttachToComponent(CatBodyMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FName{ "TopHeadSocket" });


	if (TSoftObjectPtr<UStaticMesh> const* MeshRef{ UserMeshMap_Booster.Find(UserIndex) })
	{
		if (UStaticMesh * Mesh{ MeshRef->LoadSynchronous() })
		{
			PoleMesh->SetStaticMesh(Mesh);
		}
		else
		{
			UE_LOG(LogBumperCar, Error, TEXT("Failed to load BOOSTERPOLE static mesh for UserID %d"), UserIndex);
		}
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("No BOOSTERPOLE static mesh assigned for UserID %d"), UserIndex);
	}

	// The boost cooldown gradient
	CooldownMaterial = PoleMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (CooldownMaterial)
	{
		CooldownMaterial->SetScalarParameterValue("Gradient_Scale", 1.f);
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("Failed to create the dynamic material"));
	}

	ShieldMaterial = CarShieldMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (ShieldMaterial)
	{
		ShieldMaterial->SetScalarParameterValue("Switch_Value", UserIndex);
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("Failed to create the dynamic material"));
	}

	if (TSoftObjectPtr<UMaterialInterface> const* MatRef{ UserMaterialMap_Car.Find(UserIndex) })
	{
		if (UMaterialInterface * Mat{ MatRef->LoadSynchronous() })
		{
			BumperCarBody->SetMaterial(0, Mat);
		}
		else
		{
			UE_LOG(LogBumperCar, Error, TEXT("Failed to load material for UserID %d"), UserIndex);
		}
	}


	if (TSoftObjectPtr<UMaterialInterface> const* MatRef{ UserMaterialMap_Bumper.Find(UserIndex) })
	{
		if (UMaterialInterface * Mat{ MatRef->LoadSynchronous() })
		{
			BumperCarBody->SetMaterial(1, Mat);
		}
		else
		{
			UE_LOG(LogBumperCar, Error, TEXT("Failed to load material for UserID %d"), UserIndex);
		}
	}
}

UScoreComponent* ABumperCarPawn::GetScoreComponent() const noexcept
{
	return ScoreComponent;
}

FVector ABumperCarPawn::GetVelocity() const
{
	return BumperCarBody->GetPhysicsLinearVelocity();
}

FVector const& ABumperCarPawn::GetPreviousVelocity() const noexcept
{
	return PrevVel;
}

FVector const& ABumperCarPawn::GetEstimatedPreCollisionVelocity() const noexcept
{
	return EstimatedPreCollisionVel;
}

void ABumperCarPawn::ActivateCrown()
{
	if (CrownEffect)
	{
		CrownEffect->Activate();
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("Crowneffect not set in bumpercar"));
	}
}

void ABumperCarPawn::DeActivateCrown()
{
	if (CrownEffect)
	{
		CrownEffect->DeactivateImmediate();
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("Crowneffect not set in bumpercar"));
	}
}

void ABumperCarPawn::ResetAcceleration()
{
	CurrentBrake = 0.f;
	CurrentThrottle = 0.f;
}

bool ABumperCarPawn::HasControl() const noexcept
{
	return not bLostControl;
}

class UScorePopup* ABumperCarPawn::GetScorePopup() const noexcept
{
	UScorePopup* ScoreWidgetInstance{ Cast<UScorePopup>(ScoreWidget->GetUserWidgetObject()) };
	return ScoreWidgetInstance;
}

void ABumperCarPawn::AccelerateCar(float Value)
{
	CurrentThrottle = Value;
}

void ABumperCarPawn::ReverseCar(float Value)
{
	CurrentBrake = Value;
}

void ABumperCarPawn::TurnCar(FVector2D Value)
{
	if (Value.IsNearlyZero())
	{
		TurnInput = {};
		return;
	}

	TurnInput = Value;
}

void ABumperCarPawn::Boost()
{
	if (!bCanBoost || bLostControl)
	{
		return;
	}

	//UE_LOG(LogBumperCar, Warning, TEXT("Boosting! "));

	FVector const BoostImpulse{ GetActorForwardVector() * BoostForce * BumperCarBody->GetMass() }; ImpulsesToApply += BoostImpulse;
	ImpulsesToApply += BoostImpulse;

	//vfx
	BoostEffect->Activate(true);
	bCanBoost = false;

	OnBoost.Broadcast();
	GetWorld()->GetTimerManager().SetTimer(BoostCooldownTimer, this, &ABumperCarPawn::ResetBoostCooldown, BoostCooldown, false);
	UpdateBoostCooldownGradient();
	GetWorld()->GetTimerManager().SetTimer(BoostCooldownGradientUpdate, this, &ABumperCarPawn::UpdateBoostCooldownGradient, BoostCooldown / static_cast<float>(BoostCooldownIndicatorStepSize), true);

	if (BoostSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BoostSound, GetActorLocation());
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("boost Sound Effect is not set in the bumpercar"));
	}
	}

void ABumperCarPawn::ReduceSpeed(float Percentage)
{
	if (!ensure(Percentage >= 0.0f && Percentage <= 1.f))
	{
		return;
	}

	auto const& CurrLinVel{ BumperCarBody->GetPhysicsLinearVelocity() };
	auto const& CurrAngVel{ BumperCarBody->GetPhysicsAngularVelocityInDegrees() };

	BumperCarBody->SetPhysicsLinearVelocity(CurrLinVel * (1.f - Percentage));
	BumperCarBody->SetPhysicsAngularVelocityInDegrees(CurrAngVel * (1.f - Percentage));
}

void ABumperCarPawn::Launch(FVector const& Impulse, bool bApplyAtLocation, FVector const& HitLocation, bool bResetAcceleration)
{
	if (bIsDead)
	{
		return;
	}

	if (bApplyAtLocation)
	{
		ImpulsesToApplyWithLocations.Add({ Impulse, HitLocation });
	}
	else
	{
		ImpulsesToApply += Impulse;
	}

	if (bResetAcceleration)
	{
		ResetAcceleration();
	}
}

void ABumperCarPawn::ResetPosition(float ResetTime)
{
	bIsDead = true;

	//disable controller
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->DisableInput(PlayerController);
	}
	//car disappears
	SetActorEnableCollision(false);
	if (BumperCarBody)
	{
		BumperCarBody->SetSimulatePhysics(false);
	}
	SetActorHiddenInGame(true);

	if (HasShield())
	{
		DisableShield();  // Clears HUD + timers
	}
	if (HasScoreMult())
	{
		SetScoreMult();  // Resets multiplier and HUD
	}
	if (HasBumperStrength())
	{
		ResetBumperStrength();  // Clears HUD + effect
	}
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	CurrentBrake = 0;
	CurrentThrottle = 0;

	// To do this, we need to add additional spawn point logic so 2 cars don't spawn on the same spawnpoint
	// -> boolean in spawn point for is being used, check for this in gamemode when requesting point

	//if (RespawnLocationEffectSystem)
	//{
	//	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,RespawnLocationEffectSystem,Location,Rotation, FVector(1.0f),true, true);
	//}

	//delayed transform reset
	GetWorld()->GetTimerManager().SetTimer(ResetTransformTimerHandle, this, &ABumperCarPawn::FinishResetPosition, ResetTime, false);
}
void ABumperCarPawn::OnPlayerWonCollisionEvent(EImpactLevel ImpactLevel, ABumperCarPawn* Winner, ABumperCarPawn* Loser, FVector const& HitLocation)
{
	switch (ImpactLevel)
	{
	case EImpactLevel::Light:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_1,
			HitLocation,
			Winner->GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.25f);
		break;
	case EImpactLevel::Medium:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_2,
			HitLocation,
			Winner->GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.5f);
		break;
	case EImpactLevel::Heavy:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_3,
			HitLocation,
			Winner->GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 0.75f);
		break;
	case EImpactLevel::VeryHeavy:
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			CollisionEffect_4,
			HitLocation,
			Winner->GetActorRotation()
		);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CarBumpingSound, HitLocation, 1.0f);
		break;
	case EImpactLevel::Count:
		break;
	}
}

void ABumperCarPawn::ShieldPowerUp()
{
	//UE_LOG(LogBumperCar, Warning, TEXT("Has Shield, before timer"));
	bHasShield = true;
	CarShieldMesh->SetVisibility(true);
	
	if (const auto* GameMode = Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode()))
	{
		auto GameHud = Cast<UGameHud>(GameMode->GetHUDWidget());

		if (!GameHud)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameHud is null in BumperCarPawn!"));
		}
		else
			GameHud->GetPlayerShieldImage(GetUserIndex())->SetVisibility(ESlateVisibility::Visible);
	}

	GetWorld()->GetTimerManager().ClearTimer(ShieldTimer); // Prevent overlap
	GetWorld()->GetTimerManager().SetTimer(ShieldFlickerHandle, this, &ABumperCarPawn::FlickerShield, ShieldFlickerTimer, false);
	GetWorld()->GetTimerManager().SetTimer(ShieldTimer, this, &ABumperCarPawn::DisableShield, shieldTimer,false);
}

bool ABumperCarPawn::HasShield()
{
	return bHasShield;
}

void ABumperCarPawn::DisableShield()
{
	bHasShield = false;
	//UE_LOG(LogBumperCar, Warning, TEXT("Shield ended!"));
	CarShieldMesh->SetVisibility(false);
	
	if (const auto* GameMode = Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode()))
	{
		auto GameHud = Cast<UGameHud>(GameMode->GetHUDWidget());

		if (!GameHud)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameHud is null in BumperCarPawn!"));
		}
		else
			GameHud->GetPlayerShieldImage(GetUserIndex())->SetVisibility(ESlateVisibility::Hidden);
	}

	GetWorld()->GetTimerManager().ClearTimer(ShieldTimer);
	UnFlickerShield();
	GetWorld()->GetTimerManager().ClearTimer(ShieldFlickerHandle);
	GetWorld()->GetTimerManager().ClearTimer(UnFlickerHandle);
}

void ABumperCarPawn::FlickerShield()
{
	GetWorld()->GetTimerManager().SetTimer(UnFlickerHandle, this, &ABumperCarPawn::UnFlickerShield, FlickerDuration, false);
	ShieldMaterial->SetScalarParameterValue(FName("Opacity"), ShieldFlickerBottomOpacity);
}

void ABumperCarPawn::UnFlickerShield()
{
	ShieldMaterial->SetScalarParameterValue(FName("Opacity"), ShieldFlickerTopOpacity);
	GetWorld()->GetTimerManager().SetTimer(ShieldFlickerHandle, this, &ABumperCarPawn::FlickerShield, FlickerDuration, false);
}

void ABumperCarPawn::ScoreMultPowerUp()
{
	UE_LOG(LogBumperCar, Warning, TEXT("Has Score Multiplier, before timer: "));
	bHasScoreMult = true;
	if (const auto* GameMode = Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode()))
	{
		auto GameHud = Cast<UGameHud>(GameMode->GetHUDWidget());

		if (!GameHud)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameHud is null in BumperCarPawn!"));
		}
		else
			GameHud->GetPlayerScoreMultiplierImage(GetUserIndex())->SetVisibility(ESlateVisibility::Visible);
	}
	
	scoreMult = 2.f;
	GetWorld()->GetTimerManager().ClearTimer(ScoreMultTimer); // Prevent overlap
	GetWorld()->GetTimerManager().SetTimer(ScoreMultTimer, this, &ABumperCarPawn::SetScoreMult, scoreMultTimer, false);

	ScoreMultiplierEffect->Activate();
}

bool ABumperCarPawn::HasScoreMult()
{
	return bHasScoreMult;
}

float ABumperCarPawn::GetScoreMultiplier()
{
	return scoreMult;
}

void ABumperCarPawn::SetScoreMult()
{
	if (const auto* GameMode = Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode()))
	{
		auto GameHud = Cast<UGameHud>(GameMode->GetHUDWidget());

		if (!GameHud)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameHud is null in BumperCarPawn!"));
		}
		else
			GameHud->GetPlayerScoreMultiplierImage(GetUserIndex())->SetVisibility(ESlateVisibility::Hidden);
	}

	scoreMult = 1.f;

	if (ScoreMultiplierEffect)
	{
		if (ScoreMultiplierEffect->IsActive())
		{
			//UE_LOG(LogBumperCar, Warning, TEXT("Deactivating ScoreMultiplierEffect via DeactivateImmediate()"));
			ScoreMultiplierEffect->DeactivateImmediate();
		}
	}
	else
	{
		UE_LOG(LogBumperCar, Warning, TEXT("ScoreMultiplierEffect is null!"));
	}
	bHasScoreMult = false;
	UE_LOG(LogBumperCar, Warning, TEXT("Score mult ended!"));
}

void ABumperCarPawn::BumperStrengthPowerUp()
{
	//UE_LOG(LogBumperCar, Warning, TEXT("Bumper Strength Power-Up Activated!"));
	bHasBumperStrength = true;
	if (const auto* GameMode = Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode()))
	{
		auto GameHud = Cast<UGameHud>(GameMode->GetHUDWidget());

		if (!GameHud)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameHud is null in BumperCarPawn!"));
		}
		else
			GameHud->GetPlayerBumperStrengthImage(GetUserIndex())->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Activate Niagara VFX
	ActivateBumperStrengthEffect();

	GetWorld()->GetTimerManager().ClearTimer(BumperStrengthTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(BumperStrengthTimerHandle,this,&ABumperCarPawn::ResetBumperStrength,BumperStrengthTimer,false);
}

bool ABumperCarPawn::HasBumperStrength() const
{
	return bHasBumperStrength;
}

float ABumperCarPawn::GetBumperStrength() const
{
	return BuffedBumperStrength;
}

void ABumperCarPawn::ResetBumperStrength()
{
	bHasBumperStrength = false;

	// Deactivate Niagara VFX
	DeactivateBumperStrengthEffect();
	//UE_LOG(LogBumperCar, Warning, TEXT("Bumper Strength Power-Up Expired."));
}

void ABumperCarPawn::ActivateBumperStrengthEffect()
{
	if (BumperStrenghtEffect)
	{
		BumperStrenghtEffect->Activate(true);
	}
}

void ABumperCarPawn::DeactivateBumperStrengthEffect()
{
	if (const auto* GameMode = Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode()))
	{
		auto GameHud = Cast<UGameHud>(GameMode->GetHUDWidget());

		if (!GameHud)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameHud is null in BumperCarPawn!"));
		}
		else
			GameHud->GetPlayerBumperStrengthImage(GetUserIndex())->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BumperStrenghtEffect)
	{
		BumperStrenghtEffect->DeactivateImmediate();
	}
}

void ABumperCarPawn::FinishResetPosition()
{
	if (auto BumpGameMode{ Cast<ABumpurrCarsGamemode>(GetWorld()->GetAuthGameMode()) })
	{
		auto const Transform{ BumpGameMode->GetRespawnTransform(this) };
		SetActorTransform(Transform, false, nullptr, ETeleportType::ResetPhysics);
	}

	if (bLostControl)
	{
		RestoreControl();
	}

	ResetBoostCooldown();
	ResetAcceleration();
	ShieldPowerUp();

	if (SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
	}
	else
	{
		UE_LOG(LogBumperCar, Error, TEXT("Spawn Sound Effect is not set in the bumpercar"));
	}

	if (BumperCarBody)
	{
		BumperCarBody->SetPhysicsLinearVelocity(FVector::ZeroVector);
		BumperCarBody->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		BumperCarBody->SetSimulatePhysics(true);
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	APlayerController* PlayerController{ Cast<APlayerController>(GetController()) };
	if (PlayerController)
	{
		PlayerController->EnableInput(PlayerController);
		if(RespawnForceFeedback)
		{
			PlayerController->ClientPlayForceFeedback(RespawnForceFeedback);
		}
	}

	bIsDead = false;
}

void ABumperCarPawn::TriggerCatHeadWobble(EImpactLevel ImpactLevel)
{
	bShouldWobbleCat = true;
	CatWobbleTimer = 0.f;

	//scale the head rotation offset depending on impact level
	float Scale = 1.f;
	switch (ImpactLevel)
	{
	case EImpactLevel::Light:
		Scale = 0.5f;
		break;
	case EImpactLevel::Medium:
		Scale = 1.f;
		break;
	case EImpactLevel::Heavy:
		Scale = 1.5f;
		break;
	case EImpactLevel::VeryHeavy:
		Scale = 2.f;
		break;
	default:
		break;
	}

	ScaledCatHeadRotationOffset = CatHeadRotationOffset * Scale;
}

void ABumperCarPawn::UpdateCatHeadWobble(float DeltaTime)
{
	if (!bShouldWobbleCat)
		return;

	CatWobbleTimer += DeltaTime;
	float Alpha = FMath::Clamp(CatWobbleTimer / CatWobbleDuration, 0.f, 1.f);

	float SinOffset = FMath::Sin(Alpha * PI);
	FRotator NewRotation = FRotator::ZeroRotator + ScaledCatHeadRotationOffset * SinOffset;
	CatBodyMesh->SetRelativeRotation(NewRotation);

	if (CatWobbleTimer >= CatWobbleDuration)
	{
		bShouldWobbleCat = false;
		CatBodyMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

void ABumperCarPawn::QuestionPowerUp()
{
	//Get a random number between 0 and 2 (nr of PU)
	int32 Index = FMath::RandRange(0, 2);

	switch (Index)
	{
	case 0:
		ShieldPowerUp();
		break;
	case 1:
		ScoreMultPowerUp();
		break;
	case 2:
		BumperStrengthPowerUp();
		break;
	default:
		break;
	}

	//UE_LOG(LogBumperCar, Log, TEXT("Question Mark triggered power-up #%d"), Index);
}

void ABumperCarPawn::SetPlayerImage(UTexture2D* NewImage)
{
	PlayerImage = NewImage;
}

void ABumperCarPawn::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABumperCarPawn* OtherCar{ Cast<ABumperCarPawn>(OtherActor) };
	if (not OtherCar)
	{
		return;
	}

	// Ensure function is only executed once
	if (OtherCar->GetUniqueID() < this->GetUniqueID())
	{
		return; 
	}

	// TODO shields should not make us entirely ignore collisions
	// Handle shields first
	if (bHasShield)
	{
		UE_LOG(LogBumperCar, Warning, TEXT("Shield absorbed the collision!"));
		return;
	}
	//if (OtherCar->HasShield())
	//{
	//	UE_LOG(LogBumperCar, Warning, TEXT("Opponent shield absorbed the collision!"));
	//	return;
	//}

	auto const OtherHit{ FHitResult::GetReversedHit(Hit) };
	//FVector const PostCollisionVel{ BumperCarBody->GetBodyInstance()->GetUnrealWorldVelocity() };
	//float const Mass{ BumperCarBody->GetMass() };
	//FVector const VelocityChange{ NormalImpulse / Mass };
	//FVector const ApproxPrevVel{ PostCollisionVel - VelocityChange };

	//FVector const OtherPostCollisionVel{ OtherCar->BumperCarBody->GetBodyInstance()->GetUnrealWorldVelocity() };
	//float const OtherMass{ OtherCar->BumperCarBody->GetMass() };
	//FVector const OtherVelocityChange{ NormalImpulse / OtherMass };
	//FVector const OtherApproxPrevVel{ OtherPostCollisionVel - OtherVelocityChange };

	//1. Is the highest velocity above the minimum threshold?
	if (EstimatedPreCollisionVel.Size() < MinVelocityToTriggerBump * 100
	and OtherCar->EstimatedPreCollisionVel.Size() < MinVelocityToTriggerBump * 100)
	{
		UE_LOG(LogBumperCar, Warning, TEXT("No collision, below min speed to trigger"));
		//UE_LOG(LogBumperCar, Warning, TEXT("This car prev vel: %s"), *PrevVel.ToString());
		//UE_LOG(LogBumperCar, Warning, TEXT("Other car prev vel: %s"), *OtherCar->PrevVel.ToString());

		this->HandleNoCollision(NormalImpulse, Hit);
		OtherCar->HandleNoCollision(NormalImpulse, OtherHit);

		return;
	}

	ABumperCarPawn* WinningCar{ nullptr };
	EImpactLevel ImpactLevelWinner{};
	ABumperCarPawn* LosingCar{ nullptr };

	// We must use the previous velocity (before the collision happened)
	float const ProjectionThis{ static_cast<float>(FMath::Abs(FVector::DotProduct(EstimatedPreCollisionVel, Hit.ImpactNormal))) };
	float const ProjectionOther{ static_cast<float>(FMath::Abs(FVector::DotProduct(OtherCar->EstimatedPreCollisionVel, OtherHit.ImpactNormal))) };

	//2. Are the cars facing each other?
	if (AreCarsFacingEachOther(this, OtherCar, MinFacingAngleDegreesForHeadOn))
	{
		//UE_LOG(LogBumperCar, Warning, TEXT("Cars are facing each other!"));

		//2.1 Are the velocities about equal
		if (FMath::IsNearlyEqual(ProjectionOther, ProjectionThis, HeadOnDrawProjectionTolerance))
		{
			//UE_LOG(LogBumperCar, Warning, TEXT("Cars facing each other + equal velocity --> Head on draw!"));

			WinningCar = this;
			LosingCar = OtherCar;

			ImpactLevelWinner = DetermineImpactLevel(ProjectionThis);

			// Handle head on draw logic
			WinningCar->HandleCollisionHeadOnDraw(ImpactLevelWinner, NormalImpulse, Hit, LosingCar);
			LosingCar->HandleCollisionHeadOnDraw(ImpactLevelWinner, NormalImpulse, OtherHit, WinningCar);

			HandleHeadOnDrawEffects(ImpactLevelWinner, Hit.ImpactPoint);

			return;
		}
	}

	//3. Are the projections about equal?
	if (FMath::IsNearlyEqual(ProjectionOther, ProjectionThis, DrawProjectionTolerance))
	{
		//UE_LOG(LogBumperCar, Warning, TEXT("Car velocity projections are about equal, it's a draw!"));

		WinningCar = this;
		LosingCar = OtherCar;

		ImpactLevelWinner = DetermineImpactLevel(ProjectionThis);

		// Handle draw logic
		WinningCar->HandleCollisionDraw(ImpactLevelWinner, NormalImpulse, Hit, LosingCar);
		LosingCar->HandleCollisionDraw(ImpactLevelWinner, NormalImpulse, OtherHit, WinningCar);

		HandleDrawEffects(ImpactLevelWinner, Hit.ImpactPoint);

		return;
	}


	//4. Select winner and loser
	FHitResult WinningHit;
	FHitResult LosingHit;

	if (ProjectionOther > ProjectionThis)
	{
		WinningCar = OtherCar;
		WinningHit = OtherHit;
		ImpactLevelWinner = DetermineImpactLevel(ProjectionOther);

		LosingCar = this;
		LosingHit = Hit;
	}
	else
	{
		WinningCar = this;
		WinningHit = Hit;
		ImpactLevelWinner = DetermineImpactLevel(ProjectionThis);

		LosingCar = OtherCar;
		LosingHit = OtherHit;
	}

	//UE_LOG(LogBumperCar, Warning, TEXT("Bumper Car Hit (Winning bumper car): %s"), *WinningCar->GetName());
	//UE_LOG(LogBumperCar, Warning, TEXT("Bumper Car Hit (Losing bumper car): %s"), *LosingCar->GetName());
	//UE_LOG(LogBumperCar, Warning, TEXT("Winning car prev vel: %s"), *WinningCar->PrevVel.ToString());
	//UE_LOG(LogBumperCar, Warning, TEXT("Losing car prev vel: %s"), *LosingCar->PrevVel.ToString());

	// Handle Win / Lose logic
	LosingCar->HandleCollisionLoss(ImpactLevelWinner, NormalImpulse, LosingHit, WinningCar);
	WinningCar->HandleCollisionWin(ImpactLevelWinner, NormalImpulse, WinningHit, LosingCar);

	TriggerCatHeadWobble(ImpactLevelWinner);
	OtherCar->TriggerCatHeadWobble(ImpactLevelWinner);

}

void ABumperCarPawn::PlayDirectionalHapticFeedbackOnHit(EImpactLevel ImpactLevel, FVector const& Hit, float DurationOverride)
{
	FVector const Forward{ GetActorForwardVector() };
	FVector const ToHit{ (Hit - GetActorLocation()).GetSafeNormal() };

	float const SideDot{ static_cast<float>(FVector::CrossProduct(Forward, ToHit).Z) };

	constexpr float CenterMargin{ 0.3f };

	if (FMath::Abs(SideDot) < CenterMargin)
	{
		PlayHapticFeedbackOnHit(ImpactLevel, DurationOverride);
	}
	else
	{
		bool const bHitOnRight{ SideDot < 0.f };
		PlayHapticFeedbackOnHit(ImpactLevel, DurationOverride,true, bHitOnRight);
	}
}

void ABumperCarPawn::RestoreControl()
{
	OnControlRestore.Broadcast();
	bLostControl = false;

	LossOfControlEffect->DeactivateImmediate();
	GetWorld()->GetTimerManager().ClearTimer(ControlLossTimerHandle);
}

void ABumperCarPawn::LoseControl(bool bIsTimeBased, float ControlLossTime)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ControlLossTimerHandle))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Not activating timer, already running! "));
		return;
	}

	if (bLostControl)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Does not have control atm, will not set control to false again!"));
		return;
	}

	OnControlLoset.Broadcast();
	bLostControl = true;
	LossOfControlEffect->Activate();

	if (bIsTimeBased)
	{
		// Start a timer to restore control after a delay
		GetWorld()->GetTimerManager().SetTimer(
			ControlLossTimerHandle,
			this,
			&ABumperCarPawn::RestoreControl,
			ControlLossTime,
			false // Single shot timer
		);

		//UE_LOG(LogTemp, Log, TEXT("Control Lost, Timer Started. Timer Duration: %.2f seconds"), ControlLossTime);
	}
	else
	{
		//UE_LOG(LogTemp, Log, TEXT("Control Lost, not timer based will restore when controllable speed is reached."));
		GetWorld()->GetTimerManager().ClearTimer(ControlLossTimerHandle);
	}
}

bool ABumperCarPawn::CanRestoreControl() const
{
	//UE_LOG(LogTemp, Warning, TEXT("prev vel: %f"), PrevVel.Size());
	//UE_LOG(LogTemp, Warning, TEXT("max vel: %f"), MaxForwardSpeed_ControlRestore * 100);

	return  (not GetWorld()->GetTimerManager().IsTimerActive(ControlLossTimerHandle)
			and (PrevVel.Size() <= (MaxForwardSpeed_ControlRestore * 100)));
}

