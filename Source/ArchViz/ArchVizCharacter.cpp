// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArchVizCharacter.h"
#include "Interface/I_Interact.h"
#include "UMG.h"
#include "Public/CameraActor/TopDownCameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SlateWrapperTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/WidgetComponent.h"
#include "ArchVizProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AArchVizCharacter

AArchVizCharacter::AArchVizCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	WallCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("WallCamera"));
	WallCamera->SetupAttachment(GetCapsuleComponent()); // Not attached to head, free to move
	WallCamera->bAutoActivate = false; // keep it disabled until needed

	bInteracting = false;

	CurrentInteractingActor = nullptr;

	LastHitActor = nullptr;

	PrimaryActorTick.bCanEverTick = true;

}



void AArchVizCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Call your line trace function every frame
	LineTrace();
}

//////////////////////////////////////////////////////////////////////////// Input

void AArchVizCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}



void AArchVizCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AArchVizCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AArchVizCharacter::Look);

		// ? Interact (E key, bound via InteractAction asset in editor)
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AArchVizCharacter::HandleInteract);

		// Toggle camera
		PlayerInputComponent->BindAction("ToggleCamera", IE_Pressed, this, &AArchVizCharacter::ToggleCameraView);

		// Bind top-down camera input
		EnhancedInputComponent->BindAction(RotateCameraAction, ETriggerEvent::Triggered, this, &AArchVizCharacter::RotateTopDownCamera);
		EnhancedInputComponent->BindAction(ZoomCameraAction, ETriggerEvent::Triggered, this, &AArchVizCharacter::ZoomTopDownCamera);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AArchVizCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AArchVizCharacter::Look(const FInputActionValue& Value)
{
	if (bUsingTopDownCamera) return; // ignore look input in top-down
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AArchVizCharacter::LineTrace() {

	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FRotator Rot = FirstPersonCameraComponent->GetComponentRotation();

	FVector End = Start + (Rot.Vector() * TraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	AActor* HitActor = nullptr;

	if (bHit) {
		HitActor = Hit.GetActor();
	}

	else {
		HitActor = nullptr;
	}

	if (HitActor != LastHitActor) {

		LastHitActor = HitActor;

		if (HitActor && HitActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass())) {

			//UI focus logic
			BP_ShowInteract();
		}

		else {

			//UI unfocus logic
			BP_HideInteract();
		}

	}

	//FColor LineColor = (HitActor ? FColor::Green : FColor::Red);
	//DrawDebugLine(GetWorld(), Start, End, LineColor, false, 0.1f, 0, 1.0f);

}

void AArchVizCharacter::Interact()
{
	if (bInteracting)
	{
		// Exit interaction
		if (CurrentInteractingActor && CurrentInteractingActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass()))
		{
			// Optional: you can add an "ExitInteract" to your interface if needed
			// For now, just reset state here
		}

		bInteracting = false;
		CurrentInteractingActor = nullptr;


		// Restore game input
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
	else
	{
		// Enter interaction
		if (LastHitActor && LastHitActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass()))
		{
			BP_HideInteract();
			CurrentInteractingActor = LastHitActor;
			bInteracting = true;

			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				II_Interact::Execute_Interact(CurrentInteractingActor, PC);
			}
		}
	}
}

void AArchVizCharacter::HandleInteract()
{
	GEngine->AddOnScreenDebugMessage(
		-1,              // key (-1 = always add a new one instead of updating)
		2.0f,            // time in seconds
		FColor::Green,   // text color
		TEXT("E Pressed!")  // message
	);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// If already interacting ? exit
	if (bInteracting)
	{
		bInteracting = false;
		BP_ShowCrosshair();

		// Restore camera to player
		PC->SetViewTargetWithBlend(this, 1.0f);

		// Restore game input
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;

		// (Optional) Tell the actor to close its UI (if needed)
		if (LastHitActor && LastHitActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass()))
		{
			// If your interactable has a "Close" or "Exit" method, call it here
			// Example: II_Interact::Execute_OnExit(LastHitActor, PC);
			II_Interact::Execute_ExitInteract(LastHitActor, PC);
		}


		return;
	}

	// If not interacting and we have a valid actor
	if (LastHitActor && LastHitActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass()))
	{
		bInteracting = true;
		BP_HideInteract();
		BP_HideCrosshair();

		// Call the actor’s interact function
		II_Interact::Execute_Interact(LastHitActor, PC);
	}
}

void AArchVizCharacter::RotateTopDownCamera(const FInputActionValue& Value)
{
	if (bUsingTopDownCamera && TopDownCamera)
	{
		float AxisValue = Value.Get<float>();
		//if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
		{
			UE_LOG(LogTemp, Warning, TEXT("yes please Rotate input: %f"), AxisValue);
			TopDownCamera->RotateCamera(AxisValue);
		}
	}
}

void AArchVizCharacter::ZoomTopDownCamera(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	if (bUsingTopDownCamera && TopDownCamera)
	{
		TopDownCamera->ZoomCamera(AxisValue);
	}
}

/*void AArchVizCharacter::ToggleCameraView()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// Save player as default view target
	if (!DefaultViewTarget)
	{
		DefaultViewTarget = this;
	}

	// Find TopDownCamera actor in level
	if (!TopDownCamera)
	{
		TopDownCamera = Cast<ATopDownCameraActor>(UGameplayStatics::GetActorOfClass(this, ATopDownCameraActor::StaticClass()));
		if (!TopDownCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("No TopDownCameraActor found in level!"));
			return;
		}
	}

	// Toggle camera
	if (!bUsingTopDownCamera)
	{
		// Switch to top-down camera
		PC->SetViewTargetWithBlend(TopDownCamera, 1.0f); // smooth blend
		bUsingTopDownCamera = true;

		// Optional: enable mouse cursor/input for orbit
		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		// Switch back to player camera
		PC->SetViewTargetWithBlend(DefaultViewTarget, 1.0f);
		bUsingTopDownCamera = false;

		// Restore normal player input
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}*/

/*void AArchVizCharacter::ToggleCameraView()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !TopDownCamera) return;

	bUsingTopDownCamera = !bUsingTopDownCamera;

	PC->SetViewTargetWithBlend(bUsingTopDownCamera ? TopDownCamera : this, 0.5f);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (bUsingTopDownCamera)
			Subsystem->AddMappingContext(TopDownCameraMappingContext, 1);
		else
			Subsystem->RemoveMappingContext(TopDownCameraMappingContext);
	}

	PC->bShowMouseCursor = bUsingTopDownCamera;
}*/

void AArchVizCharacter::ToggleCameraView()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// Save the player as default view target (first time only)
	if (!DefaultViewTarget)
	{
		DefaultViewTarget = this;
	}

	// Find TopDownCamera actor in level if not assigned
	if (!TopDownCamera)
	{
		TopDownCamera = Cast<ATopDownCameraActor>(UGameplayStatics::GetActorOfClass(this, ATopDownCameraActor::StaticClass()));
		if (!TopDownCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("No TopDownCameraActor found in level!"));
			return;
		}
	}

	// Toggle camera
	bUsingTopDownCamera = !bUsingTopDownCamera;
	BP_HideInteract();

	// Switch view target smoothly
	PC->SetViewTargetWithBlend(bUsingTopDownCamera ? TopDownCamera : DefaultViewTarget, 1.0f);

	// Set input mode & cursor visibility
	if (bUsingTopDownCamera)
	{
		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

