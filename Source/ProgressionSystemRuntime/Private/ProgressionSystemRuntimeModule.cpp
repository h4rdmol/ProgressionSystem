﻿// Copyright (c) Valerii Rotermel & Yevhenii Selivanov

#include "ProgressionSystemRuntimeModule.h"

#define LOCTEXT_NAMESPACE "FProgressionSystemRuntimeModule"

void FProgressionSystemRuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory;
	// the exact timing is specified in the .uplugin file per-module
}

void FProgressionSystemRuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.
	// For modules that support dynamic reloading, we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FProgressionSystemRuntimeModule, ProgressionSystemRuntime)
