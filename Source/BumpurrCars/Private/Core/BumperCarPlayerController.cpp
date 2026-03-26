#include "Core/BumperCarPlayerController.h"
#include "ui/BP_UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Characters/BumperCarPawn.h"

#include <Components/WidgetComponent.h>
#include "Blueprint/UserWidget.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include <Kismet/GameplayStatics.h>

#include "GameFramework/PlayerController.h"


DEFINE_LOG_CATEGORY(LogBumperCarController);

void ABumperCarPlayerController::HandleBoostCar()
{
	//UE_LOG(LogBumperCarController, Warning, TEXT("Boosting car!"));
	if (APawn * ControlledPawn{ GetPawn() })
	{
		if (ABumperCarPawn * BumperCar{ Cast<ABumperCarPawn>(ControlledPawn) })
		{
			BumperCar->Boost();
		}
	}
}

void ABumperCarPlayerController::HandlePauseGame()
{
	//UE_LOG(LogBumperCarController, Warning, TEXT("Pausing Game!"));
	checkf(WBP_PauseScreen, TEXT("the Pause Screen widget has not been set, st it in the bp"));

	UGameplayStatics::SetGamePaused(GetWorld(), true);
	
	auto pauseMenuInstance = CreateWidget<UUserWidget>(this, WBP_PauseScreen);
	pauseMenuInstance->AddToViewport();

	SetInputModeToUI();

	UE_LOG(LogBumperCarController, Warning, TEXT("the game has been paused"));
}

void ABumperCarPlayerController::HandleAccelerateCar(const FInputActionValue& Value)
{
	//UE_LOG(LogBumperCarController, Warning, TEXT("Accelerating car! Value: %f"), Value.Get<float>());
	float const Acceleration{ Value.Get<float>() };

	if (APawn * ControlledPawn{ GetPawn() })
	{
		if (ABumperCarPawn * BumperCar{ Cast<ABumperCarPawn>(ControlledPawn) })
		{
			BumperCar->AccelerateCar(Acceleration);
		}
	}
}

void ABumperCarPlayerController::HandleReverseCar(const FInputActionValue& Value)
{
	//UE_LOG(LogBumperCarController, Warning, TEXT("Reversing car! Value: %f"), Value.Get<float>());
	float const Reverse{ Value.Get<float>() };

	if (APawn * ControlledPawn{ GetPawn() })
	{
		if (ABumperCarPawn * BumperCar{ Cast<ABumperCarPawn>(ControlledPawn) })
		{
			BumperCar->ReverseCar(Reverse);
			//UE_LOG(LogTemp, Log, TEXT("Brake action triggered"));
		}
	}
}

void ABumperCarPlayerController::HandleTurnCar(const FInputActionValue& Value)
{
	auto const& TurnValue = Value.Get<FVector2D>();

	if (APawn * ControlledPawn{ GetPawn() })
	{
		if (ABumperCarPawn * BumperCar{ Cast<ABumperCarPawn>(ControlledPawn) })
		{
			//UE_LOG(LogBumperCarController, Warning, TEXT("Turning car! Value: X: %f, Y: %f"), TurnValue.X, TurnValue.Y);
			//UE_LOG(LogBumperCarController, Warning, TEXT("Input magnitude: %f"), TurnValue.Size());


			//auto const& Normal{ TurnValue.GetSafeNormal() };
			//UE_LOG(LogBumperCarController, Warning, TEXT("Turning car (NORMAL)! Value: X: %f, Y: %f"), Normal.X, Normal.Y);

			BumperCar->TurnCar(TurnValue);
		}
	}
}

void ABumperCarPlayerController::HandleUINavigation(const FInputActionValue& Value)
{
	auto const& UIValue = Value.Get<FVector2D>();

	//Value.

	UE_LOG(LogBumperCarController, Warning, TEXT("input value: X %f, Y %f"), UIValue.X, UIValue.Y);

	if (UIValue.Y > 0)
	{
		HandleUIUp();
	}

	if (UIValue.Y < 0)
	{
		HandleUIDown();
	}
}

void ABumperCarPlayerController::HandleUIDown()
{
	UE_LOG(LogBumperCarController, Warning, TEXT("Getting triggerd"));

	UWorld* World = GetWorld();
	if (!World) return;

	// Array to hold found widgets
	TArray<UUserWidget*> FoundWidgets;

	// Find all widgets of the specified class
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, UBP_UserWidget::StaticClass(), true);

	UE_LOG(LogBumperCarController, Warning, TEXT("all widgets size: %i"), FoundWidgets.Num());

	UBP_UserWidget* widget = (UBP_UserWidget*)FoundWidgets[0];

	if (widget)
	{
		UE_LOG(LogBumperCarController, Warning, TEXT("navegating down"));
		widget->NavigateDown();
	}
	else
	{
		UE_LOG(LogBumperCarController, Warning, TEXT("no widget found"));
	}
}

void ABumperCarPlayerController::HandleUIUp()
{
	UE_LOG(LogBumperCarController, Warning, TEXT("Getting triggerd"));

	UWorld* World = GetWorld();
	if (!World) return;

	// Array to hold found widgets
	TArray<UUserWidget*> FoundWidgets;

	// Find all widgets of the specified class
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, UBP_UserWidget::StaticClass(), true);

	UE_LOG(LogBumperCarController, Warning, TEXT("all widgets size: %i"), FoundWidgets.Num());

	UBP_UserWidget* widget = (UBP_UserWidget*)FoundWidgets[0];

	if (widget)
	{
		UE_LOG(LogBumperCarController, Warning, TEXT("navegating up"));
		widget->NavigateUp();
	}
	else
	{
		UE_LOG(LogBumperCarController, Warning, TEXT("no widget found"));
	}
}

