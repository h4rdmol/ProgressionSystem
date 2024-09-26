// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#pragma once

#include "GameFramework/Actor.h"
#include "PSStarActor.generated.h"

enum class ECurrentGameState : uint8;

UCLASS()
class PROGRESSIONSYSTEMRUNTIME_API APSStarActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APSStarActor();

	/** Helper function that plays any given star animation from various places. */
	UFUNCTION(BlueprintCallable, Category= "C++")
	bool TryPlayStarAnimation(UPARAM(ref) float& StartTimeRef, class UCurveTable* AnimationCurveTable);

	/** Set the start time for hiding stars in the main menu */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetStartTimeHideStars();

	/** Set the start time for main menu stars animation */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void SetStartTimeMenuStars();

	/** Automatically set the transform and location of actor 
	 * when a Star actor is initialized
	* @param PreviousActorLocationRef Previous star actor location reference
	*/
	UFUNCTION(BlueprintCallable, Category = "C++")
	void OnInitialized(FVector& PreviousActorLocationRef);

	/** Updates star actors Mesh material to the Locked Star, Unlocked or partially achieved
	 * 0 - locked star material
	 * 1 - unlocked star material
	 * between 0-1 - partially unlocked material (dynamic) e.g. 0.5  
	 * @param StarDynamicProgressMaterial a Dynamic fill material of a star (e.g. 0.5)
	 * @param AmountOfStars The number of stars to be added on top of the character
	 * @param bIsLockedStar Defines whether a star is locked or unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "C++")
	void UpdateStarActorMeshMaterial(UMaterialInstanceDynamic* StarDynamicProgressMaterial, float AmountOfStars, bool bIsLockedStar);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Function called every frame on this Actor */
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditInstanceOnly, Category = "C++", meta = (DisplayName = "Star static mesh component"))
	TObjectPtr<class UStaticMeshComponent> StarMeshComponent = nullptr;

	/** Stores the starting time to hide stars in the main menu when cinematic started */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to hide stars"))
	float StartTimeHideStarsInternal = 0.0f;

	/** Stores the starting time to animate stars in main menu */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to animate stars in menu"))
	float StartTimeMenuStarsInternal = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Star Material Slot Divisor"))
	FName StarMaterialSlotName = TEXT("Percentage2");

	// Temporary used to tweak the stars with bad UV  to look as expected
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "C++", meta = (BlueprintProtected, DisplayName = "Star Material Fractional Divisor Temporarry"))
	float StarMaterialFractionalDivisor = 3.f;

	/** When a local character load finished */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void OnLocalCharacterReady(class APlayerCharacter* Character, int32 CharacterID);

	/** Called when the current game state was changed */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState GameState);

	/** Is called when any cinematic started to play in the main menu */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void OnAnyCinematicStarted(const UObject* LevelSequence, const UObject* FromInstigator);

	/** Hiding stars with animation in main menu when cinematic is start to play */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void TryPlayHideStarAnimation();

	/** Menu stars with animation in main menu idle */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void TryPlayMenuStarAnimation();
};
