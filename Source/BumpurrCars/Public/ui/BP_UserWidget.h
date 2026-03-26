// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BP_UserWidget.generated.h"

/**
 * 
 */
UCLASS()
class BUMPURRCARS_API UBP_UserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    // Called when the widget is added to the viewport
    virtual void NativeConstruct() override;

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Handle navigation using controller input
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
    void NavigateUp();
    void NavigateDown();
    void Accept();

    

private:
    // Helper function to set focus on a widget
    void NavigateToWidget(int32 Direction);

    // List of UI elements that can gain focus
     TArray<UWidget*> FocusableWidgets;

    // Index of the currently focused widget
    int32 CurrentFocusIndex;
};
