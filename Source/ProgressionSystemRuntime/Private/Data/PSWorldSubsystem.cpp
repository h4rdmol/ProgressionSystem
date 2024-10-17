// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "Data/PSWorldSubsystem.h"

#include "PoolManagerSubsystem.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Components/PSHUDComponent.h"
#include "Components/PSSpotComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/PSDataAsset.h"
#include "Data/PSSaveGameData.h"
#include "DataAssets/GameStateDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyDataTable/MyDataTable.h"
#include "MyUtilsLibraries/UtilsLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PSStarActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Subsystems/GameDifficultySubsystem.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSWorldSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UPSWorldSubsystem& UPSWorldSubsystem::Get()
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	checkf(World, TEXT("%s: 'World' is null"), *FString(__FUNCTION__));
	UPSWorldSubsystem* ThisSubsystem = World->GetSubsystem<ThisClass>();
	checkf(ThisSubsystem, TEXT("%s: 'ProgressionSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns this Subsystem, is checked and wil crash if can't be obtained
UPSWorldSubsystem& UPSWorldSubsystem::Get(const UObject& WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObjectChecked(&WorldContextObject);
	checkf(World, TEXT("%s: 'World' is null"), *FString(__FUNCTION__));
	UPSWorldSubsystem* ThisSubsystem = World->GetSubsystem<ThisClass>();
	checkf(ThisSubsystem, TEXT("%s: 'ProgressionSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns current row of progression system
const FPSRowData& UPSWorldSubsystem::GetCurrentRow() const
{
	const UPSSaveGameData* SaveGameInstance = GetCurrentSaveGameData();
	checkf(SaveGameInstance, TEXT("ERROR: 'SaveGameInstanceInternal' is null"));

	return SaveGameInstance->GetCurrentRow();
}

// Set current row of progression system by tag
void UPSWorldSubsystem::SetCurrentRowByTag(FPlayerTag NewRowPlayerTag)
{
	// check if current row is similar
	if (GetCurrentRow().Character == NewRowPlayerTag)
	{
		return;
	}

	UPSSaveGameData* SaveGameInstance = GetCurrentSaveGameData();
	checkf(SaveGameInstance, TEXT("ERROR: 'SaveGameInstanceInternal' is null"));

	SaveGameInstance->SetRowByTag(NewRowPlayerTag);
	OnCurrentRowDataChanged.Broadcast(NewRowPlayerTag);
}

// Returns previous row of progression system
const FPSRowData& UPSWorldSubsystem::GetPreviousRow() const
{
	const UPSSaveGameData* SaveGameInstance = GetCurrentSaveGameData();
	checkf(SaveGameInstance, TEXT("ERROR: 'SaveGameInstanceInternal' is null"));

	return SaveGameInstance->GetPreviousRow();
}

// Returns the data asset that contains all the assets of Progression System game feature
const UPSDataAsset* UPSWorldSubsystem::GetPSDataAsset() const
{
	return PSDataAssetInternal.LoadSynchronous();
}

// Set the progression system component
void UPSWorldSubsystem::SetHUDComponent(UPSHUDComponent* MyHUDComponent)
{
	checkf(MyHUDComponent, TEXT("%s: My progression system component is null"), *FString(__FUNCTION__));
	PSHUDComponentInternal = MyHUDComponent;
}

// Set the progression system spot component
void UPSWorldSubsystem::RegisterSpotComponent(UPSSpotComponent* MyHUDComponent)
{
	checkf(MyHUDComponent, TEXT("%s: My progression system component is null"), *FString(__FUNCTION__));
	PSSpotComponentArrayInternal.AddUnique(MyHUDComponent);
	MyHUDComponent->OnSpotComponentReady.AddDynamic(this, &UPSWorldSubsystem::OnSpotComponentLoad);
}

void UPSWorldSubsystem::SetCurrentSpotComponent(UPSSpotComponent* MyHUDComponent)
{
	checkf(MyHUDComponent, TEXT("%s: My progression system component is null"), *FString(__FUNCTION__));
	PSCurrentSpotComponentInternal = MyHUDComponent;
	UpdateProgressionActorsForSpot();
}

// Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors 
void UPSWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	const TArray<FName>& GameFeaturesToEnable = UGameStateDataAsset::Get().GetGameFeaturesToEnable();
	for (const FName GameFeatureName : GameFeaturesToEnable)
	{
		if (GameFeatureName.IsNone())
		{
			continue;
		}

		if (GameFeatureName == "ProgressionSystem")
		{
			// Subscribe events on player type changed and Character spawned
			BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnCharacterReady);

			// Listen to handle input for each game state
			BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);

			LoadGameFromSave();

			StarDynamicProgressMaterial = UMaterialInstanceDynamic::Create(UPSDataAsset::Get().GetDynamicProgressionMaterial(), this);
			checkf(StarDynamicProgressMaterial, TEXT("ERROR: 'StarDynamicProgressMaterial' is null"));
		}
	}
}

// Clears all transient data created by this subsystem
void UPSWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
	PSHUDComponentInternal = nullptr;
	PSCurrentSpotComponentInternal = nullptr;
	SaveGameInstanceInternal = nullptr;
	StarDynamicProgressMaterial = nullptr;
}

// Is called when a player character is ready
void UPSWorldSubsystem::OnCharacterReady(APlayerCharacter* PlayerCharacter, int32 CharacterID)
{
	PlayerCharacter->OnPlayerTypeChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerTypeChanged);
}

