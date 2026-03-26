// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScorePopup.generated.h"

/**
 * 
 */
UCLASS()
class BUMPURRCARS_API UScorePopup : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetScoreLoss(int32 NewScore);

	UFUNCTION(BlueprintCallable)
	void SetScoreGain(int32 NewScore);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditDefaultsOnly)
	float LossClearTime{ 1.f };
	UPROPERTY(EditDefaultsOnly)
	float GainClearTime{ 1.f };

	UPROPERTY(EditDefaultsOnly)
	float SlideDuration{ 2.0f };

	UPROPERTY(EditDefaultsOnly)
	float GainEndPosOffset{ 50.f };

	UPROPERTY(EditDefaultsOnly)
	float LossEndPosOffset{ 30.f };

	UPROPERTY(EditDefaultsOnly)
	float MaxGainScale{ 2.f };

	UPROPERTY(EditDefaultsOnly)
	float MinGainScale{ .75f };

	UPROPERTY(EditDefaultsOnly)
	float GainScaleModifer{ 10000.f };

	UPROPERTY(EditDefaultsOnly)
	float MaxLossScale{ 1.75f };

	UPROPERTY(EditDefaultsOnly)
	float MinLossScale{ .75f };

	UPROPERTY(EditDefaultsOnly)
	float LossScaleModifer{ 10000.f };

	FVector2D SlideStartGain{ 0.f, 0.f };
	FVector2D SlideEndGain{ 0.f, 0.f };
	FVector2D SlideStartLoss{ 0.f, 0.f };
	FVector2D SlideEndLoss{ 0.f, 0.f };


private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreGainText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreLossText;

	float SlideElapsedTimeGain{ 0.0f };
	float SlideElapsedTimeLoss{ 0.0f };

	FTimerHandle GainSlideTimer;
	FTimerHandle LossSlideTimer;

	void ClearScoreLoss();
	void ClearScoreGain();

	void UpdateScoreGainSlide();
	void UpdateScoreLossSlide();
};
	

	
	