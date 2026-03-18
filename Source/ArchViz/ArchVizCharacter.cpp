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

		//Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AArchVizCharacter::HandleInteract);

		//Toggle camera
		PlayerInputComponent->BindAction("ToggleCamera", IE_Pressed, this, &AArchVizCharacter::ToggleCameraView);

		//Bind topdown camera input
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
			BP_ShowInteract();
		}

		else {
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
		
		if (CurrentInteractingActor && CurrentInteractingActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass()))
		{
	
		}

		bInteracting = false;
		CurrentInteractingActor = nullptr;


		
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
	else
	{
		
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
		-1,             
		2.0f,           
		FColor::Green,   
		TEXT("E Pressed!")  
	);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	
	if (bInteracting)
	{
		bInteracting = false;
		BP_ShowCrosshair();


		PC->SetViewTargetWithBlend(this, 1.0f);


		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;

		if (LastHitActor && LastHitActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass()))
		{
			
			II_Interact::Execute_ExitInteract(LastHitActor, PC);
		}


		return;
	}

	if (LastHitActor && LastHitActor->GetClass()->ImplementsInterface(UI_Interact::StaticClass()))
	{
		bInteracting = true;
		BP_HideInteract();
		BP_HideCrosshair();

	
		II_Interact::Execute_Interact(LastHitActor, PC);
	}
}

void AArchVizCharacter::RotateTopDownCamera(const FInputActionValue& Value)
{
	if (bUsingTopDownCamera && TopDownCamera)
	{
		float AxisValue = Value.Get<float>();
	
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



void AArchVizCharacter::ToggleCameraView()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;


	if (!DefaultViewTarget)
	{
		DefaultViewTarget = this;
	}

	
	if (!TopDownCamera)
	{
		TopDownCamera = Cast<ATopDownCameraActor>(UGameplayStatics::GetActorOfClass(this, ATopDownCameraActor::StaticClass()));
		if (!TopDownCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("No TopDownCameraActor found in level!"));
			return;
		}
	}

	
	bUsingTopDownCamera = !bUsingTopDownCamera;
	BP_HideInteract();

	
	PC->SetViewTargetWithBlend(bUsingTopDownCamera ? TopDownCamera : DefaultViewTarget, 1.0f);

	
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

