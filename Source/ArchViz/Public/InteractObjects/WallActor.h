// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface/I_Interact.h"
#include "GameFramework/Actor.h"
#include "WallActor.generated.h"

UCLASS()
class ARCHVIZ_API AWallActor : public AActor, public II_Interact
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWallActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//virtual void Interact(APlayerController* PlayerController) override;
	virtual void Interact_Implementation(APlayerController* PlayerController) override;

	virtual void ExitInteract_Implementation(APlayerController* PlayerController) override;

	
	UFUNCTION(BlueprintCallable, Category = "Wall")
	void ApplyColor(FLinearColor NewColor);


	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* WallMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* WallMaterial;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ColorPaletteWidgetClass;

	UUserWidget* ColorPaletteWidget;

};