void ABumperCarPlayerController::HandleUIBack()
{
	UE_LOG(LogBumperCarController, Warning, TEXT("needs to be implemented"));
}

void ABumperCarPlayerController::HandleUIAccept()
{
	UE_LOG(LogBumperCarController, Warning, TEXT("Getting triggerd"));

	UWorld* World = GetWorld();
	if (!World) return;

	// Array to hold found widgets
	TArray<UUserWidget*> FoundWidgets;

	// Find all widgets of the specified class
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, UBP_UserWidget::StaticClass(), true);

	UE_LOG(LogBumperCarController, Warning, TEXT("all widgets size: %i"), FoundWidgets.Num());

	UBP_UserWidget* widget = (UBP_UserWidget*)FoundWidgets[0];

	if (widget)
	{
		UE_LOG(LogBumperCarController, Warning, TEXT("accepting button"));
		widget->Accept();
	}
}

void ABumperCarPlayerController::SetupInput()
{
	if (ULocalPlayer const* const LocalPlayer{ GetLocalPlayer() })
	{
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem{ LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() })
		{
			//TODO map UI by default when we actually have a UI
			checkf(GameplayMappingContext, TEXT("GameplayMappingContext is nullptr — this must be set in BP!"));
			//checkf(UIMappingContext, TEXT("UIMappingContext is nullptr — this must be set in BP!"));

			Subsystem->AddMappingContext(GameplayMappingContext, 0);
			//Subsystem->AddMappingContext(UIMappingContext, 1);

		}
		else
		{
			UE_LOG(LogBumperCarController, Error, TEXT("No enhanced input sys in SetupInput of PlayerController!"));
		}
	}
	else
	{
		UE_LOG(LogBumperCarController, Error, TEXT("No local player in SetupInput of PlayerController!"));
	}
}

void ABumperCarPlayerController::SetInputModeToGameplay()
{
	if (ULocalPlayer const* const LocalPlayer{ GetLocalPlayer() })
	{
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem{ LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() })
		{
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);

			bShowMouseCursor = false;
		}
	}
}

void ABumperCarPlayerController::SetInputModeToUI()
{
	if (ULocalPlayer const* const LocalPlayer{ GetLocalPlayer() })
	{
		if (UEnhancedInputLocalPlayerSubsystem* const Subsystem{ LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() })
		{
			TArray<UUserWidget*> foundWidget;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), foundWidget,
				UBP_UserWidget::StaticClass(), false);

			FInputModeUIOnly InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			if (foundWidget.Num() > 0)
			{
				UE_LOG(LogBumperCarController, Error, TEXT("found widgets"));

				auto widget = Cast<UBP_UserWidget>(foundWidget[0]);
				if (widget)
				{
					UE_LOG(LogBumperCarController, Error, TEXT("casted widget"));
					widget->SetKeyboardFocus();
					InputMode.SetWidgetToFocus(widget->TakeWidget());
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

					widget->SetKeyboardFocus();
					//FSlateApplication::Get().SetUserFocus(0, widget->TakeWidget(), EFocusCause::SetDirectly);
				}
			}

			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void ABumperCarPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ABumperCarPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent * EnhancedInputComp{ Cast<UEnhancedInputComponent>(InputComponent) })
	{
		checkf(BoostAction, TEXT("Boost action must be set!"));
		EnhancedInputComp->BindAction(BoostAction, ETriggerEvent::Started, this, &ABumperCarPlayerController::HandleBoostCar);

		checkf(PauseAction, TEXT("Pause action must be set!"));
		EnhancedInputComp->BindAction(PauseAction, ETriggerEvent::Started, this, &ABumperCarPlayerController::HandlePauseGame);

		checkf(TurnAction, TEXT("Accelerate action must be set!"));
		EnhancedInputComp->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ABumperCarPlayerController::HandleTurnCar);
		EnhancedInputComp->BindAction(TurnAction, ETriggerEvent::Completed, this, &ABumperCarPlayerController::HandleTurnCar);

		checkf(AccelerateAction, TEXT("Accelerate action must be set!"));
		EnhancedInputComp->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &ABumperCarPlayerController::HandleAccelerateCar);
		EnhancedInputComp->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &ABumperCarPlayerController::HandleAccelerateCar);

		checkf(ReverseAction, TEXT("Reverse action must be set!"));
		EnhancedInputComp->BindAction(ReverseAction, ETriggerEvent::Triggered, this, &ABumperCarPlayerController::HandleReverseCar);
		EnhancedInputComp->BindAction(ReverseAction, ETriggerEvent::Completed, this, &ABumperCarPlayerController::HandleReverseCar);

		/*checkf(UIBackAction, TEXT("UI back Action action must be set!"));
		EnhancedInputComp->BindAction(UIBackAction, ETriggerEvent::Started, this, &ABumperCarPlayerController::HandleUIBack);

		checkf(UIAcceptAction, TEXT("UI accept Action action must be set!"));
		EnhancedInputComp->BindAction(UIAcceptAction, ETriggerEvent::Started, this, &ABumperCarPlayerController::HandleUIAccept);

		checkf(UINavigationAction, TEXT("UI Navigation Action action must be set!"));
		EnhancedInputComp->BindAction(UINavigationAction, ETriggerEvent::Triggered, this, &ABumperCarPlayerController::HandleUINavigation);*/
	}
}
