// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BumpurrCarsGameInstance.h"

#include "Core/BumpurrCarsGamemode.h"
#include "Kismet/GameplayStatics.h"

#include "EnhancedInputSubsystems.h"      // For UEnhancedInputLocalPlayerSubsystem
#include "InputCoreTypes.h"               // For FInputDeviceId

DEFINE_LOG_CATEGORY(LogGameInstance);

void UBumpurrCarsGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogGameInstance, Warning, TEXT("Game instance initialized"));

    FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UBumpurrCarsGameInstance::OnLevelLoaded);
}

void UBumpurrCarsGameInstance::ReloadLevel()
{
    UE_LOG(LogGameInstance, Warning, TEXT("Reloading level %s"), *GetWorld()->GetName());

    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

//bool UBumpurrCarsGameInstance::IsControllerConnected(int32 playerID)
//{
//    IPlatformInputDeviceMapper& DeviceMapper{ IPlatformInputDeviceMapper::Get() };
//    auto LocalPlayer{ GetLocalPlayerByIndex(playerID) };
//    TArray<FInputDeviceId> InputDevices;
//    DeviceMapper.GetAllInputDevicesForUser(LocalPlayer->GetPlatformUserId(), InputDevices);
//
//	for (const FInputDeviceId& DeviceId : InputDevices)
//    {
//        if (DeviceMapper.GetInputDeviceConnectionState(DeviceId) != EInputDeviceConnectionState::Connected)
//        {
//            return false;
//        }
//    }
//
//    return true;
//}

void UBumpurrCarsGameInstance::OnLevelLoaded(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (not World)
	{
        return;
	}
    UE_LOG(LogGameInstance, Warning, TEXT("Level Loaded %s"), *GetWorld()->GetName());
}

// COMMENTED CODE WAS FOR TESTING

//void UBumpurrCarsGameInstance::WaitForLevelLoad()
//{
//    GetWorld()->GetTimerManager().SetTimer(LevelWaitTimerHandle, this, &UBumpurrCarsGameInstance::TryStartGame, 0.05f, true);
//}
//
//void UBumpurrCarsGameInstance::TryStartGame()
//{
//    AGameModeBase* GameMode = { GetWorld()->GetAuthGameMode() };
//
//    if (UWorld* World = { GetWorld() }; World && GameMode && GameMode->IsActorInitialized())
//    {
//        UE_LOG(LogGameInstance, Warning, TEXT("GameMode ready: %s"), *GameMode->GetClass()->GetName());
//
//        ABumpurrCarsGamemode* BumpurrGM = Cast<ABumpurrCarsGamemode>(GameMode);
//        if (BumpurrGM)
//        {
//            bool bAllLvlsLoaded{ true };
//
//            // Check all streaming levels to ensure they are fully loaded
//            for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
//            {
//	            if (StreamingLevel->ShouldBeAlwaysLoaded())
//	            {
//                    UE_LOG(LogGameInstance, Warning, TEXT("LVL should always be loaded"));
//                }
//                if (StreamingLevel)
//                {
//	                if (!StreamingLevel->IsLevelLoaded())
//	                {
//	                	UE_LOG(LogGameInstance, Warning, TEXT("A streaming level is NOT loaded! %s"), *StreamingLevel->GetName());
//                        bAllLvlsLoaded = false;
//                        break;
//	                }
//	     
//                    UE_LOG(LogGameInstance, Warning, TEXT("A streaming level is loaded! %s"), *StreamingLevel->GetName());
//                }
//            }
//            
//
//            if (bAllLvlsLoaded)
//            {
//			    // Done, clear the timer
//				GetWorld()->GetTimerManager().ClearTimer(LevelWaitTimerHandle);
//            }
//
//        }
//    }
//    else
//    {
//        UE_LOG(LogGameInstance, Warning, TEXT("Waiting for GameMode to be ready..."));
//    }
//}
