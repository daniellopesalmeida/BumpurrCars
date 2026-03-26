// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/PinballBumppadActor.h"
#include "Hazards/HazardManagerWorldSubsystem.h"
#include "Characters/BumperCarPawn.h"
#include "NiagaraFunctionLibrary.h"
#include <Kismet/GameplayStatics.h>


// Sets default values
APinballBumppadActor::APinballBumppadActor()
{
	PrimaryActorTick.bCanEverTick = false;

	HazardType = EHazardType::Ground;

	PinballBumpPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PinballBumpPadMesh"));
	RootComponent = PinballBumpPadMesh;

	PinballBumpPadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PinballBumpPadMesh->SetNotifyRigidBodyCollision(true);
	PinballBumpPadMesh->SetCollisionObjectType(ECC_WorldDynamic);
	PinballBumpPadMesh->SetCollisionResponseToAllChannels(ECR_Block);

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::Hit;
	HazardTriggerComponent->TargetComponent = PinballBumpPadMesh;
}

// Called when the game starts or when spawned
void APinballBumppadActor::BeginPlay()
{
	Super::BeginPlay();

	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &APinballBumppadActor::OnBumpPadHit);
}

// Called every frame
void APinballBumppadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APinballBumppadActor::OnBumpPadHit(FHazardTriggerEventData const& Data)
{
	GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>()->OnBumpPadHit.Broadcast(Data.HitResult.ImpactPoint);

	UE_LOG(LogTemp, Warning, TEXT("Pinball Bumpad hit!"));

	FVector PadLocation = GetActorLocation();
	FVector HitLocation = Data.HitResult.ImpactPoint;

	//direction from center of pad to hit point
	FVector BaseDirection = (HitLocation - PadLocation).GetSafeNormal();

	//randomness to the direction
	float RandomAngleDegrees = FMath::RandRange(-RandDeg, RandDeg);
	FVector RandomizedDirection = BaseDirection.RotateAngleAxis(RandomAngleDegrees, FVector::UpVector);

	//final impulse
	FVector Impulse = RandomizedDirection * LaunchBoostModifier;

    float EffectiveImpulseScale = FMath::Max(static_cast<float>(Data.NormalImpulse.Size()), 1.0f);
	
	Data.InstigatorActor->Launch(Impulse * EffectiveImpulseScale, true, HitLocation);
	auto Car{ Cast<ABumperCarPawn>(Data.InstigatorActor) };
	APlayerController* PlayerController{ Cast<APlayerController>(Car->GetController()) };
	if (PlayerController && HitForceFeedbackEffect)
	{
		PlayerController->ClientPlayForceFeedback(HitForceFeedbackEffect);
	}

	if (PinballBumpPadHitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			PinballBumpPadHitEffect,
			Data.HitResult.ImpactPoint + RandomizedDirection * 50.f,
			RandomizedDirection.Rotation()
		);
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

	if (PinballBumpingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), PinballBumpingSound, Data.HitResult.ImpactPoint);
	}
}

