// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/LightSwitchHazardActor.h"
#include <Kismet/GameplayStatics.h>
#include "Components/SkyLightComponent.h"
#include "Hazards/HazardManagerWorldSubsystem.h"

// Sets default values
ALightSwitchHazardActor::ALightSwitchHazardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardType = EHazardType::Wall;

	LightSwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightSwitchMesh"));
	RootComponent = LightSwitchMesh;
	
	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::Hit;
	HazardTriggerComponent->TargetComponent = LightSwitchMesh;

	Light = nullptr;

	// Debugging in Constructor
	UE_LOG(LogTemp, Warning, TEXT("LightSwitchHazardActor constructor called!"));
}

// Called when the game starts or when spawned
void ALightSwitchHazardActor::BeginPlay()
{
	Super::BeginPlay();

	LightSwitchMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LightSwitchMesh->SetNotifyRigidBodyCollision(true);
	LightSwitchMesh->SetCollisionObjectType(ECC_WorldStatic);
	LightSwitchMesh->SetCollisionResponseToAllChannels(ECR_Block);
	
	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &ALightSwitchHazardActor::OnLightSwitchHit);

	bIsOn = true;

	// Now search for the SkyLight actor by name
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SkyLight actors found at all!"));
	}

	for (AActor* Actor : FoundActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Actor: %s"), *Actor->GetName());

		// Check for the name of the actor (this should match the SkyLight's name in the editor)
		if (Actor->GetName() == "SkyLight_0") // Replace with exact name of your SkyLight
		{
			Light = Cast<ASkyLight>(Actor);
			if (Light)
			{
				UE_LOG(LogTemp, Warning, TEXT("SkyLight found by name in BeginPlay!"));
				break; // Stop after the first match
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Found actor is not a SkyLight!"));
			}
		}
	}

	if (!Light)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkyLight not found in BeginPlay!"));
	}
}

// Called every frame
void ALightSwitchHazardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlayedErrorSound += DeltaTime;
	if (PlayedErrorSound >= .2f)
	{
		PlayedErrorSound = .2f;
	}
}

void ALightSwitchHazardActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	USkyLightComponent* SkyLightComponent{ Cast<USkyLightComponent>(Light->GetLightComponent()) };
	SkyLightComponent->SetIntensity(LightsOnIntensity);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void ALightSwitchHazardActor::OnLightSwitchHit(FHazardTriggerEventData const& Data)
{
	//UE_LOG(LogTemp, Warning, TEXT("collision with lightswitch!"));
	if (!bIsOn)
	{
		if (LightSwitchFailSound and PlayedErrorSound >= .2f)
		{
			UGameplayStatics::PlaySoundAtLocation(this, LightSwitchFailSound, Data.HitResult.ImpactPoint);
			PlayedErrorSound = 0.F;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Light switch fail sound effect not set"));
		}

		return;
	}

	SetLightSwitchState(false);

	if (Light)
	{
		// Directly get the SkyLightComponent from the ASkyLight actor
		USkyLightComponent* SkyLightComponent = Cast<USkyLightComponent>(Light->GetLightComponent());
		if (SkyLightComponent)
		{
			SkyLightComponent->SetIntensity(LightsOffLightIntensity);
			UE_LOG(LogTemp, Warning, TEXT("SkyLight turned off!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkyLight component not found!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SkyLight not found!"));
	}

	GetWorldTimerManager().SetTimer(CooldownTimerHandle, this, &ALightSwitchHazardActor::StartCooldown, CooldownTime, false);
}

void ALightSwitchHazardActor::SetLightSwitchState(bool bActive)
{
	bIsOn = bActive;
	if (LightSwitchActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LightSwitchActivateSound, GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Light switch activate sound effect not set"));
	}

	LightSwitchMesh->SetMaterial(0, bActive ? IsOnMaterial : IsOffMaterial);
}

void ALightSwitchHazardActor::ReactivateLightSwitch()
{
	SetLightSwitchState(true);
	UE_LOG(LogTemp, Warning, TEXT("LightSwitch reactivated!"));
}

void ALightSwitchHazardActor::StartCooldown()
{
	if (Light)
	{
		USkyLightComponent* SkyLightComponent{ Cast<USkyLightComponent>(Light->GetLightComponent()) };
		SkyLightComponent->SetIntensity(LightsOnIntensity);

		UE_LOG(LogTemp, Warning, TEXT("SkyLight turned on!"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("SkyLight not found!"));

	GetWorldTimerManager().SetTimer(SwitchReactivationTimerHandle, this, &ALightSwitchHazardActor::ReactivateLightSwitch, ReactivationTime, false);
}

