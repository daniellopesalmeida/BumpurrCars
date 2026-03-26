// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/IceCubeHazardActor.h"
#include "Hazards/HazardManagerWorldSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AIceCubeHazardActor::AIceCubeHazardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardType = EHazardType::Ground;

	IceCubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IceCubeMesh"));
	RootComponent = IceCubeMesh;

	IceCubeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	IceCubeMesh->SetNotifyRigidBodyCollision(true);
	IceCubeMesh->SetCollisionObjectType(ECC_WorldStatic);
	IceCubeMesh->SetCollisionResponseToAllChannels(ECR_Block);

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::Hit;
	HazardTriggerComponent->TargetComponent = IceCubeMesh;
}

void AIceCubeHazardActor::BeginPlay()
{
	Super::BeginPlay();

	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &AIceCubeHazardActor::OnIceCubeHit);
	RotSpeed = FMath::RandRange(MinRotSpeed, MaxRotSpeed);
}

void AIceCubeHazardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorLocalRotation(FRotator(0.f, RotSpeed * DeltaTime, 0.f));
}

void AIceCubeHazardActor::OnIceCubeHit(FHazardTriggerEventData const& Data)
{
	GetWorld()->GetSubsystem<UHazardManagerWorldSubsystem>()->OnIceCubeHit.Broadcast(Data.HitResult.ImpactPoint);

	if (IceBreakEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			IceBreakEffect,
			Data.HitResult.ImpactPoint,
			GetActorRotation()
		);
	}

	if (IceBreakSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, IceBreakSound, Data.HitResult.ImpactPoint);
	}

	Destroy();
}

