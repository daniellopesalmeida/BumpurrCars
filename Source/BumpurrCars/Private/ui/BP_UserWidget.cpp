// Fill out your copyright notice in the Description page of Project Settings.


#include "ui/BP_UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Widget.h"
#include <Blueprint/WidgetTree.h>
#include "Core/BumperCarPlayerController.h"
#include <EnhancedInputComponent.h>
#include "EnhancedInputSubsystems.h"

void UBP_UserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TArray<UWidget*> wid;

    FocusableWidgets.Empty();
    this->WidgetTree->GetAllWidgets(wid);

    for (UWidget* widget : wid)
    {
        if (UButton* button = Cast<UButton>(widget))
        {
            FocusableWidgets.Add(button);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("size of ini is: %i"), FocusableWidgets.Num());

    if (FocusableWidgets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("no widgets found"));
    }

    // Set initial focus
    if (FocusableWidgets.IsValidIndex(CurrentFocusIndex))
    {
        if (UButton* InitialFocusButton = Cast<UButton>(FocusableWidgets[CurrentFocusIndex]))
        {
            UE_LOG(LogTemp, Warning, TEXT("focuse set"));
            InitialFocusButton->SetKeyboardFocus();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("no button found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("false focusable widget"));
    }

    bIsFocusable = true;
}

void UBP_UserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (FocusableWidgets.Num() > 0)
    {
        for (auto wid : FocusableWidgets)
            if (wid->HasKeyboardFocus())
                return;

        FocusableWidgets[0]->SetKeyboardFocus();
    }
    //UE_LOG(LogTemp, Warning, TEXT("is ticking"));
}

FReply UBP_UserWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    return FReply::Unhandled();
}

FReply UBP_UserWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return FReply::Unhandled();
}

void UBP_UserWidget::NavigateUp()
{
    UE_LOG(LogTemp, Warning, TEXT("going up"));
    NavigateToWidget(-1);
}

void UBP_UserWidget::NavigateDown()
{
    UE_LOG(LogTemp, Warning, TEXT("going down"));
    NavigateToWidget(1);
}

void UBP_UserWidget::Accept()
{
    if (FocusableWidgets.IsValidIndex(CurrentFocusIndex))
    {
        if (UButton* NewFocusButton = Cast<UButton>(FocusableWidgets[CurrentFocusIndex]))
        {
            UE_LOG(LogTemp, Warning, TEXT("accepting"));

            NewFocusButton->OnClicked.Broadcast();

            APlayerController* controler = GetWorld()->GetFirstPlayerController();

            controler->SetInputMode(FInputModeGameOnly());
        }
    }
}

void UBP_UserWidget::NavigateToWidget(int32 Direction)
{
    if (FocusableWidgets.Num() == 0) 
    {
        UE_LOG(LogTemp, Warning, TEXT("focusable widget size whas 0"));
        return;
    }
        //auto widgets = GetWidgets();

        UE_LOG(LogTemp, Warning, TEXT("size of ini is: %i"), FocusableWidgets.Num());
        // Update focus index
        // the + FucusableWidget is to prevent the devide by zero error
        CurrentFocusIndex = (CurrentFocusIndex + Direction + FocusableWidgets.Num()) % FocusableWidgets.Num();

        // Set focus to the new widget
        if (FocusableWidgets.IsValidIndex(CurrentFocusIndex))
        {
            if (UButton* NewFocusButton = Cast<UButton>(FocusableWidgets[CurrentFocusIndex]))
            {
                NewFocusButton->SetKeyboardFocus();
            }
        }
}
