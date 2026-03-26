// Fill out your copyright notice in the Description page of Project Settings.


#include "Hazards/MeteorHazardActor.h"

#include "Characters/BumperCarPawn.h"
#include "Hazards/HazardTriggerComponent.h"
#include "Components/BoxComponent.h"
#include "Core/BumpurrCarsGamemode.h"
#include "Environment/FloorActor.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AMeteorHazardActor::AMeteorHazardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardType = EHazardType::Other;

	MeteorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeteorMesh"));
	RootComponent = MeteorMesh;

	MeteorMesh->SetSimulatePhysics(true);
	MeteorMesh->SetEnableGravity(false); //  Initially false, when the falling starts set to true
	MeteorMesh->SetNotifyRigidBodyCollision(true);

	//MeteorMesh->SetMassOverrideInKg(NAME_None, 200.f);

	MeteorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeteorMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeteorMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeteorMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	HazardTriggerComponent = CreateDefaultSubobject<UHazardTriggerComponent>(TEXT("HazardTriggerComponent"));
	HazardTriggerComponent->DetectionType = EHazardTriggerDetectionType::BeginOverlap;
	HazardTriggerComponent->TargetComponent = MeteorMesh;
}

void AMeteorHazardActor::BeginPlay()
{
	Super::BeginPlay();

	HazardTriggerComponent->OnHazardTriggered.AddDynamic(this, &AMeteorHazardActor::OnMeteorOverlap);

	if (MeteorMesh && MeteorMesh->GetStaticMesh())
	{
		FVector const StartLocation{ GetActorLocation() };
		FVector const EndLocation{ StartLocation.X, StartLocation.Y, 1.f };

		FVector const MeteorSize{ MeteorMesh->Bounds.BoxExtent }; // Half-size of the mesh
		//float const ImpactRadius{ static_cast<float>(FMath::Max(MeteorSize.X, MeteorSize.Y)) }; // Choose largest horizontal axis

		if (MeteorIndicatorEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				MeteorIndicatorEffect,
				EndLocation
			);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No meteor indicator effect set"));
		}


		//DrawDebugCircle(GetWorld(), EndLocation, ImpactRadius, 25, FColor::Red, false, 5.0f, 0, 20, FVector(1, 0, 0), FVector(0, 1, 0));
		//DrawDebugCircle(GetWorld(), EndLocation, LaunchRadius, 25, FColor::Green, false, 5.0f, 0, 20, FVector(1, 0, 0), FVector(0, 1, 0));
	}

	MeteorMesh->OnComponentHit.AddDynamic(this, &AMeteorHazardActor::OnMeteorHit);

	if (MeteorFallSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MeteorFallSound, GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No meteor fall sound set"));
	}

	GetWorld()->GetTimerManager().SetTimer(StartFallingTimerHandle, [this]()
		{
			// mps -> cmps
			MeteorMesh->AddImpulse(GetActorForwardVector() * InitialFallSpeed * 100);

			MeteorMesh->SetEnableGravity(true);

			if (MeteorFallingEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					MeteorFallingEffect,
					MeteorMesh,
					NAME_None,
					FVector::ZeroVector,
					MeteorMesh->GetComponentRotation(),
					EAttachLocation::KeepRelativeOffset,
					true
				);
				
					
			}

		}, FallDelay, false);
}

void AMeteorHazardActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DestroyTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
	}
	if (StartFallingTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(StartFallingTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AMeteorHazardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMeteorHazardActor::OnMeteorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHit)
	{
		return;
	}

	if (Cast<AFloorActor>(OtherActor))
	{
		bHit = true;

		MeteorMesh->SetSimulatePhysics(false);
		MeteorMesh->SetAllPhysicsLinearVelocity({});
		MeteorMesh->SetAllPhysicsAngularVelocityInDegrees({});

		if (ImpactShake)
		{
			UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(ImpactShake);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No meteor impact cam shake effect set"));
		}

		if (ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				ImpactEffect,
				FVector{ GetActorLocation().X, GetActorLocation().Y, 1 }
			);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No meteor impact effect set"));
		}


		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No meteor impact sound set"));
		}

		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, [this]()
			{
				Destroy();
			}, MeteorDestroyDelay, false);

		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				auto const GameMode{ GetWorld()->GetAuthGameMode() };
				if (auto const& BumpGameMode{ Cast<ABumpurrCarsGamemode>(GameMode) })
				{
					FVector const MeteorLocation{ GetActorLocation() };
					FVector const MeteorLocationGround{ MeteorLocation.X, MeteorLocation.Y, 0.f };

					auto const AllCars{ BumpGameMode->GetAllCarsInRadius(MeteorLocationGround, LaunchRadius) };
					for (auto const& Car : AllCars)
					{
						FVector const DirectionAwayFromMeteor{ (Car->GetActorLocation() - MeteorLocationGround) };
						FVector const DistanceFactor{ 1.f - DirectionAwayFromMeteor.Size() / LaunchRadius};

						//DrawDebugLine(GetWorld(), MeteorLocationGround + FVector{0,0,5}, (MeteorLocationGround + DirectionAwayFromMeteor * LaunchForce * 100 * DistanceFactor) + FVector{ 0,0,5 }, FColor::Red, false, 5.f, 0, 5.0f);
						if (Car->HasShield())
						{
							//halve the launch force
							Car->Launch(DirectionAwayFromMeteor * (LaunchForce*0.5f) * 100 * DistanceFactor);
						}
						else
						{
							Car->Launch(DirectionAwayFromMeteor * LaunchForce * 100 * DistanceFactor);
						}

						APlayerController* PlayerController{ Cast<APlayerController>(Car->GetController()) };
						if (PlayerController && PushForceFeedbackEffect)
						{
							PlayerController->ClientPlayForceFeedback(PushForceFeedbackEffect);
						}
					}
				}
			});
	}
}

void AMeteorHazardActor::OnMeteorOverlap(FHazardTriggerEventData const& TriggerData)
{
	TriggerData.InstigatorActor->OnPlayerHitByMeteor.Broadcast();
	auto Car{ Cast<ABumperCarPawn>(TriggerData.InstigatorActor) };

	APlayerController* PlayerController{ Cast<APlayerController>(Car->GetController()) };
	if (PlayerController && ImpactForceFeedbackEffect)
	{
		PlayerController->ClientPlayForceFeedback(ImpactForceFeedbackEffect);
	}
}
