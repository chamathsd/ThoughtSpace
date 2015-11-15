// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "ThoughtSpaceHUD.generated.h"

UCLASS()
class AThoughtSpaceHUD : public AHUD
{
	GENERATED_BODY()

public:
	AThoughtSpaceHUD(const FObjectInitializer& ObjectInitializer);

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

