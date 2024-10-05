﻿// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "Components/PSHUDComponent.h"
//---
#include "Bomber.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "Data/PSDataAsset.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/PSMenuWidget.h"
#include "Data/PSTypes.h"
#include "Data/PSSaveGameData.h"
#include "Data/PSWorldSubsystem.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "MyUtilsLibraries/WidgetUtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "Widgets/PSOverlayWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSHUDComponent)

// Sets default values for this component's properties
UPSHUDComponent::UPSHUDComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Subscribes to the end game state change notification on the player state.
void UPSHUDComponent::OnLocalPlayerStateReady(AMyPlayerState* PlayerState, int32 CharacterID)
{
	// Ensure that PlayerState is not null before subscribing to the event
	checkf(PlayerState, TEXT("ERROR: 'PlayerState' is null"));
	PlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Called when the game starts
void UPSHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	ProgressionMenuWidgetInternal = Cast<UPSMenuWidget>(FWidgetUtilsLibrary::CreateWidgetChecked(UPSDataAsset::Get().GetProgressionMenuWidget(), true, -10));

	ProgressionMenuOverlayWidgetInternal = Cast<UPSOverlayWidget>(FWidgetUtilsLibrary::CreateWidgetChecked(UPSDataAsset::Get().GetProgressionOverlayWidget(), true, 1));

	// Binds the local player state ready event to the handler
	BIND_ON_LOCAL_PLAYER_STATE_READY(this, ThisClass::OnLocalPlayerStateReady);

	// Listen to handle input for each game state
	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

	// Subscribe to the event notifying changes in player type
	UPSWorldSubsystem::Get().OnCurrentRowDataChanged.AddDynamic(this, &ThisClass::OnPlayerTypeChanged);

	// Save reference of this component to the world subsystem
	UPSWorldSubsystem::Get().SetHUDComponent(this);

	// Update the progression widget based on current player state
	UpdateProgressionWidgetForPlayer();
}

// Called when the component is unregistered, used to clean up resources
void UPSHUDComponent::OnUnregister()
{
	Super::OnUnregister();
	if (ProgressionMenuWidgetInternal)
	{
		FWidgetUtilsLibrary::DestroyWidget(*ProgressionMenuWidgetInternal);
		ProgressionMenuWidgetInternal = nullptr;
	}
	if (ProgressionMenuOverlayWidgetInternal)
	{
		FWidgetUtilsLibrary::DestroyWidget(*ProgressionMenuOverlayWidgetInternal);
		ProgressionMenuOverlayWidgetInternal = nullptr;
	}
}

// Save the progression depends on EEndGameState
void UPSHUDComponent::SavePoints(EEndGameState EndGameState)
{
	// @h4rdmol to move to Subsystem instead of hud
	UPSSaveGameData* SaveGameInstance = UPSWorldSubsystem::Get().GetCurrentSaveGameData();
	checkf(SaveGameInstance, TEXT("ERROR: 'SaveGameInstanceInternal' is null"));

	SaveGameInstance->SavePoints(EndGameState);
}

// Listening game states changes events 
void UPSHUDComponent::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	CurrentGameStateInternal = CurrentGameState;
	checkf(ProgressionMenuWidgetInternal, TEXT("ERROR: 'ProgressionMenuWidgetInternal' is null"));

	switch (CurrentGameState)
	{
	case ECurrentGameState::GameStarting:
		ProgressionMenuWidgetInternal->SetVisibility(ESlateVisibility::Collapsed);

	case ECurrentGameState::Menu:
		UpdateProgressionWidgetForPlayer();

	default: return;
	}
}

// Listening end game states changes events (win, lose, draw) 
void UPSHUDComponent::OnEndGameStateChanged(EEndGameState EndGameState)
{
	if (EndGameState != EEndGameState::None)
	{
		SavePoints(EndGameState);
		// show the stars widget at the bottom.
		ProgressionMenuWidgetInternal->SetVisibility(ESlateVisibility::Visible);
		DisplayLevelUIOverlay(false); // isLevelLocked to show/hide the level blocking overlay with padlock icon at InGame state always level locked is false
		ProgressionMenuWidgetInternal->SetPadding(FMargin(0, 800, 0, 0));
		UpdateProgressionWidgetForPlayer();
	}
}

// Handle events when player type changes
void UPSHUDComponent::OnPlayerTypeChanged(FPlayerTag PlayerTag)
{
	UpdateProgressionWidgetForPlayer();
}

// Refresh the main menu progression widget player 
void UPSHUDComponent::UpdateProgressionWidgetForPlayer()
{
	const FPSRowData& CurrentRowData = UPSWorldSubsystem::Get().GetCurrentRow();
	// check if empty returned Row from GetCurrentRow 
	checkf(ProgressionMenuWidgetInternal, TEXT("ERROR: 'ProgressionMenuWidgetInternal' is null"));

	//set updated amount of stars
	if (CurrentRowData.CurrentLevelProgression >= CurrentRowData.PointsToUnlock)
	{
		// set required points (stars)  to achieve for a level  
		ProgressionMenuWidgetInternal->AddImagesToHorizontalBox(CurrentRowData.PointsToUnlock, 0, CurrentRowData.PointsToUnlock);
	}
	else
	{
		// Calculate the unlocked against locked points (stars) 
		ProgressionMenuWidgetInternal->AddImagesToHorizontalBox(CurrentRowData.CurrentLevelProgression, CurrentRowData.PointsToUnlock - CurrentRowData.CurrentLevelProgression, CurrentRowData.PointsToUnlock); // Listen game state changes events 
	}

	if (CurrentGameStateInternal == ECurrentGameState::Menu)
	{
		ProgressionMenuWidgetInternal->SetPadding(FMargin(0));

		if (PSMenuWidgetEnabledInternal)
		{
			ProgressionMenuWidgetInternal->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ProgressionMenuWidgetInternal->AddImagesToHorizontalBox(0, 0, 0);
		}
	}

	DisplayLevelUIOverlay(CurrentRowData.IsLevelLocked);
}

// Show or hide the LevelUIOverlay depends on the level lock state for current level
// by default overlay is always displayed 
void UPSHUDComponent::DisplayLevelUIOverlay(bool IsLevelLocked)
{
	checkf(ProgressionMenuWidgetInternal, TEXT("ERROR: 'ProgressionMenuWidgetInternal' is null"));
	if (IsLevelLocked)
	{
		// Level is locked show the blocking overlay
		ProgressionMenuOverlayWidgetInternal->SetOverlayVisibility(ESlateVisibility::Visible, true);
	}
	else
	{
		// Level is unlocked hide the blocking overlay
		ProgressionMenuOverlayWidgetInternal->SetOverlayVisibility(ESlateVisibility::Collapsed, true);
	}
}
