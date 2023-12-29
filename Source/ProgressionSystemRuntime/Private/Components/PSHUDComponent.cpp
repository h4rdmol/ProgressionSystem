﻿// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "Components\PSHUDComponent.h"
//---
#include "Bomber.h"
#include "Controllers/MyPlayerController.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"
#include "Data/PSDataAsset.h"
#include "ProgressionSystemRuntimeModule.h"
#include "Blueprint/WidgetTree.h"
#include "UI/MyHUD.h"
#include "Widgets/ProgressionMenuWidget.h"
#include "Widgets/ProgressionSaveWidget.h"
#include "Data/PSTypes.h"
#include "PSCSaveGame.h"
#include "PSCWorldSubsystem.h"
#include "Components/Image.h"
#include "GameFramework/MyGameStateBase.h"
#include "GameFramework/MyPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyDataTable/MyDataTable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSHUDComponent)

// Sets default values for this component's properties
UPSHUDComponent::UPSHUDComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Called when the game starts
void UPSHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogProgressionSystem, Warning, TEXT("--- I'm running ---"));

	AMyHUD* MyHUD = Cast<AMyHUD>(GetOwner());
	const AMyHUD& HUD = *MyHUD;
	ProgressionMenuWidgetInternal = HUD.CreateWidgetByClass<UProgressionMenuWidget>(ProgressionSystemDataAssetInternal->GetProgressionMenuWidget(), true, 1);
	ProgressionDataTableInternal = ProgressionSystemDataAssetInternal->GetProgressionDataTable();

	// Listen states to spawn widgets
	if (AMyGameStateBase* MyGameState = UMyBlueprintFunctionLibrary::GetMyGameState())
	{
		HandleGameState(MyGameState);
	}
	else if (AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController())
	{
		MyPC->OnGameStateCreated.AddUniqueDynamic(this, &ThisClass::HandleGameState);
	}
	if (AMyPlayerState* MyPlayerState = UMyBlueprintFunctionLibrary::GetLocalPlayerState())
	{
		HandleEndGameState(MyPlayerState);
	}

	// Listen events on player type changed and Character spawned
	if (APlayerCharacter* MyPlayerCharacter = UMyBlueprintFunctionLibrary::GetLocalPlayerCharacter())
	{
		MyPlayerCharacter->OnPlayerTypeChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerTypeChanged);
	}
	else if (AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController())
	{
		MyPC->GetOnNewPawnNotifier().AddUObject(this, &ThisClass::OnCharacterPossessed);
	}

	// Save reference of this component to the world subsystem
	UPSCWorldSubsystem::Get().SetProgressionSystemComponent(this);

	LoadGameFromSave();
}

void UPSHUDComponent::LoadGameFromSave()
{
	// Check if the save game file exists
	if (UGameplayStatics::DoesSaveGameExist(SaveGameInstanceInternal->GetSaveSlotName(), SaveGameInstanceInternal->GetSaveSlotIndex()))
	{
		SaveGameInstanceInternal = Cast<UPSCSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveGameInstanceInternal->GetSaveSlotName(), SaveGameInstanceInternal->GetSaveSlotIndex()));
		
	}
	else
	{
		// Save file does not exist
		// do initial load from data table
		TMap<FName, FPSRowData> SavedProgressionRows;
		UMyDataTable::GetRows(*ProgressionDataTableInternal, SavedProgressionRows);
		SaveGameInstanceInternal = Cast<UPSCSaveGame>(UGameplayStatics::CreateSaveGameObject(UPSCSaveGame::StaticClass()));

		if (SaveGameInstanceInternal)
		{
			SaveGameInstanceInternal->SavedProgressionRows = SavedProgressionRows;
		}
	}
	SetFirstElemetAsCurrent();
	SaveDataAsync();
	UpdateProgressionWidgetForPlayer();
}

void UPSHUDComponent::SetFirstElemetAsCurrent()
{
	if (SaveGameInstanceInternal)
	{
		// Create an iterator for the TMap
		TMap<FName, FPSRowData>::TIterator Iterator = SaveGameInstanceInternal->SavedProgressionRows.CreateIterator();

		// Check if the iterator is pointing to a valid position
		if (Iterator)
		{
			// Access the key and value of the first element
			SavedProgressionRowDataInternal = Iterator.Value();
			SavedProgressionRowDataInternal.IsLevelLocked = false;
			CurrentPlayerTagInternal = SavedProgressionRowDataInternal.Character;
			SaveCurrentGameProgression();
		}		
		else
		{
			// The TMap is empty
			UE_LOG(LogTemp, Warning, TEXT("The TMap is empty."));
		}		
	}
}

