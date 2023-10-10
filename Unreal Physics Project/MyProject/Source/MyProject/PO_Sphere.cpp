// Fill out your copyright notice in the Description page of Project Settings.


#include "PO_Sphere.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
APO_Sphere::APO_Sphere()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Physics Sphere"));
	RootComponent = mSphere;

	mSphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Sphere"));
	mSphereVisual->AttachToComponent(mSphere, FAttachmentTransformRules::KeepRelativeTransform);

	
	
	mRadius = mSphere->GetUnscaledSphereRadius();
 	

}

// Called when the game starts or when spawned
void APO_Sphere::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APO_Sphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Calculate Velocity and displacement
	Velocity += {0.0f, 0.0f, g};
	Displacement += Velocity;


	// After calculations set the actor location to displacement
	SetActorLocation(Displacement * DeltaTime);
}

bool APO_Sphere::CheckForCollision(FVector centreToCentreVector, float otherRadius)
{
	return false;
}

