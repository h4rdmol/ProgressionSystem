// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#pragma once

#include "Data/PSTypes.h"
#include "Components/ActorComponent.h"
#include "PSSpotComponent.generated.h"

/**
 * Represents a spot where a character can be selected in the Main Menu.
 * Is added dynamically to the My Skeletal Mesh actors on the level.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROGRESSIONSYSTEMRUNTIME_API UPSSpotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPSSpotComponent();

	/** Returns the Skeletal Mesh of the Bomber character. */
	UFUNCTION(BlueprintPure, Category = "C++")
	class UMySkeletalMeshComponent* GetMySkeletalMeshComponent() const;
	class UMySkeletalMeshComponent& GetMeshChecked() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/** Clears all transient data created by this component. */
	virtual void OnUnregister() override;

	/** Is called when a player has been changed */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void OnPlayerTypeChanged(FPlayerTag PlayerTag);

	/** Locks the player spot when progression for level achieved */
	UFUNCTION(BlueprintCallable, Category= "C++", meta=(BlueprintProtected))
	void ChangeSpotVisibilityStatus();

	/** A Progression System Component */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Progression System HUD Component"))
	TObjectPtr<UPSHUDComponent> PSHUDComponentInternal = nullptr;

	/** A player skeletal mesh actor */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Spot On Level"))
	TObjectPtr<UMySkeletalMeshComponent> PlayerSpotOnLevelInternal = nullptr;

	/** Current Progression  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Current Progression Row Data"))
	FPSRowData CurrentProgressionRowDataInternal = FPSRowData::EmptyData;

	/** Store the save game instance */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Save Game Instance"))
	TObjectPtr<class UPSSaveGameData> SaveGameInstanceInternal = nullptr;
};
