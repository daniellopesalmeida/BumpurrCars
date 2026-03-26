// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/FloorActor.h"

#include "Hazards/HazardManagerWorldSubsystem.h"

// Sets default values
AFloorActor::AFloorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	RootComponent = FloorMesh;

	FloorMesh->SetMobility(EComponentMobility::Static);
	FloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FloorMesh->SetCollisionObjectType(ECC_WorldStatic);
	FloorMesh->SetCollisionResponseToAllChannels(ECR_Block);
	FloorMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	FloorMesh->SetCollisionResponseToChannel(ECC_EngineTraceChannel1, ECR_Ignore);
	FloorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	FloorMesh->SetNotifyRigidBodyCollision(false);
	FloorMesh->SetSimulatePhysics(false);
}

void AFloorActor::BeginPlay()
{
	Super::BeginPlay();

	bIsIceFloor = false;

	check(FloorMaterial);
	check(FloorPhysicsMaterial);
	check(IceFloorMaterial);
	check(IceFloorPhysicsMaterial);

	FloorMesh->SetMaterial(0, FloorMaterial);
	FloorMesh->SetPhysMaterialOverride(FloorPhysicsMaterial);
	//Error checking due to crash
	if (auto World{ GetWorld() })
	{
		if (auto SubSys{ World->GetSubsystem<UHazardManagerWorldSubsystem>() })
		{
			SubSys->OnIceFloorActivated.AddDynamic(this, &AFloorActor::OnIceFloorActivated);
			SubSys->OnIceFloorDeactivated.AddDynamic(this, &AFloorActor::OnIceFloorDeactivated);
			SubSys->OnEndGameActivated.AddDynamic(this, &AFloorActor::OnEndGameEventActivated);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No SUbsys in Floor actor beginPlay"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No World in Floor actor beginPlay"));
	}
}

void AFloorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFloorActor::OnIceFloorActivated()
{
	FloorMesh->SetMaterial(0, IceFloorMaterial);
	FloorMesh->SetPhysMaterialOverride(IceFloorPhysicsMaterial);

	bIsIceFloor = true;
}

void AFloorActor::OnIceFloorDeactivated()
{
	FloorMesh->SetMaterial(0, FloorMaterial);
	FloorMesh->SetPhysMaterialOverride(FloorPhysicsMaterial);

	bIsIceFloor = false;
}

void AFloorActor::OnEndGameEventActivated()
{
	FloorMaterial = FloorMaterialEndGame;
	if (not bIsIceFloor)
	{
		FloorMesh->SetMaterial(0, FloorMaterial);
	}
}

