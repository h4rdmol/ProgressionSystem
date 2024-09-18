// Copyright (c) Valerii Rotermel and Yevhenii Selivanov


#include "LevelActors/PSStarActor.h"

#include "Components/StaticMeshComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Data/PSTypes.h"
#include "Data/PSWorldSubsystem.h"
#include "GameFramework/MyGameStateBase.h"
#include "LevelActors/PlayerCharacter.h"
#include "MyUtilsLibraries/GameplayUtilsLibrary.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSStarActor)

// Sets default values
APSStarActor::APSStarActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Called when the game starts or when spawned
void APSStarActor::BeginPlay()
{
	Super::BeginPlay();

	// Listen to hande when local character is ready 
	BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnLocalCharacterReady);

	// Listen to handle input for each game state
	BIND_ON_GAME_STATE_CHANGED(this, ThisClass::OnGameStateChanged);
}

// Function called every frame on this Actor
void APSStarActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// play only if cinematic is started
	// else play menu animation
	if (StartTimeHideStarsInternal)
	{
		TryPlayHideStarAnimation();
	}
	else if (StartTimeMenuStarsInternal && UMyBlueprintFunctionLibrary::GetMyGameState()->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		TryPlayMenuStarAnimation();
	}
}

// When a local character load finished
void APSStarActor::OnLocalCharacterReady(APlayerCharacter* Character, int32 CharacterID)
{
	AMyPlayerController* LocalPC = Character ? Character->GetController<AMyPlayerController>() : nullptr;
	if (ensureMsgf(LocalPC, TEXT("ASSERT: [%i] %hs:\n'LocalPC' is null!"), __LINE__, __FUNCTION__))
	{
		LocalPC->OnAnyCinematicStarted.AddDynamic(this, &APSStarActor::OnAnyCinematicStarted);
		Character->OnPlayerTypeChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerTypeChanged);
	}
}


// Called when the current  player was changed
void APSStarActor::OnPlayerTypeChanged(FPlayerTag PlayerTag)
{
	InitStarMenuAnimation();
}

// Called when the current game state was changed
void APSStarActor::OnGameStateChanged(ECurrentGameState GameState)
{
	InitStarMenuAnimation();
}

// Is called when any cinematic started
void APSStarActor::OnAnyCinematicStarted(const UObject* LevelSequence, const UObject* FromInstigator)
{
	SetAnimationStartTime(StartTimeHideStarsInternal);
	TryPlayHideStarAnimation();
}

// Is called to set a start time for animations
void APSStarActor::SetAnimationStartTime(float& StartTime)
{
	const UWorld* World = GetWorld();
	check(World);

	StartTime = World->GetTimeSeconds();

	// Enable tick for the actor
	SetActorTickEnabled(true);
}

// Called to initialize the Star menu animation
void APSStarActor::InitStarMenuAnimation()
{
	if (UMyBlueprintFunctionLibrary::GetMyGameState()->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		SetAnimationStartTime(StartTimeMenuStarsInternal);
		TryPlayMenuStarAnimation();
	}
	else
	{
		SetActorTickEnabled(false);
		StartTimeMenuStarsInternal = 0.f;
	}
}

// Hiding stars with animation
void APSStarActor::TryPlayHideStarAnimation()
{
	const FPSRowData& CurrentRow = UPSWorldSubsystem::Get().GetCurrentRow();
	TryPlayStarAnimation(StartTimeHideStarsInternal, CurrentRow.HideStarsAnimation);
}

// Menu stars with animation 
void APSStarActor::TryPlayMenuStarAnimation()
{
	const FPSRowData& CurrentRow = UPSWorldSubsystem::Get().GetCurrentRow();
	TryPlayStarAnimation(StartTimeMenuStarsInternal, CurrentRow.MenuStarsAnimation);
}

// Playing star animation
void APSStarActor::TryPlayStarAnimation(float& StartTime, UCurveTable* AnimationCurveTable)
{
	if (!StartTime || !AnimationCurveTable)
	{
		SetActorTickEnabled(false);
		StartTime = 0.f;
		return;
	}

	const float SecondsSinceStart = GetWorld()->GetTimeSeconds() - StartTime;

	const bool bIsFinished = !UGameplayUtilsLibrary::ApplyTransformFromCurveTable(this, AnimationCurveTable, SecondsSinceStart);
	if (bIsFinished)
	{
		// if it's looped menu star animation then reset
		// if it's one-time cinematic animation - disable tick and stop
		if (AnimationCurveTable == UPSWorldSubsystem::Get().GetCurrentRow().MenuStarsAnimation)
		{
			StartTime = 0.f;
		}
		else
		{
			SetActorTickEnabled(false);
			StartTime = 0.f;
		}
	}
}
