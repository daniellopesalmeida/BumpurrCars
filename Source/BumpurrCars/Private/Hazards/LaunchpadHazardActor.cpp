// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/LaunchpadHazardActor.h"
#include "Hazards/HazardManagerWorldSubsystem.h"
#include "Characters/BumperCarPawn.h"
#include <NiagaraFunctionLibrary.h>

#include "Kismet/GameplayStatics.h"

ALaunchpadHazardActor::ALaunchpadHazardActor()
{
	PrimaryActorTick.bCanEverTick = false;

	HazardType = EHazardType::Ground;

	LaunchpadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaunchpadMesh"));
	RootComponent = LaunchpadMesh;

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::Hit;
	HazardTriggerComponent->TargetComponent = LaunchpadMesh;
}

void ALaunchpadHazardActor::BeginPlay()
{
	Super::BeginPlay();

	LaunchpadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LaunchpadMesh->SetNotifyRigidBodyCollision(true);
	LaunchpadMesh->SetCollisionObjectType(ECC_WorldStatic);
	LaunchpadMesh->SetCollisionResponseToAllChannels(ECR_Block);

	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &ALaunchpadHazardActor::OnLaunchpadHit);
}

void ALaunchpadHazardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALaunchpadHazardActor::OnLaunchpadHit(FHazardTriggerEventData const& Data)
{
	GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>()->OnLaunchpadHit.Broadcast(Data.HitResult.ImpactPoint);

	auto const ReverseHit{ FHitResult::GetReversedHit(Data.HitResult) };
	auto const ImpulseDir{ ReverseHit.ImpactNormal };

	//DrawDebugDirectionalArrow(
	//	GetWorld(),
	//	Data.HitResult.ImpactPoint,
	//	Data.HitResult.ImpactPoint + ImpulseDir * 200.f,
	//	100.f,
	//	FColor::Green,
	//	false,
	//	2.f
	//);

	//UE_LOG(LogTemp, Warning, TEXT("The normal: %f, %f, %f"), ImpulseDir.X, ImpulseDir.Y, ImpulseDir.Z);

	Data.InstigatorActor->Launch(ImpulseDir * Data.NormalImpulse.Size() * LaunchBoostModifier, bApplyAtLocation, Data.HitResult.ImpactPoint);
	auto Car{ Cast<ABumperCarPawn>(Data.InstigatorActor) };
	APlayerController* PlayerController{ Cast<APlayerController>(Car->GetController()) };
	if (PlayerController && HitForceFeedbackEffect)
	{
		PlayerController->ClientPlayForceFeedback(HitForceFeedbackEffect);
	}

	if (LaunchBumpPadHitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LaunchBumpPadHitEffect,
			Data.HitResult.ImpactPoint,
			GetActorRotation()
		);
	}

	if (LaunchPadHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), LaunchPadHitSound, Data.HitResult.ImpactPoint);
	}
}

