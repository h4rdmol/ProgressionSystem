// Copyright (c) Yevhenii Selivanov


#include "Widgets/PSOverlayWidget.h"

#include "Components/Image.h"
#include "Curves/CurveFloat.h"
#include "Data/PSDataAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSOverlayWidget)

// Sets the visibility of the overlay elements and playing fade animation if needed
void UPSOverlayWidget::SetOverlayVisibility(ESlateVisibility VisibilitySlate, bool bShouldPlayFadeAnimation/* = false*/ )
{
	if (!bShouldPlayFadeAnimation)
	{
		SetOverlayItemsVisibility(VisibilitySlate);
		return;
	}

	bShouldPlayFadeAnimationInternal = bShouldPlayFadeAnimation;
	
	if (VisibilitySlate == ESlateVisibility::Visible)
	{
		bIsFadeInAnimationInternal = true;
		SetOverlayItemsVisibility(VisibilitySlate);
	}
	else
	{
		bIsFadeInAnimationInternal = false;
	}

	const UWorld* World = GetWorld();
	check(World);

	StartTimeFadeAnimationInternal = World->GetTimeSeconds();
}

// overrides NativeTick to make the user widget tickable
void UPSOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bShouldPlayFadeAnimationInternal)
	{
		const bool bIsFinished = !FadeOverlayElementsAnimation(StartTimeFadeAnimationInternal);
		if (bIsFinished)
		{
			bShouldPlayFadeAnimationInternal = false;
			StartTimeFadeAnimationInternal = 0.f;
			if (!bIsFadeInAnimationInternal)
			{
				SetOverlayItemsVisibility(ESlateVisibility::Collapsed);	
			}
			
		}
	}
}

// Event to execute when widget is ready
void UPSOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FadeCurveFloatInternal = UPSDataAsset::Get().GetFadeCurveFloat();
}

// Play the overlay elemetns fade-in/fade-out animation. Uses the internal FadeCurveFloatInternal initialized in NativeConstruct
bool UPSOverlayWidget::FadeOverlayElementsAnimation(float& StartTimeRef)
{
	if (!StartTimeRef || !FadeCurveFloatInternal || !bShouldPlayFadeAnimationInternal)
	{
		StartTimeRef = 0.f;
		return false;
	}
	
	const UWorld* World = GetWorld();
	check(World);
	
	float test = World->GetTimeSeconds();
	const float SecondsSinceStart = GetWorld()->GetTimeSeconds() - StartTimeRef;
	float OpacityValue = FadeCurveFloatInternal->GetFloatValue(SecondsSinceStart);

	float MinTime = 0.f;
	float MaxTime = 0.f;
	FadeCurveFloatInternal->GetTimeRange(MinTime, MaxTime);
	if (SecondsSinceStart >= MaxTime)
	{
		// The curve is finished
		return false;
	}
	
	if (!bIsFadeInAnimationInternal)
	{
		OpacityValue = MaxTime - OpacityValue;
	}

	if (PSCBackgroundOverlay && PSCBackgroundIconLock)
	{
		PSCBackgroundOverlay->SetRenderOpacity(OpacityValue);
		PSCBackgroundIconLock->SetRenderOpacity(OpacityValue);
	}
	return true;
}

void UPSOverlayWidget::SetOverlayItemsVisibility(ESlateVisibility VisibilitySlate)
{
	// Level is unlocked hide the blocking overlay
	PSCBackgroundOverlay->SetVisibility(VisibilitySlate);
	PSCBackgroundIconLock->SetVisibility(VisibilitySlate);
}
