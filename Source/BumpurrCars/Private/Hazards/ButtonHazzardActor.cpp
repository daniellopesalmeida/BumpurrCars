// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/ButtonHazzardActor.h"
#include "Hazards/HazardManagerWorldSubsystem.h"
#include "Characters/BumperCarPawn.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AButtonHazzardActor::AButtonHazzardActor()
{
	HazardType = EHazardType::Wall;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	RootComponent = ButtonMesh;

	ButtonMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ButtonMesh->SetNotifyRigidBodyCollision(true);
	ButtonMesh->SetCollisionObjectType(ECC_WorldStatic);
	ButtonMesh->SetCollisionResponseToAllChannels(ECR_Block);

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::Hit;
	HazardTriggerComponent->TargetComponent = ButtonMesh;
}

// Called when the game starts or when spawned
void AButtonHazzardActor::BeginPlay()
{
	Super::BeginPlay();

	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &AButtonHazzardActor::OnButtonHit);

	SetButtonState(true);
}

// Called every frame
void AButtonHazzardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlayedErrorSound += DeltaTime;
	if (PlayedErrorSound >= .2f)
	{
		PlayedErrorSound = .2f;
	}
}

void AButtonHazzardActor::OnButtonHit(FHazardTriggerEventData const& Data)
{
	if (!bIsActive)
	{
		if (ButtonFailSound and PlayedErrorSound >= .2f)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ButtonFailSound, Data.HitResult.ImpactPoint);
			PlayedErrorSound = 0.F;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Button fail sound effect not set"));
		}
		return;
	}

	SetButtonState(false);

	/*auto const playerId{ BumperCar->GetUserIndex() };
	UE_LOG(LogTemp, Warning, TEXT("Player: %d activated button!"), playerId);*/

	// Select a random hazard to spawn
	TSubclassOf<AActor> SelectedHazard { };
	if (SpawnAbleHazards.Num() > 0)
	{
		int32 const RandomIndex{ FMath::RandRange(0, SpawnAbleHazards.Num() - 1) };
		SelectedHazard = SpawnAbleHazards[RandomIndex];
	}
	else
	{
		UE_LOG(LogHazardManager, Error, TEXT("No spawnable hazards in the list"))
	}


	if (ButtonActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ButtonActivateSound, Data.HitResult.ImpactPoint);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Button Activate sound effect not set"));
	}

	GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>()->OnButtonHit.Broadcast(SelectedHazard, Data.HitResult.ImpactPoint);
	GetWorldTimerManager().SetTimer(ReactivationTimerHandle, this, &AButtonHazzardActor::ReactivateButton, ReactivationTime, false);
}

void AButtonHazzardActor::SetButtonState(bool bActive)
{
	bIsActive = bActive;
	ButtonMesh->SetMaterial(0, bActive ? IsOnMaterial : IsOffMaterial);
}

void AButtonHazzardActor::ReactivateButton()
{
	SetButtonState(true);
	//UE_LOG(LogTemp, Warning, TEXT("Button reactivated!"));
}

