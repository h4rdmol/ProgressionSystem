// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "PSCWorldSubsystem.h"

#include "MyUtilsLibraries/UtilsLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PSCWorldSubsystem)

// Returns this Subsystem, is checked and wil crash if can't be obtained
UPSCWorldSubsystem& UPSCWorldSubsystem::Get()
{
	const UWorld* World = UUtilsLibrary::GetPlayWorld();
	checkf(World, TEXT("%s: 'World' is null"), *FString(__FUNCTION__));
	UPSCWorldSubsystem* ThisSubsystem = World->GetSubsystem<ThisClass>();
	checkf(ThisSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

// Returns this Subsystem, is checked and wil crash if can't be obtained
UPSCWorldSubsystem& UPSCWorldSubsystem::Get(const UObject& WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObjectChecked(&WorldContextObject);
	checkf(World, TEXT("%s: 'World' is null"), *FString(__FUNCTION__));
	UPSCWorldSubsystem* ThisSubsystem = World->GetSubsystem<ThisClass>();
	checkf(ThisSubsystem, TEXT("%s: 'SoundsSubsystem' is null"), *FString(__FUNCTION__));
	return *ThisSubsystem;
}

void UPSCWorldSubsystem::SetProgressionSystemComponent(UPSHUDComponent* MyProgressionSystemComponent)
{
	checkf(MyProgressionSystemComponent, TEXT("%s: My progression system component is null"), *FString(__FUNCTION__));
	ProgressionSystemComponentInternal = MyProgressionSystemComponent;
}
