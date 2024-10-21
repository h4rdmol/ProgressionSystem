// Copyright (c) Yevhenii Selivanov


#include "Widgets/PSOverlayWidget.h"

#include "Components/Image.h"
#include "Curves/CurveFloat.h"
#include "Data/PSDataAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSOverlayWidget)

// Sets the visibility of the overlay elements and playing fade animation if needed
void UPSOverlayWidget::SetOverlayVisibility(ESlateVisibility VisibilitySlate, bool bShouldPlayFadeAnimation/* = false*/)
{
	if (!bShouldPlayFadeAnimation)
	{
		SetOverlayItemsVisibility(VisibilitySlate);
		return;
	}

	bShouldPlayFadeAnimationInternal = bShouldPlayFadeAnimation;

	if (VisibilitySlate == ESlateVisibility::Visible)
	{
		if (!ensureMsgf(PSCOverlay, TEXT("ASSERT: [%i] %s:\n'PSCOverlay' is not valid!"), __LINE__, *FString(__FUNCTION__)))
		{
			return;
		}
		ESlateVisibility PrevOverlayOpacity = PSCOverlay->GetVisibility();

		//if new visibility is same as previous animation is not required 
		if (PrevOverlayOpacity == VisibilitySlate)
		{
			bShouldPlayFadeAnimationInternal = false;
		}
		SetOverlayItemsVisibility(VisibilitySlate);
	}
	else
	{
		SetOverlayItemsVisibility(VisibilitySlate);
	}

	const UWorld* World = GetWorld();
	if (!ensureMsgf(World, TEXT("ASSERT: [%i] %s:\n'World' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	StartTimeFadeAnimationInternal = World->GetTimeSeconds();
}

// overrides NativeTick to make the user widget tickable
void UPSOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TickPlayFadeOverlayAnimation();
}

// Event to execute when widget is ready
void UPSOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

// Play the overlay elements fade-in/fade-out animation. Uses the internal FadeCurveFloatInternal initialized in NativeConstruct
void UPSOverlayWidget::TickPlayFadeOverlayAnimation()
{
	const UWorld* World = GetWorld();

	if (!bShouldPlayFadeAnimationInternal
		|| !World
		|| !ensureMsgf(UPSDataAsset::Get().GetOverlayFadeDuration() > 0.0f, TEXT("ASSERT: [%i] %hs:\n'FadeDuration' must be greater than 0"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const float SecondsSinceStart = GetWorld()->GetTimeSeconds() - StartTimeFadeAnimationInternal;
	const float OpacityValue = FMath::Clamp(SecondsSinceStart / UPSDataAsset::Get().GetOverlayFadeDuration(), 0.0f, 1.0f);

	if (SecondsSinceStart >= UPSDataAsset::Get().GetOverlayFadeDuration())
	{
		bShouldPlayFadeAnimationInternal = false;
		return;
	}

	if (PSCOverlay)
	{
		PSCOverlay->SetRenderOpacity(OpacityValue);
	}
}

void UPSOverlayWidget::SetOverlayItemsVisibility(ESlateVisibility VisibilitySlate)
{
	// Level is unlocked hide the blocking overlay
	PSCOverlay->SetVisibility(VisibilitySlate);
}
