// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"   // ? Add this
#include "Camera/CameraComponent.h"   
#include "TopDownCameraActor.generated.h"

UCLASS()
class ARCHVIZ_API ATopDownCameraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATopDownCameraActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* CameraPivot;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float RotationSpeed = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinZoom = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxZoom = 2000.f;


	void RotateCamera(float Value);
	void ZoomCamera(float Value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FRotator CurrentRotation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
