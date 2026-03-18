// Fill out your copyright notice in the Description page of Project Settings.
#include "CameraActor/TopDownCameraActor.h"
#include "Camera/CameraComponent.h"


// Sets default values
ATopDownCameraActor::ATopDownCameraActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    CameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPivot"));
    RootComponent = CameraPivot;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    //RootComponent = SpringArm;
    SpringArm->SetupAttachment(CameraPivot);
    SpringArm->TargetArmLength = 1000.f;
    SpringArm->bDoCollisionTest = false;
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f)); // Slight tilt, not full -90

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    CurrentRotation = SpringArm->GetComponentRotation();

}

// Called when the game starts or when spawned
void ATopDownCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATopDownCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATopDownCameraActor::RotateCamera(float Value)
{
    if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
    {
        FRotator DeltaRotation(0.f, Value * RotationSpeed * GetWorld()->GetDeltaSeconds(), 0.f);
        CameraPivot->AddLocalRotation(DeltaRotation);
    }
}

void ATopDownCameraActor::ZoomCamera(float Value)
{
    if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
    {
        float NewLength = FMath::Clamp(SpringArm->TargetArmLength + Value * ZoomSpeed * GetWorld()->GetDeltaSeconds(), MinZoom, MaxZoom);
        SpringArm->TargetArmLength = NewLength;
    }
}