// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void UAudioManager::StartLevelMusic()
{
	if (GameMusic and MusicComponent)
	{
		if (MusicComponent->IsPlaying())
		{
			return;
		}

		UE_LOG(LogAudio, Warning, TEXT("Playing level music"));

		MusicComponent->SetSound(GameMusic);
		MusicComponent->Play();
	}
	else
	{
		UE_LOG(LogAudio, Error, TEXT("Game music not found"));
	}
}

void UAudioManager::StopLevelMusic()
{
	UE_LOG(LogAudio, Warning, TEXT("Stopping level music"));

	MusicComponent->Stop();
}

bool UAudioManager::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only trigger initialization code once (the blueprint version)
	return Super::ShouldCreateSubsystem(Outer) && GetClass()->IsInBlueprint();
}

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogAudio, Warning, TEXT("Audio manager initialized"));

	if (!MusicComponent)
	{
		MusicComponent = NewObject<UAudioComponent>(this);
		MusicComponent->AttachToComponent(GetWorld()->GetWorldSettings()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MusicComponent->bAutoActivate = false;
		MusicComponent->RegisterComponentWithWorld(GetWorld());
		ensure(GetWorld());
		//MusicComponent->Activate();
	}

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UAudioManager::HandleMapLoad);
}

void UAudioManager::Deinitialize()
{
	Super::Deinitialize();

	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
}

void UAudioManager::HandleMapLoad(UWorld* LoadedWorld, UWorld::InitializationValues init)
{
	UE_LOG(LogAudio, Error, TEXT("Mapload called"));
	if (MusicComponent && MusicComponent->GetWorld() != LoadedWorld)
	{
		// Unregister and re-register the component to the new world
		MusicComponent->UnregisterComponent();
		MusicComponent->AttachToComponent(GetWorld()->GetWorldSettings()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MusicComponent->bAutoActivate = false;
		MusicComponent->RegisterComponentWithWorld(LoadedWorld);
	//	MusicComponent->Activate();

		UE_LOG(LogTemp, Log, TEXT("Re-registered music component with new world"));
	}
}
	