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

	Acceleration = { 0.0f, 0.0f, g };
 	

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


bool APO_Sphere::CheckForSphereCollision(FVector centreToCentreVector, float otherRadius)
{
	if (Velocity.Size() <= 0.0f) return false;
	//define V and |V|
	float magnitudeV = Velocity.Size();
	FVector V = Velocity;
	//define A and |A|
	float magnitudeA = centreToCentreVector.Size();


	//If magnitude of A or V is 0 then no collision (object is stationary)
	if (magnitudeA <= 0.0f || magnitudeV <= 0.0f)
	{
		return false;
	}

	//FVector A = centreToCentreVector;
	float q = FVector::DotProduct(centreToCentreVector, V); // get dot product between A and V
	q = acos(q / (magnitudeA * magnitudeV)); // find angle q between A and V
	
	q = q * (180 / PI); // convert to degrees
	float d = sin(q) * magnitudeA; // find size of d
	d = sqrt(d * d); // absolute value of d
	float radiusSum = otherRadius + mRadius; // get sum of radii

	//can a collision occur?
	if (d < radiusSum)
	{
		//find e using pythagoras
		float e = sqrtf((radiusSum * radiusSum) - (d * d));

		//find vector VC length
		float VCMagnitude = sqrtf((magnitudeA * magnitudeA) - (d * d)) - e;

		//find vector VC
		FVector VC = V.GetSafeNormal() * VCMagnitude;


		FVector collisionPoint = Displacement + VC;

		if (VCMagnitude <= 0) // collision has taken place
		{
			GEngine->AddOnScreenDebugMessage(10, 15.0f, FColor::Yellow, TEXT("Collision Detect Sphere"));

			// once collision occurs set position to collision point (so no overlap happens)
			Displacement = collisionPoint;
			return true;
		}
	
	}


	return false;
}

bool APO_Sphere::CheckForPlaneCollision(FVector KToSphereVector, FVector surfaceNormalOfPlane)
{

	if (Velocity.Size() <= 0.0f) return false;
	FString floatStr;
	FVector N = surfaceNormalOfPlane;
	float magnitudeN = surfaceNormalOfPlane.Size();
	FVector P = KToSphereVector;
	float magnitudeP = KToSphereVector.Size();
	// find angle between N and P
	float q1 = FVector::DotProduct(surfaceNormalOfPlane, KToSphereVector);

	q1 = acos(q1 / (magnitudeN * magnitudeP));

	q1 = q1 * (180 / PI); // convert to degrees

	float q2 = 90.0f - q1; // q1 + q2 should equal 90 degrees so we find q2 by taking 90 from q1

	float d = FVector::DotProduct(N, KToSphereVector) / magnitudeN; // find magnitude of d using pythagorean theorem (sin(theta) = Opposite/Hypoteneus)

	d = sqrt(d * d); // absolute value of d

	float r = mRadius;

	FVector V = Velocity;
	float magnitudeV = Velocity.Size();

	// find angle between V and -N
	float s = FVector::DotProduct(V, -surfaceNormalOfPlane);

	if (s / magnitudeN * magnitudeV > 1.0f) s = acos(1.0f);
	s = acos(s / (magnitudeN * magnitudeV));

	s = s * (180 / PI);
	

	float vcOffset = r / (cos(s)); // amount to take away from magnitudeVC to get accurate magnitude
	float magnitudeVC = d / cos(s); // find magnitude of VC using pythagorean theorem (cos(theta) = adjacent/Hypoteneus)

	magnitudeVC = sqrt(magnitudeVC * magnitudeVC);
	vcOffset = sqrt(vcOffset * vcOffset);

	magnitudeVC -= vcOffset;


	if (magnitudeVC <= magnitudeV) // if true a collision will accur and VC will give the point of collision
	{

		GEngine->AddOnScreenDebugMessage(14, 15.0f, FColor::Yellow, TEXT("Collision Detect Plane"));
		FVector VC = V.GetSafeNormal() * magnitudeVC; // VC and V are in same direction so we can use length of VC to find VC

		// VC is point of collision
		Displacement += VC/2;
		SetActorLocation(Displacement);
		Velocity = { 0.0f, 0.0f, 0.0f };
		isMovingSphere = false;
		return true;
	}

	return false;
}

