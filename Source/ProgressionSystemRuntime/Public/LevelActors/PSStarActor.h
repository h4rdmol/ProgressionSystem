// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#pragma once

#include "GameFramework/Actor.h"
#include "PSStarActor.generated.h"

UCLASS()
class PROGRESSIONSYSTEMRUNTIME_API APSStarActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APSStarActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Function called every frame on this Actor */
	virtual void Tick(float DeltaTime) override;

	/** Stores the starting time to hide stars in the main menu when cinematic started */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to hide stars"))
	float StartTimeHideStarsInternal = 0.0f;

	/** Stores the starting time to animate stars in main menu */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Starting time to animate stars in menu"))
	float StartTimeMenuStarsInternal = 0.0f;

	/** When a local character load finished */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void OnLocalCharacterReady(APlayerCharacter* Character, int32 CharacterID);

	/** Called when the current player was changed */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnPlayerTypeChanged(FPlayerTag PlayerTag);

	/** Called when the current game state was changed */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void OnGameStateChanged(ECurrentGameState GameState);

	/** Is called when any cinematic started to play in the main menu */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void OnAnyCinematicStarted(const UObject* LevelSequence, const UObject* FromInstigator);

	/** Is called to set a start time for animations */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void SetAnimationStartTime(float& StartTime);

	/** Called to initialize the Star animation in the main menu */
	UFUNCTION(BlueprintCallable, Category = "C++", meta = (BlueprintProtected))
	void InitStarMenuAnimation();

	/** Hiding stars with animation in main menu when cinematic is start to play */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void TryPlayHideStarAnimation();

	/** Menu stars with animation in main menu idle */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void TryPlayMenuStarAnimation();

	/** Playing star animation */
	UFUNCTION(BlueprintCallable, Category= "C++", meta = (BlueprintProtected))
	void TryPlayStarAnimation(float& StartTime, UCurveTable* AnimationCurveTable);
};
