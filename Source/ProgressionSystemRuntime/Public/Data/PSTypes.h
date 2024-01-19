﻿// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#pragma once

#include "Bomber.h"
#include "Structures/PlayerTag.h"
#include "PSTypes.generated.h"

/**
 * Basic structure for all save data information regarding the progression.
 * Same structure will reflected in the save file. Initial load performed based on the data in the DT Table 
 */
USTRUCT(BlueprintType)
struct FPSRowData : public FTableRowBase
{
	GENERATED_BODY()

	static const FPSRowData EmptyData;

	/** Default constructor. */
	FPSRowData() = default;

	/** Stores the value of the map for progression system component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C++")
	ELevelType Map = ELevelType::None;

	/** Contains the character player tag used in the save/load progression system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C++")
	FPlayerTag Character = FPlayerTag::None;

	/** Current progression for each level  */
	UPROPERTY()
	int32 CurrentLevelProgression = 0;

	/** Required about of points to unlock level  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C++")
	int32 PointsToUnlock = 0;

	/** Amount of points to gain after win */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C++")
	int32 WinReward = 0;

	/** Amount of points to gain after draw */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C++")
	int32 DrawReward = 0;

	/** Amount of points to gain after loss */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="C++")
	int32 LossReward = 0;

	/** Defines if level is locked or not */
	UPROPERTY()
	bool IsLevelLocked = true;
};
