// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractObjects/WallActor.h"
#include "WallColorWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARCHVIZ_API UWallColorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Exposed wall reference
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	AWallActor* TargetWall;
	
};
