// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/SpeedBoosterHazardActor.h"
#include "Hazards/HazardManagerWorldSubsystem.h"
#include "Characters/BumperCarPawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ASpeedBoosterHazardActor::ASpeedBoosterHazardActor()
{
	PrimaryActorTick.bCanEverTick = false;

	HazardType = EHazardType::Ground;

	SpeedBoosterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpeedBoosterMesh"));
	RootComponent = SpeedBoosterMesh;

	SpeedBoosterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	SpeedBoosterMesh->SetNotifyRigidBodyCollision(true);
	SpeedBoosterMesh->SetCollisionObjectType(ECC_WorldDynamic);
	SpeedBoosterMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	SpeedBoosterMesh->SetGenerateOverlapEvents(true);

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::BeginOverlap;
	HazardTriggerComponent->TargetComponent = SpeedBoosterMesh;

}

// Called when the game starts or when spawned
void ASpeedBoosterHazardActor::BeginPlay()
{
	Super::BeginPlay();

	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &ASpeedBoosterHazardActor::OnSpeedBoosterHit);
}

// Called every frame
void ASpeedBoosterHazardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpeedBoosterHazardActor::OnSpeedBoosterHit(FHazardTriggerEventData const& Data)
{
	GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>()->OnSpeedBoosterHit.Broadcast(Data.HitResult.ImpactPoint);

	FVector const VelDir{ Data.InstigatorActor->GetVelocity().GetSafeNormal() };

	float const ImpulseStrength{ (bRandomizeImpulseStrengthOnHit ? FMath::RandRange(MinImpulseStrength * 100, MaxImpulseStrength * 100) : (MinImpulseStrength * 100 + MaxImpulseStrength * 100) / 2) };
	Data.InstigatorActor->Launch(VelDir * ImpulseStrength);
	
	UNiagaraComponent* Trail{ UNiagaraFunctionLibrary::SpawnSystemAttached(
		BumpercarTrailHitEffect,
		Data.InstigatorActor->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		Data.InstigatorActor->GetActorRotation(),
		EAttachLocation::KeepRelativeOffset,
		false  //we dostroy manually later
	) };

	//destroy the trail after x seconds
	if (Trail)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([Trail]()
			{
				Trail->Deactivate();
				Trail->DestroyComponent();
			});

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.15f, false); //change trail duration here manualy (for testing)
	}
	else
	{
		UE_LOG(LogHazardManager, Error, TEXT("failed to create trail effect"));
	}

	if (SpeedBoosterHitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SpeedBoosterHitEffect,
			GetActorLocation() + FVector(0, 0, 50),
			GetActorRotation()
		);
	}

	if (SpeedBoostSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SpeedBoostSound, Data.HitResult.ImpactPoint, 1.f,  ImpulseStrength / MaxImpulseStrength);
	}
}

