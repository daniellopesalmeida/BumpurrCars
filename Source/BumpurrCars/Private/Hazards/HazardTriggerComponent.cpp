// Fill out your copyright notice in the Description page of Project Settings.

#include "Hazards/HazardTriggerComponent.h"

UHazardTriggerComponent::UHazardTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UHazardTriggerComponent::GetApproxOverlapContactPoint(UPrimitiveComponent* ComponentA, UPrimitiveComponent* ComponentB)
{
    if (!ComponentA || !ComponentB)
    {
        return FVector::ZeroVector;
    }

    FVector RefPoint{ ComponentB->GetComponentLocation() };
    FVector ClosestPoint;

    if (ComponentA->GetClosestPointOnCollision(RefPoint, ClosestPoint) > 0.F)
    {
        return ClosestPoint;
    }

    RefPoint = ComponentA->GetComponentLocation();
    if (ComponentB->GetClosestPointOnCollision(RefPoint, ClosestPoint) > 0.f)
    {
        return ClosestPoint;
    }

    return (ComponentA->GetComponentLocation() + ComponentB->GetComponentLocation()) * 0.5f;
}
void UHazardTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

    if (!TargetComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("HazardHitComponent on %s has no TargetComponent assigned."), *GetOwner()->GetName());
        return;
    }

	switch (DetectionType)
    {
    case EHazardTriggerDetectionType::BeginOverlap:
        TargetComponent->OnComponentBeginOverlap.AddDynamic(this, &UHazardTriggerComponent::HandleBeginOverlap);

        // Enable hit overlap notifications if they aren't already on
        TargetComponent->SetGenerateOverlapEvents(true);
        break;

    case EHazardTriggerDetectionType::EndOverlap:
        TargetComponent->OnComponentEndOverlap.AddDynamic(this, &UHazardTriggerComponent::HandleEndOverlap);

        // Enable hit overlap notifications if they aren't already on
        TargetComponent->SetGenerateOverlapEvents(true);
        break;

    case EHazardTriggerDetectionType::Hit:
        TargetComponent->OnComponentHit.AddDynamic(this, &UHazardTriggerComponent::HandleHit);

		// Enable hit notifications if they aren't already on
        TargetComponent->SetNotifyRigidBodyCollision(true);
        break;

    default:
        break;
    }
}


// Called every frame
void UHazardTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHazardTriggerComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (auto const Car{ Cast<ABumperCarPawn>(OtherActor) })
	{
        FHitResult hit;
        hit.ImpactPoint = GetApproxOverlapContactPoint(OverlappedComponent, OtherComp);
		OnHazardTriggered.Broadcast({ Car, EHazardTriggerDetectionType::BeginOverlap, {}, hit });
	}
}

void UHazardTriggerComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (auto const Car{ Cast<ABumperCarPawn>(OtherActor) })
    {
        OnHazardTriggered.Broadcast({ Car, EHazardTriggerDetectionType::EndOverlap });
    }
}

void UHazardTriggerComponent::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (auto const Car{ Cast<ABumperCarPawn>(OtherActor) })
    {
		OnHazardTriggered.Broadcast({ Car, EHazardTriggerDetectionType::Hit, NormalImpulse, Hit });
    }
}

