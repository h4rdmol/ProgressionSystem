// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "PSCSaveGame.h"

const FString& UPSCSaveGame::GetSaveSlotName()
{
	static const FString SaveSlotName = StaticClass()->GetName();
	return SaveSlotName;
}
