﻿// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "Widgets/PSEndGameWidget.h"
//---
#include "Data/PSDataAsset.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Widgets/PSStarWidget.h"
//---

#include "PoolManagerSubsystem.h"
#include "PoolManagerTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/RealCurve.h"
#include "Engine/CurveTable.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(PSEndGameWidget)

// Called after the underlying slate widget is constructed.
void UPSEndGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide this widget by default
	SetVisibility(ESlateVisibility::Collapsed);

	// Listen to handle input for each game state
	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	// Binds the local player state ready event to the handler
	BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);
}

// Called when the end game state was changed to toggle progression widget visibility
void UPSEndGameWidget::OnGameStateChanged_Implementation(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
	case ECurrentGameState::GameStarting:
		SetVisibility(ESlateVisibility::Collapsed);
		break;
	case ECurrentGameState::Menu:
		SetVisibility(ESlateVisibility::Collapsed);
		break;
	default: break;
	}
}

// Subscribes to the end game state change notification on the player state
void UPSEndGameWidget::OnLocalPlayerStateReady_Implementation(AMyPlayerState* PlayerState, int32 CharacterID)
{
	// Ensure that PlayerState is not null before subscribing to the event
	checkf(PlayerState, TEXT("ERROR: [%i] %hs:\n'PlayerState' is null!"), __LINE__, __FUNCTION__);
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Called when the end game state was changed
void UPSEndGameWidget::OnEndGameStateChanged_Implementation(EEndGameState EndGameState)
{
	if (EndGameState != EEndGameState::None)
	{
		// show the stars widget at the bottom.
		SetVisibility(ESlateVisibility::Visible);
	}
}

// Dynamically populates a Horizontal Box with images representing unlocked and locked progression icons.
void UPSEndGameWidget::AddImagesToHorizontalBox(float AmountOfUnlockedPoints, float AmountOfLockedPoints, float MaxLevelPoints)
{
	//Return to Pool Manager the list of handles which is not needed (if there are any) 

	if (!PoolWidgetHandlersInternal.IsEmpty())
	{
		UPoolManagerSubsystem::Get().ReturnToPoolArray(PoolWidgetHandlersInternal);
		PoolWidgetHandlersInternal.Empty();
	}

	// --- Prepare spawn request
	const TWeakObjectPtr<ThisClass> WeakThis = this;
	const FOnSpawnAllCallback OnTakeFromPoolCompleted = [WeakThis, AmountOfUnlockedPoints, AmountOfLockedPoints, MaxLevelPoints](const TArray<FPoolObjectData>& CreatedObjects)
	{
		if (UPSEndGameWidget* This = WeakThis.Get())
		{
			This->OnTakeFromPoolCompleted(CreatedObjects, AmountOfUnlockedPoints, AmountOfLockedPoints, MaxLevelPoints);
		}
	};

	// --- Spawn widgets
	const int32 TotalRequests = AmountOfLockedPoints + AmountOfUnlockedPoints;
	if (TotalRequests == 0)
	{
		// no items to request nothing to add
		return;
	}
	UPoolManagerSubsystem::Get().TakeFromPoolArray(PoolWidgetHandlersInternal, UPSDataAsset::Get().GetStarWidgetClass(), TotalRequests, OnTakeFromPoolCompleted);
}

// Dynamically populates a Horizontal Box with images representing unlocked and locked progression icons
void UPSEndGameWidget::OnTakeFromPoolCompleted(const TArray<FPoolObjectData>& CreatedObjects, float AmountOfUnlockedPoints, float AmountOfLockedPoints, float MaxLevelPoints)
{
	if (!ensureMsgf(HorizontalBox, TEXT("ASSERT: [%i] %hs:\n'HorizontalBox' is null!"), __LINE__, __FUNCTION__))
	{
		return;
	}
	HorizontalBox->ClearChildren();
	float CurrentAmountOfUnlocked = AmountOfUnlockedPoints;
	float CurrentAmountOfLocked = AmountOfLockedPoints;
	// Setup spawned widget
	for (const FPoolObjectData& CreatedObject : CreatedObjects)
	{
		if (CurrentAmountOfUnlocked > 0)
		{
			// #1 Create MyFunction
			UpdateStarImages(CreatedObject, 1, 0);
			// more than 0 means that it's not an integer
			if ((MaxLevelPoints - CurrentAmountOfUnlocked) > 0)
			{
				UpdateStarProgressBarValue(CreatedObject, CurrentAmountOfUnlocked);
			}
			CurrentAmountOfUnlocked--;
			continue;
		}

		if (CurrentAmountOfLocked > 0)
		{
			UpdateStarImages(CreatedObject, 0, 1);
			CurrentAmountOfLocked--;
		}
	}
}

// Updates star images icon to locked/unlocked according to input amounnt
void UPSEndGameWidget::UpdateStarImages(const FPoolObjectData& CreatedData, float AmountOfUnlockedStars, float AmountOfLockedStars)
{
	UPSStarWidget& SpawnedWidget = CreatedData.GetChecked<UPSStarWidget>();

	if (AmountOfUnlockedStars > 0)
	{
		SpawnedWidget.SetStarImage(UPSDataAsset::Get().GetUnlockedProgressionIcon());
		SpawnedWidget.UpdateProgressionBarPercentage(AmountOfUnlockedStars);
	}

	if (AmountOfLockedStars > 0)
	{
		SpawnedWidget.SetStarImage(UPSDataAsset::Get().GetLockedProgressionIcon());
	}
	// Load and set the image texture here using ImagePath or other methods
	if (!HorizontalBox->HasChild(&SpawnedWidget))
	{
		HorizontalBox->AddChildToHorizontalBox(&SpawnedWidget);
		SpawnedWidget.UpdateProgressionBarPercentage(AmountOfUnlockedStars);
	}
}

// Updates Progress bar icon for unlocked icons 
void UPSEndGameWidget::UpdateStarProgressBarValue(const FPoolObjectData& CreatedData, float NewProgressBarValue)
{
	UPSStarWidget& SpawnedWidget = CreatedData.GetChecked<UPSStarWidget>();
	SpawnedWidget.UpdateProgressionBarPercentage(NewProgressBarValue);
}