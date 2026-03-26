// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/SpikesHazardActor.h"

#include "NiagaraFunctionLibrary.h"
#include "Hazards/HazardManagerWorldSubsystem.h"
#include "Characters/BumperCarPawn.h"

ASpikesHazardActor::ASpikesHazardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardType = EHazardType::Ground;

	SpikesMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikesMesh"));
	RootComponent = SpikesMesh;

	SpikesMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	SpikesMesh->SetNotifyRigidBodyCollision(true);
	SpikesMesh->SetCollisionObjectType(ECC_WorldDynamic);
	SpikesMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	SpikesMesh->SetGenerateOverlapEvents(true);

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::BeginOverlap;
	HazardTriggerComponent->TargetComponent = SpikesMesh;
}

// Called when the game starts or when spawned
void ASpikesHazardActor::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &ASpikesHazardActor::OnSpikesHit);
	GetWorldTimerManager().SetTimer(StayEmergedTimerHandle, this, &ASpikesHazardActor::StartRetract, StayEmergedDuration);

}

// Called every frame
void ASpikesHazardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle the movement smoothly
	if (bIsEmerging)
	{
		// Get elapsed time for emergence
		float Elapsed = GetWorldTimerManager().GetTimerElapsed(EmergeTimerHandle);
		float Alpha = FMath::Clamp(Elapsed / EmergeDuration, 0.f, 1.f);

		// Smoothly move the spikes upward
		FVector NewLocation = FMath::Lerp(StartLocation - FVector(0.f, 0.f, SpikeHeight), StartLocation, Alpha);
		SetActorLocation(NewLocation);

		// Check if emerge is complete
		if (Alpha >= 1.f)
		{
			bIsEmerging = false;
			GetWorldTimerManager().SetTimer(StayEmergedTimerHandle, this, &ASpikesHazardActor::StartRetract, StayEmergedDuration);
		}
	}
	else if (bIsRetracting)
	{
		// Get elapsed time for retraction
		float Elapsed = GetWorldTimerManager().GetTimerElapsed(EmergeTimerHandle);
		float Alpha = FMath::Clamp(Elapsed / RetractDuration, 0.f, 1.f);

		// Smoothly move the spikes downward
		FVector NewLocation = FMath::Lerp(StartLocation, StartLocation - FVector(0.f, 0.f, SpikeHeight), Alpha);
		SetActorLocation(NewLocation);

		// Check if retraction is complete
		if (Alpha >= 1.f)
		{
			bIsRetracting = false;
			GetWorldTimerManager().SetTimer(StayRetractedTimerHandle, this, &ASpikesHazardActor::StartEmerge, StayRetractedDuration);
		}
	}

}

void ASpikesHazardActor::OnSpikesHit(FHazardTriggerEventData const& Data)
{
	ABumperCarPawn const* BumperCar{ Cast<ABumperCarPawn>(Data.InstigatorActor) };

	//int32 const CarId{ BumperCar->GetUserIndex() };
	//UE_LOG(LogTemp, Warning, TEXT("Bumper car '%s' (ID: %d) hit the spikes!"),
	//	*BumperCar->GetName(), CarId);

	BumperCar->OnPlayerHitBySpikes.Broadcast(SpeedReducePercentage);
	
		if (SpikeHitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SpikeHitEffect,
			Data.HitResult.ImpactPoint,
			GetActorRotation()
		);
	}
}

void ASpikesHazardActor::StartEmerge()
{
	UE_LOG(LogTemp, Warning, TEXT("Spikes emerging..."));
	bIsEmerging = true;

	GetWorldTimerManager().SetTimer(EmergeTimerHandle, this, &ASpikesHazardActor::StopEmerge, EmergeDuration);
}

void ASpikesHazardActor::StopEmerge()
{
	UE_LOG(LogTemp, Warning, TEXT("Spikes done emerging."));
	bIsEmerging = false;

	GetWorldTimerManager().SetTimer(StayRetractedTimerHandle, this, &ASpikesHazardActor::StartRetract, StayEmergedDuration);
}

void ASpikesHazardActor::StartRetract()
{
	UE_LOG(LogTemp, Warning, TEXT("Spikes retracting..."));
	bIsRetracting = true;
	GetWorldTimerManager().SetTimer(EmergeTimerHandle, this, &ASpikesHazardActor::StopRetract, RetractDuration);

}

void ASpikesHazardActor::StopRetract()
{
	UE_LOG(LogTemp, Warning, TEXT("Spikes done retracting."));
	bIsRetracting = false;
	GetWorldTimerManager().SetTimer(StayRetractedTimerHandle, this, &ASpikesHazardActor::StartEmerge, StayRetractedDuration);
}