// Is called when a player has been changed
void UPSWorldSubsystem::OnPlayerTypeChanged(FPlayerTag PlayerTag)
{
	SetCurrentRowByTag(PlayerTag);

	for (UPSSpotComponent* SpotComponent : PSSpotComponentArrayInternal)
	{
		if (SpotComponent->GetMeshChecked().GetPlayerTag() == PlayerTag)
		{
			PSCurrentSpotComponentInternal = SpotComponent;
			UpdateProgressionActorsForSpot();
		}
	}
}

// Called when the current game state was changed
void UPSWorldSubsystem::OnGameStateChanged(ECurrentGameState CurrentGameState)
{
	switch (CurrentGameState)
	{
	case ECurrentGameState::Menu:
		// refresh 3D Stars actors
		UpdateProgressionActorsForSpot();
		break;
	case ECurrentGameState::GameStarting:
		// Show Progression Menu widget in Main Menu
		break;
	default:
		return;
	}
}

// Load game from save file or create a new one (does initial load from data table)
void UPSWorldSubsystem::LoadGameFromSave()
{
	// Check if the save game file exists
	if (UGameplayStatics::DoesSaveGameExist(SaveGameInstanceInternal->GetSaveSlotName(), SaveGameInstanceInternal->GetSaveSlotIndex()))
	{
		SaveGameInstanceInternal = Cast<UPSSaveGameData>(UGameplayStatics::LoadGameFromSlot(SaveGameInstanceInternal->GetSaveSlotName(), SaveGameInstanceInternal->GetSaveSlotIndex()));
	}
	else
	{
		// Save file does not exist
		// do initial load from data table
		TMap<FName, FPSRowData> SavedProgressionRows;
		const UDataTable* ProgressionDataTable = UPSDataAsset::Get().GetProgressionDataTable();
		checkf(ProgressionDataTable, TEXT("ERROR: 'ProgressionDataTableInternal' is null"));
		UMyDataTable::GetRows(*ProgressionDataTable, SavedProgressionRows);
		SaveGameInstanceInternal = Cast<UPSSaveGameData>(UGameplayStatics::CreateSaveGameObject(UPSSaveGameData::StaticClass()));

		if (SaveGameInstanceInternal)
		{
			SaveGameInstanceInternal->SetProgressionMap(SavedProgressionRows);
		}
	}
	SetFirstElemetAsCurrent();
}

// Always set first levels as unlocked on begin play
void UPSWorldSubsystem::SetFirstElemetAsCurrent()
{
	if (SaveGameInstanceInternal)
	{
		constexpr int32 FirstElementIndex = 0;
		SaveGameInstanceInternal->SetCurrentProgressionRowByIndex(FirstElementIndex);
		const FName RowName = SaveGameInstanceInternal->GetCurrentRowName();
		SaveGameInstanceInternal->UnlockLevelByName(RowName);
	}
	SaveDataAsync();
}

// Updates the stars actors for a spot
void UPSWorldSubsystem::UpdateProgressionActorsForSpot()
{
	AddProgressionStarActors();
}

// Spawn/add the stars actors for a spot
void UPSWorldSubsystem::AddProgressionStarActors()
{
	const FPSRowData& CurrentRowData = GetCurrentRow();

	//if it's not loaded yet
	if (!CurrentRowData.PointsToUnlock)
	{
		return;
	}

	//Return to Pool Manager the list of handles which is not needed (if there are any) 
	UPoolManagerSubsystem::Get().ReturnToPoolArray(PoolActorHandlersInternal);
	// --- Prepare spawn request
	const TWeakObjectPtr<ThisClass> WeakThis = this;
	const FOnSpawnAllCallback OnTakeActorsFromPoolCompleted = [WeakThis](const TArray<FPoolObjectData>& CreatedObjects)
	{
		if (UPSWorldSubsystem* This = WeakThis.Get())
		{
			This->OnTakeActorsFromPoolCompleted(CreatedObjects);
		}
	};

	// --- Spawn actors
	UPoolManagerSubsystem::Get().TakeFromPoolArray(PoolActorHandlersInternal, UPSDataAsset::Get().GetStarActorClass(), CurrentRowData.PointsToUnlock, OnTakeActorsFromPoolCompleted, ESpawnRequestPriority::High);
}

