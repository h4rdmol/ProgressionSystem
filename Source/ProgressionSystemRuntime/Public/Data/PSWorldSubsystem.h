// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#pragma once

#include "PSTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "PoolManagerTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PSWorldSubsystem.generated.h"

enum class ECurrentGameState : uint8;

/**
 * Implements the world subsystem to access different components in the module 
 */
UCLASS(BlueprintType, Blueprintable, Config = "ProgressionSystem", DefaultConfig)
class PROGRESSIONSYSTEMRUNTIME_API UPSWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCurrentRowDataChanged, const FPlayerTag, SavedProgressionRowData);

	/** Returns this Subsystem, is checked and will crash if it can't be obtained.*/
	static UPSWorldSubsystem& Get();
	static UPSWorldSubsystem& Get(const UObject& WorldContextObject);

	/* Delegate for informing row data changed */
	UPROPERTY(BlueprintAssignable, Transient, Category = "C++")
	FCurrentRowDataChanged OnCurrentRowDataChanged;

	/** Returns the data asset that contains all the assets of Progression System game feature.
	 * @see UPSWorldSubsystem::PSDataAssetInternal. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "C++")
	const class UPSDataAsset* GetPSDataAsset() const;

	/** Returns a progression System component reference */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UPSHUDComponent* GetProgressionSystemHUDComponent() const { return PSHUDComponentInternal; }

	/** Returns a current progression row name */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE FName GetCurrentRowName() const { return CurrentRowNameInternal; }

	/** Returns a current progression save game data */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE class UPSSaveGameData* GetCurrentSaveGameData() { return SaveGameDataInternal; }

	/** Returns first save to disk row data */
	UFUNCTION(BlueprintPure, Category = "C++")
	FName GetFirstSaveToDiskRowName() const;

	/** Returns a current save to disk row by name */
	UFUNCTION(BlueprintPure, Category = "C++")
	const  FPSSaveToDiskData& GetCurrentSaveToDiskRowByName() const ;

	/** Returns a current progression row settings data row by name */
	UFUNCTION(BlueprintPure, Category = "C++")
	const FPSRowData& GetCurrentProgressionSettingsRowByName() const;

	/** Set current row of progression system by tag*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCurrentRowByTag(FPlayerTag NewRowPlayerTag);

	/** Set the progression system component */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetHUDComponent(class UPSHUDComponent* MyHUDComponent);

	/** Set the progression system spot component */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void RegisterSpotComponent(class UPSSpotComponent* MyHUDComponent);

	/** Set the progression system spot component */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetCurrentSpotComponent(class UPSSpotComponent* MyHUDComponent);

	/** Saves the progression to the local files */
	UFUNCTION()
	void SaveDataAsync() const;

	/** Removes all saved data of the Progression system and creates a new empty data */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void ResetSaveGameData();

	/** Unlocks all levels of the Progression System */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UnlockAllLevels();

	/** Returns difficultyMultiplier */
	UFUNCTION(BlueprintCallable, Category="C++")
	float GetDifficultyMultiplier();

protected:
	/** Contains all the assets and tweaks of Progression System game feature.
	 * Note: Since Subsystem is code-only, there is config property set in BaseProgressionSystem.ini.
	 * Property is put to subsystem because its instance is created before any other object.
	 * It can't be put to DevelopSettings class because it does work properly for MGF-modules. */
	UPROPERTY(Config, VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Progression System Data Asset"))
	TSoftObjectPtr<const class UPSDataAsset> PSDataAssetInternal;

	/** Progression System component reference*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Progression System HUD Component"))
	TObjectPtr<class UPSHUDComponent> PSHUDComponentInternal = nullptr;

	/** Progression System Array of Spot Components */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Progression System Spot Array"))
	TArray<class UPSSpotComponent*> PSSpotComponentArrayInternal;

	/** Progression System Spot Component reference*/
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Progression System Spot Component"))
	TObjectPtr<class UPSSpotComponent> PSCurrentSpotComponentInternal = nullptr;

	/** Store the current save game instance
	 * Contains the FPSSaveToDiskData which has actual data from save file */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Save Game Data Internal"))
	TObjectPtr<class UPSSaveGameData> SaveGameDataInternal = nullptr;

	/** Store the Progression Settings data instance */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Save Game Instance"))
	TMap<FName, FPSRowData> ProgressionSettingsDataInternal;

	/** Store the current row name */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current Row Name"))
	FName CurrentRowNameInternal;

	/** Array of pool actors handlers which should be released */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Pool Actors Handlers"))
	TArray<FPoolObjectHandle> PoolActorHandlersInternal;

	/** Array of actors spawned */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Spawned Actors Internal"))
	TArray<AActor*> SpawnedStarActorsInternal;

	/** Store the material for dynamic progress material fill for a star actor */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Star Dynamic Progress Material"))
	TObjectPtr<UMaterialInstanceDynamic> StarDynamicProgressMaterial = nullptr;

	/*********************************************************************************************
	* Protected functions
	********************************************************************************************* */
protected:
	/** Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Clears all transient data created by this subsystem. */
	virtual void Deinitialize() override;

	/** Is called when a player character is ready */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void OnCharacterReady(class APlayerCharacter* PlayerCharacter, int32 CharacterID);

	/** Is called when a player has been changed */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void OnPlayerTypeChanged(FPlayerTag PlayerTag);

	/** Called when the current game state was changed. */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState CurrentGameState);

	/** Load game from save file or create a new one (does initial load from data table) */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void LoadGameFromSave();

	/** Set first element as current active */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void SetFirstElementAsCurrent();

	/** Updates the stars actors for a spot */
	UFUNCTION(Blueprintable, Category="C++", meta=(BlueprintProtected))
	void UpdateProgressionActorsForSpot();

	/** Spawn/add the stars actors for a spot */
	UFUNCTION(Blueprintable, Category="C++", meta=(BlueprintProtected))
	void AddProgressionStarActors();

	/**
	 * Dynamically adds Star actors which representing unlocked and locked progression above the character
	 * @param CreatedObjects - Handles of objects from Pool Manager
	 */
	UFUNCTION(BlueprintCallable, Category= "C++")
	void OnTakeActorsFromPoolCompleted(const TArray<FPoolObjectData>& CreatedObjects);

	/** Updates star actor to locked/unlocked according to input amount
	 * @param CreatedData Object received from Pool Manager which contains the reference to Start Widget 
	 * @param AmountOfUnlockedStars The number of stars (unlocked-stars as actors) to be added on top of the character
	 * @param AmountOfLockedStars The number of stars (locked-stars as actors) to be added on top of the character
	 */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void UpdateStarActor(const FPoolObjectData& CreatedData, float AmountOfUnlockedStars, float AmountOfLockedStars);

	/** Triggers when a spot is loaded */
	UFUNCTION(Blueprintable, Category="C++", meta=(BlueprintProtected))
	void OnSpotComponentLoad(class UPSSpotComponent* SpotComponent);
};
