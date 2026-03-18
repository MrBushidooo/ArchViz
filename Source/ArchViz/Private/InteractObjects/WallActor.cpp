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
        
        WallMaterial->SetVectorParameterValue(TEXT("BaseColor"), NewColor);
    }
}

void AWallActor::Interact_Implementation(APlayerController* PlayerController)
{
    if (!PlayerController) return;

    FVector WallLocation = GetActorLocation();
  
    FVector WallForward = GetActorForwardVector(); 

    float Distance = 400.f;
   
    FVector CameraPos = WallLocation - FVector(Distance, 0.f, 0.f);

    FVector Origin, Extent;
    GetActorBounds(true, Origin, Extent);
    CameraPos.Z = Origin.Z + Extent.Z * 0.5f; 
    CameraPos.Z -= 50.f; 
    FRotator CameraRot = UKismetMathLibrary::FindLookAtRotation(CameraPos, Origin);
    FTransform CameraTransform(CameraRot, CameraPos);
    ACameraActor* TempCamera = PlayerController->GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(),
        CameraTransform
    );

    PlayerController->SetViewTargetWithBlend(TempCamera, 0.5f);
   
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

    UE_LOG(LogTemp, Log, TEXT("WallActor: Exited interaction mode."));
}
