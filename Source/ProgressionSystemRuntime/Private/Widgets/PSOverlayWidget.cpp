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
		if (!ensureMsgf(PSCBackgroundOverlay, TEXT("ASSERT: [%i] %s:\n'PSCBackgroundOverlay' is not valid!"), __LINE__, *FString(__FUNCTION__)))
		{
			return;
		}
		if (!ensureMsgf(PSCBackgroundIconLock, TEXT("ASSERT: [%i] %s:\n'PSCBackgroundIconLock' is not valid!"), __LINE__, *FString(__FUNCTION__)))
		{
			return;
		}
		ESlateVisibility PrevOverlayOpacity = PSCBackgroundOverlay->GetVisibility();
		ESlateVisibility PrevIconOpacity = PSCBackgroundIconLock->GetVisibility();

		//if new visibility is same as previous animation is not required 
		if (PrevOverlayOpacity == VisibilitySlate && PrevIconOpacity == VisibilitySlate)
		{
			bShouldPlayFadeAnimationInternal = false;
		}
		bIsFadeInAnimationInternal = true;
		SetOverlayItemsVisibility(VisibilitySlate);
	}
	else
	{
		bIsFadeInAnimationInternal = false;
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
	if (!UPSDataAsset::Get().GetFadeCurveFloat() || !bShouldPlayFadeAnimationInternal)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!ensureMsgf(World, TEXT("ASSERT: 'World' is not valid")))
	{
		return;
	}

	const float SecondsSinceStart = GetWorld()->GetTimeSeconds() - StartTimeFadeAnimationInternal;
	float OpacityValue = UPSDataAsset::Get().GetFadeCurveFloat()->GetFloatValue(SecondsSinceStart);

	float MinTime = 0.f;
	float MaxTime = 0.f;
	UPSDataAsset::Get().GetFadeCurveFloat()->GetTimeRange(MinTime, MaxTime);
	if (SecondsSinceStart >= MaxTime)
	{
		// The curve is finished
		if (!bIsFadeInAnimationInternal)
		{
			SetOverlayItemsVisibility(ESlateVisibility::Collapsed);
		}
		bShouldPlayFadeAnimationInternal = false;
		return;
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
}

void UPSOverlayWidget::SetOverlayItemsVisibility(ESlateVisibility VisibilitySlate)
{
	// Level is unlocked hide the blocking overlay
	PSCBackgroundOverlay->SetVisibility(VisibilitySlate);
	PSCBackgroundIconLock->SetVisibility(VisibilitySlate);
}