// Dynamically adds Star actors which representing unlocked and locked progression above the character
void UPSWorldSubsystem::OnTakeActorsFromPoolCompleted(const TArray<FPoolObjectData>& CreatedObjects)
{
	const FPSRowData& CurrentRowData = GetCurrentRow();
	float AmountUnlocked = CurrentRowData.CurrentLevelProgression;
	FVector PreviousActorLocationRef;

	for (const FPoolObjectData& CreatedObject : CreatedObjects)
	{
		APSStarActor& SpawnedActor = CreatedObject.GetChecked<APSStarActor>();

		float StarAmount = FMath::Clamp(AmountUnlocked, 0.0f, 1.0f);
		if (AmountUnlocked > 0)
		{
			SpawnedActor.UpdateStarActorMeshMaterial(StarDynamicProgressMaterial, StarAmount, false);
		}
		else
		{
			SpawnedActor.UpdateStarActorMeshMaterial(StarDynamicProgressMaterial, 1, true);
		}
		AmountUnlocked -= StarAmount;

		SpawnedActor.OnInitialized(PreviousActorLocationRef);
	}
}

// Triggers when a spot is loaded
void UPSWorldSubsystem::OnSpotComponentLoad(UPSSpotComponent* SpotComponent)
{
	if (SpotComponent)
	{
		PSCurrentSpotComponentInternal = SpotComponent;
	}
}

// Saves the progression to the local files
void UPSWorldSubsystem::SaveDataAsync()
{
	checkf(SaveGameInstanceInternal, TEXT("ERROR: 'SaveGameInstanceInternal' is null"));
	UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstanceInternal, SaveGameInstanceInternal->GetSaveSlotName(), SaveGameInstanceInternal->GetSaveSlotIndex());
}

// Removes all saved data of the Progression system and creates a new empty data
void UPSWorldSubsystem::ResetSaveGameData()
{
	const FString& SlotName = SaveGameInstanceInternal->GetSaveSlotName();
	const int32 UserIndex = SaveGameInstanceInternal->GetSaveSlotIndex();

	UGameplayUtilsLibrary::ResetSaveGameData(SaveGameInstanceInternal, SlotName, UserIndex);

	// Re-load a new save game object. Load game from save creates a save file if there is no such
	LoadGameFromSave();
	UPSHUDComponent* PSHUDComponent = GetProgressionSystemHUDComponent();
	checkf(PSHUDComponent, TEXT("ERROR: 'PSHUDComponent' is null"));
	PSHUDComponent->UpdateProgressionWidgetForPlayer();
	UpdateProgressionActorsForSpot();
}

// Unlocks all levels of the Progression System
void UPSWorldSubsystem::UnlockAllLevels()
{
	SaveGameInstanceInternal->UnlockAllLevels();
	SaveDataAsync();
	UPSHUDComponent* PSHUDComponent = GetProgressionSystemHUDComponent();
	checkf(PSHUDComponent, TEXT("ERROR: 'PSHUDComponent' is null"));
	PSHUDComponent->UpdateProgressionWidgetForPlayer();
	UpdateProgressionActorsForSpot();
}

// Returns difficultyMultiplier
float UPSWorldSubsystem::GetDifficultyMultiplier()
{
	TMap<EGameDifficulty, float> DifficultyMap = UPSDataAsset::Get().GetProgressionDifficultyMultiplier();
	ensureMsgf(!DifficultyMap.IsEmpty(), TEXT("ASSERT: DifficultyMap is empty"));

	switch (UGameDifficultySubsystem::GetGameDifficultySubsystem()->GetDifficultyType())
	{
	case EGameDifficulty::Easy:
		return *DifficultyMap.Find(EGameDifficulty::Easy);
	case EGameDifficulty::Normal:
		return *DifficultyMap.Find(EGameDifficulty::Normal);
	case EGameDifficulty::Hard:
		return *DifficultyMap.Find(EGameDifficulty::Hard);
	case EGameDifficulty::Vanilla:
		return 1.0f;
	default:
		return 1.0f;
	}
}
