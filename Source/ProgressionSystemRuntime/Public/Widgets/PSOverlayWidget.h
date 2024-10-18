// Copyright (c) Yevhenii Selivanov

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PSOverlayWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROGRESSIONSYSTEMRUNTIME_API UPSOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	* Sets the visibility of the overlay elements and playing fade animation if needed 
	* @param VisibilitySlate The visibility state (e.g., Visible, Collapsed) to apply to the overlay and icon.
	* @param bShouldPlayFadeAnimation Defines if the fade animation should be played after or before widget is visible
	*/
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void SetOverlayVisibility(ESlateVisibility VisibilitySlate, bool bShouldPlayFadeAnimation);

protected:
	/** overrides NativeTick to make the user widget tickable **/ 
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/** Event to execute when widget is ready */
	virtual void NativeConstruct() override;

	/**
	* Play the overlay elements fade-in/fade-out animation.
	* Uses the internal FadeCurveFloatInternal initialized in NativeConstruct
	*/
	void TickPlayFadeOverlayAnimation();

	/**
	* Sets the visibility of the background overlay and lock icon.
	* @param VisibilitySlate The visibility state (e.g., Visible, Collapsed) to apply to the overlay and icon.
	*/
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void SetOverlayItemsVisibility(ESlateVisibility VisibilitySlate);
	
	/** Background overlay for tint effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UImage> PSCBackgroundOverlay = nullptr;

	/** Background overlay lock icon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UImage> PSCBackgroundIconLock = nullptr;

	/** Stores the starting time to fade-in/fade-out overlay in the main menu when cinematic started */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to hide stars"))
	float StartTimeFadeAnimationInternal = 0.0f;

	/** if the fade-in/fade-out overlay animation in the main menu when cinematic started should be played */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to hide stars"))
	bool bShouldPlayFadeAnimationInternal = false;

	/** if the fade-in/fade-out overlay animation in the main menu when cinematic started should be played */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to hide stars"))
	bool bIsFadeInAnimationInternal = false;

	/** Stores the default fade-in/fade-out curve float animation */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to hide stars"))
	TObjectPtr<class UCurveFloat> FadeCurveFloatInternal = nullptr;
	
};
