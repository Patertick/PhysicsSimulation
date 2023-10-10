// Fill out your copyright notice in the Description page of Project Settings.


#include "PO_Sphere.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include <String>

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
	Displacement = GetActorLocation();
	//Velocity = InitialVelocity;
}

// Called every frame
void APO_Sphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Calculate Velocity and displacement
	if (isMovingSphere)
	{
		Velocity += Acceleration;
		Displacement += Velocity;


		// After calculations set the actor location to displacement
		SetActorLocation(Displacement);
	}
}


bool APO_Sphere::CheckForCollision(FVector centreToCentreVector, float otherRadius)
{
	if (isMovingSphere)
	{
		return false;
	}
	//define V and |V|
	float magnitudeV = Velocity.Size();
	FString floatStr = FString::SanitizeFloat(magnitudeV) + " Magnitude V";
	GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::Yellow, *floatStr);
	FVector V = Velocity;
	GEngine->AddOnScreenDebugMessage(2, 15.0f, FColor::Yellow, V.ToString() + " vector V");
	//define A and |A|
	float magnitudeA = centreToCentreVector.Size();


	//If magnitude of A or V is 0 then no collision (object is stationary)
	if (magnitudeA <= 0.0f || magnitudeV <= 0.0f)
	{
		return false;
	}

	floatStr = FString::SanitizeFloat(magnitudeA) + " Magnitude A";
	GEngine->AddOnScreenDebugMessage(3, 15.0f, FColor::Yellow, *floatStr);
	FVector A = centreToCentreVector;
	GEngine->AddOnScreenDebugMessage(4, 15.0f, FColor::Yellow, A.ToString() + " Vector A");
	float q = FVector::DotProduct(A, V); // get dot product between A and V
	floatStr = FString::SanitizeFloat(q) + " dot product A,V";
	GEngine->AddOnScreenDebugMessage(5, 15.0f, FColor::Yellow, *floatStr);
	while (q == NULL) {
		q = acos(q / (magnitudeA * magnitudeV)); // find angle q between A and V
	}
	floatStr = FString::SanitizeFloat(q) + " Angle between A and V in radians";
	GEngine->AddOnScreenDebugMessage(6, 15.0f, FColor::Yellow, *floatStr);
	q = q * (180 / PI); // convert to degrees
	if (q == 180.0 || q == 360.0)
	{
		q = 0.0f;
	}
	floatStr = FString::SanitizeFloat(q) + " Angle between A and V in degrees";
	GEngine->AddOnScreenDebugMessage(7, 15.0f, FColor::Yellow, *floatStr);
	float d = sin(q) * magnitudeA; // find size of d
	d = sqrt(d * d); // absolute value of d
	while (d == NULL) {
		d = sin(q) * magnitudeA; // find size of d
		d = sqrt(d * d); // absolute value of d
	}
	floatStr = FString::SanitizeFloat(d) + " Size of d";
	GEngine->AddOnScreenDebugMessage(11, 15.0f, FColor::Yellow, *floatStr);
	float radiusSum = otherRadius + mRadius; // get sum of radii
	floatStr = FString::SanitizeFloat(radiusSum) + " Sum of radii";
	GEngine->AddOnScreenDebugMessage(9, 15.0f, FColor::Yellow, *floatStr);

	//can a collision occur?
	if (d < radiusSum)
	{
		//find e using pythagoras
		float e = sqrtf((radiusSum * radiusSum) - (d * d));
		floatStr = FString::SanitizeFloat(e) + " magnitude e";
		GEngine->AddOnScreenDebugMessage(12, 15.0f, FColor::Yellow, *floatStr);

		//find vector VC length
		float VCMagnitude = sqrtf((magnitudeA * magnitudeA) - (d * d)) - e;
		floatStr = FString::SanitizeFloat(VCMagnitude) + " magnitude VC";
		GEngine->AddOnScreenDebugMessage(13, 15.0f, FColor::Yellow, *floatStr);

		//find vector VC
		FVector VC = V.GetSafeNormal() * VCMagnitude;

		GEngine->AddOnScreenDebugMessage(14, 15.0f, FColor::Yellow, VC.ToString() + " Vector VC");

		FVector collisionPoint = Displacement + VC;

		if (VCMagnitude <= 0) // collision has taken place
		{
			GEngine->AddOnScreenDebugMessage(10, 15.0f, FColor::Yellow, TEXT("Collision has taken place"));
			
			return true;
		}
	
	}


	return false;
}