// Save the progression depends on EEndGameState
void UPSHUDComponent::SavePoints(ELevelType Map, FPlayerTag Character, EEndGameState EndGameState)
{
	SavedProgressionRowDataInternal.CurrentLevelProgression += GetProgressionReward(Map, Character, EndGameState);

	if (SavedProgressionRowDataInternal.CurrentLevelProgression >= SavedProgressionRowDataInternal.PointsToUnlock)
	{
		NextLevelProgressionRowData();
	}

	SaveCurrentGameProgression();
}

// Returns rewards from data table for each type of game endings 
int32 UPSHUDComponent::GetProgressionReward(ELevelType Map, FPlayerTag Character, EEndGameState EndGameState)
{
	FName CurrentProgressionRowName = GetProgressionRowName(Map, Character);
	FPSRowData* ProgressionRowData = ProgressionDataTableInternal->FindRow<FPSRowData>(CurrentProgressionRowName, "Finding a needed row");
	if (!ProgressionRowData)
	{
		return 0;
	}

	switch (EndGameState)
	{
	case EEndGameState::Win:
		return ProgressionRowData->WinReward;
	case EEndGameState::Draw:
		return ProgressionRowData->DrawReward;
	case EEndGameState::Lose:
		return ProgressionRowData->LossReward;
	default:
		return 0;
	}
}

// Finds current game progression and save to save file
void UPSHUDComponent::SaveCurrentGameProgression()
{
	if (SaveGameInstanceInternal)
	{
		SaveGameInstanceInternal->SavedProgressionRowDataInternal = SavedProgressionRowDataInternal;

		//find in Save current FProgressionRow struct and save
		for (const auto& KeyValue : SaveGameInstanceInternal->SavedProgressionRows)
		{
			FName Key = KeyValue.Key;
			FPSRowData RowData = KeyValue.Value;

			if (RowData.Map == UMyBlueprintFunctionLibrary::GetLevelType() && RowData.Character == CurrentPlayerTagInternal)
			{
				SaveGameInstanceInternal->SavedProgressionRows[Key] = SavedProgressionRowDataInternal;
			}
		}
	}
	SaveDataAsync();
}

void UPSHUDComponent::SaveDataAsync()
{
	UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstanceInternal, SaveGameInstanceInternal->GetSaveSlotName(), SaveGameInstanceInternal->GetSaveSlotIndex());
}

FName UPSHUDComponent::GetProgressionRowName(ELevelType Map, FPlayerTag Character)
{
	FName CurrentProgressionRowName;
	TArray<FName> RowNames = ProgressionDataTableInternal->GetRowNames();
	for (auto RowName : RowNames)
	{
		FPSRowData* Row = ProgressionDataTableInternal->FindRow<FPSRowData>(RowName, "Finding progression row name");
		if (Row)
		{
			if (Row->Map == Map && Row->Character == Character)
			{
				CurrentProgressionRowName = RowName;
			}
		}
	}
	return CurrentProgressionRowName;
}

// Find next item in the level from current. TMap represents levels in savefile 
void UPSHUDComponent::NextLevelProgressionRowData()
{
	FName Key;

	if (SaveGameInstanceInternal)
	{
		for (const auto& KeyValue : SaveGameInstanceInternal->SavedProgressionRows)
		{
			if (SavedProgressionRowDataInternal.Map == KeyValue.Value.Map && SavedProgressionRowDataInternal.Character == KeyValue.Value.Character)
			{
				Key = KeyValue.Key;
			}
		}

		// Find the iterator for the current key
		TMap<FName, FPSRowData>::TIterator CurrentIterator = SaveGameInstanceInternal->SavedProgressionRows.CreateIterator();

		// Iterate through the map until the specified key is found
		while (CurrentIterator && CurrentIterator.Key() != Key)
		{
			++CurrentIterator;
		}

		// Check if the iterator is pointing to a valid position
		if (CurrentIterator)
		{
			// Move to the next element
			++CurrentIterator;

			// Check if the iterator is pointing to a valid position (not at the end)
			if (CurrentIterator)
			{
				// Access the key and value using the iterator
				FName NextProgressionName = CurrentIterator.Key();
				SaveGameInstanceInternal->SavedProgressionRows[NextProgressionName].IsLevelLocked = false;

				SaveDataAsync();
			}
		}
	}
}

// Listening game states changes events 
void UPSHUDComponent::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	// Show Progression Menu widget in Main Menu
	ProgressionMenuWidgetInternal->SetVisibility(CurrentGameState == ECurrentGameState::Menu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (CurrentGameState == ECurrentGameState::Menu)
	{
		UpdateProgressionWidgetForPlayer();
	}
}

// Listening end game states changes events (win, lose, draw) 
void UPSHUDComponent::OnEndGameStateChanged(EEndGameState EndGameState)
{
	if (EndGameState != EEndGameState::None)
	{
		SavePoints(UMyBlueprintFunctionLibrary::GetLevelType(), CurrentPlayerTagInternal, EndGameState);
	}
}

