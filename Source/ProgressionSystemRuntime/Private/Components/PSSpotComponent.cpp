// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "Components/PSSpotComponent.h"

#include "Components/PSHUDComponent.h"
#include "Data/PSWorldSubsystem.h"
#include "Data/PSSaveGameData.h"
#include "Components/MySkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LevelActors/PlayerCharacter.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Subsystems/GlobalEventsSubsystem.h"
#include "UtilityLibraries/MyBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSSpotComponent)

// Sets default values for this component's properties
UPSSpotComponent::UPSSpotComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Called when the game starts
void UPSSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	// Ensure the component's mesh is properly assigned and not null.
	PlayerSpotOnLevelInternal = GetMeshChecked();

	UPSWorldSubsystem::Get().OnCurrentRowDataChanged.AddDynamic(this, &ThisClass::OnPlayerTypeChanged);
	// Subscribe events on player type changed and Character spawned
	BIND_ON_LOCAL_CHARACTER_READY(this, ThisClass::OnCharacterReady);

	// Ensure the save game data is properly loaded and not null.
	SaveGameInstanceInternal = UPSWorldSubsystem::Get().GetCurrentSaveGameData();
	checkf(SaveGameInstanceInternal, TEXT("ERROR: 'SaveGameInstanceInternal' is null"));

	ChangeSpotVisibilityStatus();

	// Save reference of this component to the world subsystem
	UPSWorldSubsystem::Get().RegisterSpotComponent(this);
}

// Clears all transient data created by this component.
void UPSSpotComponent::OnUnregister()
{
	Super::OnUnregister();
	PlayerSpotOnLevelInternal = nullptr;
}

// Returns the Skeletal Mesh of the Bomber character
UMySkeletalMeshComponent* UPSSpotComponent::GetMySkeletalMeshComponent() const
{
	return GetOwner()->FindComponentByClass<UMySkeletalMeshComponent>();
}

// Returns the Skeletal Mesh of the Bomber character
UMySkeletalMeshComponent& UPSSpotComponent::GetMeshChecked() const
{
	UMySkeletalMeshComponent* Mesh = GetMySkeletalMeshComponent();
	checkf(Mesh, TEXT("'Mesh' is nullptr, can not get mesh for '%s' spot."), *GetNameSafe(this));
	return *Mesh;
}

// Is called when a player has been changed
void UPSSpotComponent::OnPlayerTypeChanged(FPlayerTag PlayerTag)
{
	UMySkeletalMeshComponent& Mesh = GetMeshChecked();
	if (Mesh.GetPlayerTag() == PlayerTag)
	{
		PlayerSpotOnLevelInternal = Mesh;
		ChangeSpotVisibilityStatus();

		// Save reference of this component to the world subsystem
		UPSWorldSubsystem::Get().SetCurrentSpotComponent(this);
	}
}

//  Is called when a player has been changed 
void UPSSpotComponent::OnCharacterReady(APlayerCharacter* PlayerCharacter, int32 CharacterID)
{
	if (PlayerCharacter->GetPlayerTag() == GetMeshChecked().GetPlayerTag())
	{
		PlayerSpotOnLevelInternal = GetMeshChecked();
		OnSpotComponentReady.Broadcast(this);
	}
}

// Locks the player spot when progression for level achieved 
void UPSSpotComponent::ChangeSpotVisibilityStatus()
{
	// Locks and unlocks the spot depends on the current level progression status
	if (PlayerSpotOnLevelInternal)
	{
		PlayerSpotOnLevelInternal->SetActive(!SaveGameInstanceInternal->GetCurrentRow().IsLevelLocked);
	}
}
