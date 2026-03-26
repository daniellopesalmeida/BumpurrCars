// Fill out your copyright notice in the Description page of Project Settings.


#include "ui/ScorePopup.h"

#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"

void UScorePopup::SetScoreLoss(int32 NewScore)
{
	if (ScoreLossText)
	{
		// Only display text if score is not zero
		if (NewScore != 0)
		{
			ScoreLossText->SetText(FText::FromString(FString::Printf(TEXT("-%d"), FMath::Abs(NewScore))));
			ScoreLossText->SetRenderOpacity(1.0f);

			SlideElapsedTimeLoss = 0.0f;

			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ScoreLossText->Slot))
			{
				CanvasSlot->SetPosition(SlideStartLoss);
			}

			float const Scale{ FMath::Min(MinLossScale + (NewScore / LossScaleModifer), MaxLossScale) };
			ScoreGainText->SetRenderScale({ Scale , Scale });

			// Start timer tick for sliding the text
			GetWorld()->GetTimerManager().ClearTimer(LossSlideTimer);
			GetWorld()->GetTimerManager().SetTimer(LossSlideTimer, this, &UScorePopup::UpdateScoreLossSlide, 0.01f, true);
		}
		else
		{
			//GetWorld()->GetTimerManager().ClearTimer(LossSlideTimer);
			//ScoreLossText->SetText(FText::GetEmpty());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Score loss text not found"));
	}
}

void UScorePopup::SetScoreGain(int32 NewScore)
{
	if (!ScoreGainText) return;

	// Only display text if score is not zero
	if (NewScore != 0)
	{
		ScoreGainText->SetText(FText::FromString(FString::Printf(TEXT("+%d"), NewScore)));
		ScoreGainText->SetRenderOpacity(1.0f);

		float const Scale{ FMath::Min(MinGainScale + (NewScore / GainScaleModifer), MaxGainScale) };
		ScoreGainText->SetRenderScale({ Scale , Scale });
		
		// Reset slide
		SlideElapsedTimeGain = 0.0f;

		// Optional: Start from fresh position
		if (UCanvasPanelSlot * CanvasSlot{ Cast<UCanvasPanelSlot>(ScoreGainText->Slot) })
		{
			CanvasSlot->SetPosition(SlideStartGain);
		}

		GetWorld()->GetTimerManager().ClearTimer(GainSlideTimer);
		GetWorld()->GetTimerManager().SetTimer(GainSlideTimer, this, &UScorePopup::UpdateScoreGainSlide, 0.01f, true);
	}
	else
	{
		//GetWorld()->GetTimerManager().ClearTimer(GainSlideTimer);
		//ScoreGainText->SetText(FText::GetEmpty());
	}
}

void UScorePopup::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCanvasPanelSlot * CanvasSlot{ Cast<UCanvasPanelSlot>(ScoreGainText->Slot) })
	{
		SlideStartGain = CanvasSlot->GetPosition();
		SlideEndGain = SlideStartGain;
		SlideEndGain.Y -= GainEndPosOffset;
	}
	if (UCanvasPanelSlot * CanvasSlot{ Cast<UCanvasPanelSlot>(ScoreLossText->Slot) })
	{
		SlideStartLoss = CanvasSlot->GetPosition();
		SlideEndLoss = SlideStartLoss;
		SlideEndLoss.Y -= LossEndPosOffset;
	}
}

void UScorePopup::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Super::NativeDestruct();
}

void UScorePopup::ClearScoreLoss()
{
	if (ScoreLossText)
	{
		ScoreLossText->SetText(FText::GetEmpty());

		// Reset to original position (so next score starts fresh)
		if (UCanvasPanelSlot * CanvasSlot{ Cast<UCanvasPanelSlot>(ScoreLossText->Slot) })
		{
			CanvasSlot->SetPosition(SlideStartLoss);
		}
	}
}

void UScorePopup::ClearScoreGain()
{
	if (ScoreGainText)
	{
		ScoreGainText->SetText(FText::GetEmpty());
		// Reset to original position (so next score starts fresh)
		if (UCanvasPanelSlot * CanvasSlot{ Cast<UCanvasPanelSlot>(ScoreGainText->Slot) })
		{
			CanvasSlot->SetPosition(SlideStartGain);
		}
	}
}

void UScorePopup::UpdateScoreGainSlide()
{
	SlideElapsedTimeGain += 0.01f;
	float Alpha{ SlideElapsedTimeGain / SlideDuration };

	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	if (UCanvasPanelSlot * CanvasSlot{ Cast<UCanvasPanelSlot>(ScoreGainText->Slot) })
	{
		FVector2D const NewPos{ FMath::Lerp(SlideStartGain, SlideEndGain, Alpha) };
		CanvasSlot->SetPosition(NewPos);
	}

	ScoreGainText->SetRenderOpacity((1.0f - Alpha) + .3f);

	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(GainSlideTimer);
		ClearScoreGain();
	}
}

void UScorePopup::UpdateScoreLossSlide()
{
	SlideElapsedTimeLoss += 0.01f;

	float Alpha{ SlideElapsedTimeLoss / SlideDuration };
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ScoreLossText->Slot))
	{
		FVector2D const NewPos{ FMath::Lerp(SlideStartLoss, SlideEndLoss, Alpha) };
		CanvasSlot->SetPosition(NewPos);
	}

	ScoreLossText->SetRenderOpacity((1.0f - Alpha) + .3f);

	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LossSlideTimer);
		ClearScoreLoss();
	}
}