// Listen game state changes events 
void UPSHUDComponent::HandleGameState(AMyGameStateBase* MyGameState)
{
	checkf(MyGameState, TEXT("ERROR: 'MyGameState' is null!"));

	// Listen states to handle this widget behavior
	MyGameState->OnGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnGameStateChanged);

	if (MyGameState->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		// Handle current game state initialized with delay
		OnGameStateChanged(ECurrentGameState::Menu);
	}
	else
	{
		// Enter the game in Menu game state
		MyGameState->ServerSetGameState(ECurrentGameState::Menu);
	}
}

void UPSHUDComponent::HandleEndGameState(AMyPlayerState* MyPlayerState)
{
	checkf(MyPlayerState, TEXT("ERROR: 'MyGameState' is null!"));
	MyPlayerState->OnEndGameStateChanged.AddUniqueDynamic(this, &ThisClass::OnEndGameStateChanged);
}

// Listen event whe Character Possessed 
void UPSHUDComponent::OnCharacterPossessed(APawn* MyPawn)
{
	if (APlayerCharacter* MyPlayerCharacter = Cast<APlayerCharacter>(MyPawn))
	{
		MyPlayerCharacter->OnPlayerTypeChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerTypeChanged);

		//Unsubscribe to ignore null events call
		AMyPlayerController* MyPC = UMyBlueprintFunctionLibrary::GetLocalPlayerController();
		MyPC->GetOnNewPawnNotifier().RemoveAll(this);
	}
}

// Handle events when player type changes
void UPSHUDComponent::OnPlayerTypeChanged(FPlayerTag PlayerTag)
{
	UE_LOG(LogTemp, Warning, TEXT("Prev player tag: : %s"), *CurrentPlayerTagInternal.ToString());
	CurrentPlayerTagInternal = PlayerTag;
	UE_LOG(LogTemp, Warning, TEXT("isLevelLocked: %d"), SavedProgressionRowDataInternal.IsLevelLocked);
	UE_LOG(LogTemp, Warning, TEXT("Current player tag: : %s"), *CurrentPlayerTagInternal.ToString());
	UpdateProgressionWidgetForPlayer();
}

// Refresh the main menu progression widget player 
void UPSHUDComponent::UpdateProgressionWidgetForPlayer()
{
	if (SaveGameInstanceInternal)
	{
		for (const auto& KeyValue : SaveGameInstanceInternal->SavedProgressionRows)
		{
			FName Key = KeyValue.Key;
			FPSRowData RowData = KeyValue.Value;

			if (RowData.Map == UMyBlueprintFunctionLibrary::GetLevelType() && RowData.Character == CurrentPlayerTagInternal)
			{
				SavedProgressionRowDataInternal = SaveGameInstanceInternal->SavedProgressionRows[Key];
			}
		}
		SaveDataAsync();

		// reset amount of start for a level
		ProgressionMenuWidgetInternal->ClearImagesFromHorizontalBox();

		//set updated amount of stars
		if (SavedProgressionRowDataInternal.CurrentLevelProgression >= SavedProgressionRowDataInternal.PointsToUnlock)
		{
			// set required points (stars)  to achieve for a level  
			ProgressionMenuWidgetInternal->AddImagesToHorizontalBox(SavedProgressionRowDataInternal.PointsToUnlock, 0);
		}
		else
		{
			// Calculate the unlocked against locked points (stars) 
			ProgressionMenuWidgetInternal->AddImagesToHorizontalBox(SavedProgressionRowDataInternal.CurrentLevelProgression, SavedProgressionRowDataInternal.PointsToUnlock - SavedProgressionRowDataInternal.CurrentLevelProgression); // Listen game state changes events 
		}
		DisplayLevelUIOverlay(SavedProgressionRowDataInternal.IsLevelLocked);
	}
}

// Show or hide the LevelUIOverlay depends on the level lock state for current level
// by default overlay is always displayed 
void UPSHUDComponent::DisplayLevelUIOverlay(bool IsLevelLocked)
{
	UE_LOG(LogTemp, Warning, TEXT("isLevelLocked: %d"), IsLevelLocked);
	if (IsLevelLocked)
	{
		// Level is locked show the blocking overlay
		ProgressionMenuWidgetInternal->PSCBackgroundOverlay->SetVisibility(ESlateVisibility::Visible);
		ProgressionMenuWidgetInternal->PSCBackgroundIconLock->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// Level is unlocked hide the blocking overlay
		ProgressionMenuWidgetInternal->PSCBackgroundOverlay->SetVisibility(ESlateVisibility::Collapsed);
		ProgressionMenuWidgetInternal->PSCBackgroundIconLock->SetVisibility(ESlateVisibility::Collapsed);
	}
}
