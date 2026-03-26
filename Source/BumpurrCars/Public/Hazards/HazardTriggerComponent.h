// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Characters/BumperCarPawn.h"
#include "HazardTriggerComponent.generated.h"

UENUM(BlueprintType)
enum class EHazardTriggerDetectionType : uint8
{
	None				UMETA(DisplayName = "None"),
	BeginOverlap		UMETA(DisplayName = "BeginOverlap"),
	EndOverlap			UMETA(DisplayName = "EndOverlap"),
	Hit					UMETA(DisplayName = "Hit"),
};

USTRUCT(BlueprintType)
struct FHazardTriggerEventData
{
	GENERATED_BODY()

	// Car that caused the event
	UPROPERTY(BlueprintReadOnly, Category = "Trigger")
	ABumperCarPawn* InstigatorActor{ nullptr };

	// What type of trigger event occurred
	UPROPERTY(BlueprintReadOnly, Category = "Trigger")
	EHazardTriggerDetectionType TriggerType{ EHazardTriggerDetectionType::None };

	// For hit events: the impulse applied
	UPROPERTY(BlueprintReadOnly, Category = "Trigger")
	FVector NormalImpulse{ FVector::ZeroVector };

	// For hit events: the detailed hit result
	UPROPERTY(BlueprintReadOnly, Category = "Trigger")
	FHitResult HitResult{};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardTriggered, FHazardTriggerEventData const&, EventData);


// You should ensure the collision channels and objects are setup properly for the component that's referenced here (TargetComponent)
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUMPURRCARS_API UHazardTriggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHazardTriggerComponent();

	// Which type of trigger event should this component respond to?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	EHazardTriggerDetectionType DetectionType{ EHazardTriggerDetectionType::Hit };

	// The collision component to listen on. For example; E.G UStaticMeshComponent or UBoxComponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	UPrimitiveComponent* TargetComponent;

	// The event to bind responses to when the hazard is triggered.
	UPROPERTY(BlueprintAssignable, Category = "Trigger")
	FOnHazardTriggered OnHazardTriggered;

	static FVector GetApproxOverlapContactPoint(UPrimitiveComponent* ComponentA, UPrimitiveComponent* ComponentB);


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
