// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BumperCarSpawnPoint.h"
#include "Components/ArrowComponent.h"

ABumperCarSpawnPoint::ABumperCarSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	//// Initialize ArrowComponent only in the editor
	//ArrowComponentEditorOnly = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponentEditorOnly"));
	//ArrowComponentEditorOnly->SetupAttachment(RootComponent);
	//ArrowComponentEditorOnly->SetRelativeRotation(GetActorRotation());
	//ArrowComponentEditorOnly->bIsEditorOnly = true;
}

void ABumperCarSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
}

void ABumperCarSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//#if WITH_EDITOR
//void ABumperCarSpawnPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeProperty(PropertyChangedEvent);
//
//	if (ArrowComponentEditorOnly)
//	{
//		ArrowComponentEditorOnly->SetRelativeRotation(GetActorRotation());
//	}
//}
//#endif


