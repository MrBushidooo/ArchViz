// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/InteractObjects/WallActor.h"
#include "Camera/CameraActor.h"
#include "UI/WallColorWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"



// Sets default values
AWallActor::AWallActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;


}

// Called when the game starts or when spawned
void AWallActor::BeginPlay()
{
	Super::BeginPlay();

    if (WallMesh)
    {
        UMaterialInterface* BaseMat = WallMesh->GetMaterial(0);
        if (BaseMat)
        {
            WallMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
            WallMesh->SetMaterial(0, WallMaterial);
        }
    }
	
}

// Called every frame
void AWallActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWallActor::ApplyColor(FLinearColor NewColor)
{
    if (WallMaterial)
    {
        // Assumes your material has a VectorParameter named "BaseColor"
        WallMaterial->SetVectorParameterValue(TEXT("BaseColor"), NewColor);
    }
}

void AWallActor::Interact_Implementation(APlayerController* PlayerController)
{
    if (!PlayerController) return;

    // Wall center
    FVector WallLocation = GetActorLocation();

    // Forward direction of the wall
    FVector WallForward = GetActorForwardVector(); // with yaw 90, this points along +Y

    // Distance from wall to place camera
    float Distance = 400.f;

    // Place camera in front of wall (opposite of forward)
    //FVector CameraPos = WallLocation + WallForward * Distance;
    FVector CameraPos = WallLocation - FVector(Distance, 0.f, 0.f);

    // Raise the camera slightly to center it vertically on the wall
    FVector Origin, Extent;
    GetActorBounds(true, Origin, Extent);
    CameraPos.Z = Origin.Z + Extent.Z * 0.5f; // center vertically

    // Decrease the height a bit (move camera down)
    CameraPos.Z -= 50.f; // tweak this value as needed

    // Make camera look at wall center
    FRotator CameraRot = UKismetMathLibrary::FindLookAtRotation(CameraPos, Origin);

    // Spawn a temporary camera actor
    FTransform CameraTransform(CameraRot, CameraPos);
    ACameraActor* TempCamera = PlayerController->GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(),
        CameraTransform
    );

    // Smoothly switch view
    PlayerController->SetViewTargetWithBlend(TempCamera, 0.5f);

    // Switch to UI mode
    /*PlayerController->SetInputMode(FInputModeUIOnly());
    PlayerController->bShowMouseCursor = true;*/

    // --- Switch input to Game+UI so E still works ---
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputMode);
    PlayerController->bShowMouseCursor = true;

    if (ColorPaletteWidgetClass)
    {
        UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, ColorPaletteWidgetClass);
        if (Widget)
        {
            if (UWallColorWidget* WallColorWidget = Cast<UWallColorWidget>(Widget))
            {
                WallColorWidget->TargetWall = this;
            }

            Widget->AddToViewport();
            ColorPaletteWidget = Widget;
        }
    }
}

void AWallActor::ExitInteract_Implementation(APlayerController* PlayerController)
{
    if (ColorPaletteWidget)
    {
        ColorPaletteWidget->RemoveFromParent();
        ColorPaletteWidget = nullptr;
    }

    // If you spawn a temporary camera, clean it up here as well
    /*
    if (TempCamera && TempCamera->IsValidLowLevel())
    {
        TempCamera->Destroy();
        TempCamera = nullptr;
    }
    */

    UE_LOG(LogTemp, Log, TEXT("WallActor: Exited interaction mode."));
}