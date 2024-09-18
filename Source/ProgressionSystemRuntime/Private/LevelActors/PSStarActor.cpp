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
	if (StartTimeHideStarsInternal)
	{
		TryPlayHideStarAnimation();
	}
	if (UMyBlueprintFunctionLibrary::GetMyGameState()->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		TryPlayMenuStarAnimation(DeltaTime);
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
	const UWorld* World = GetWorld();
	check(World);

	StartTimeHideStarsInternal = World->GetTimeSeconds();

	// Enable tick and process this frame
	SetActorTickEnabled(true);
	TryPlayHideStarAnimation();
}

// Called to initialize the Star menu animation
void APSStarActor::InitStarMenuAnimation()
{
	if (UMyBlueprintFunctionLibrary::GetMyGameState()->GetCurrentGameState() == ECurrentGameState::Menu)
	{
		// Enable tick and start main menu animation
		SetActorTickEnabled(true);
		TryPlayMenuStarAnimation(0);
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

// Hiding stars with animation
void APSStarActor::TryPlayHideStarAnimation()
{
	if (!StartTimeHideStarsInternal)
	{
		return;
	}

	const FPSRowData& CurrentRow = UPSWorldSubsystem::Get().GetCurrentRow();
	if (!CurrentRow.MenuStarsAnimation)
	{
		SetActorTickEnabled(false);
		StartTimeHideStarsInternal = 0.f;
		return;
	}

	const float SecondsSinceStart = GetWorld()->GetTimeSeconds() - StartTimeHideStarsInternal;

	const bool bIsFinished = !UGameplayUtilsLibrary::ApplyTransformFromCurveTable(this, CurrentRow.HideStarsAnimation, SecondsSinceStart);
	if (bIsFinished)
	{
		SetActorTickEnabled(false);
		StartTimeHideStarsInternal = 0.f;
	}
}

// Menu stars with animation 
void APSStarActor::TryPlayMenuStarAnimation(float DeltaTime)
{
	const FPSRowData& CurrentRow = UPSWorldSubsystem::Get().GetCurrentRow();
	if (!CurrentRow.MenuStarsAnimation)
	{
		SetActorTickEnabled(false);
		return;
	}

	FCurveTableRowHandle Handle;
	Handle.CurveTable = CurrentRow.MenuStarsAnimation;
	Handle.RowName = FName("RotationYaw");

	const FRealCurve* Curve = CurrentRow.MenuStarsAnimation ? Handle.CurveTable->FindCurve(Handle.RowName, TEXT("Rotation")) : nullptr;
	if (!Curve)
	{
		return;
	}

	float MinTime = 0.f;
	float MaxTime = 0.f;
	Curve->GetTimeRange(MinTime, MaxTime);

	// Update elapsed time
	ElapsedTime += DeltaTime;

	UGameplayUtilsLibrary::ApplyTransformFromCurveTable(this, CurrentRow.MenuStarsAnimation, ElapsedTime);

	if (ElapsedTime > MaxTime)
	{
		ElapsedTime = 0.f;
	}
}
