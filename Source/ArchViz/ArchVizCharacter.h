// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ArchVizCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AArchVizCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	// Top-down camera input actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|TopDownCamera", meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|TopDownCamera", meta = (AllowPrivateAccess = "true"))
	UInputAction* ZoomCameraAction;
	
public:
	AArchVizCharacter();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:
	// APawn interface
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface
	//virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	AActor* LastHitActor;

	// Input binding
	void HandleInteract();

	//virtual void BeginPlay() override;

	//virtual void Tick(float DeltaTime) override;

	void LineTrace();
	//void Interact();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceDistance = 1200.f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_ShowInteract();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_HideInteract();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_ShowCrosshair();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_HideCrosshair();


	void Interact(); // toggle interaction

	// Camera for viewing walls
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* WallCamera;

	// State to track whether we are in wall view mode
	bool bInteracting;

	// Reference to last focused wall
	AActor* CurrentInteractingActor;

	bool bUsingTopDownCamera = false;
	AActor* DefaultViewTarget = nullptr;
	class ATopDownCameraActor* TopDownCamera = nullptr;

	// Forward input to top-down camera
	void RotateTopDownCamera(const FInputActionValue& Value);
	void ZoomTopDownCamera(const FInputActionValue& Value);

	// Function to toggle camera
	void ToggleCameraView();
};

