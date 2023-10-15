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

	startRadius = mSphere->GetUnscaledSphereRadius();
	
	mSphere->SetSphereRadius(mRadius); 
 	
	mRadius = mRadius / 100; // convert to meters

	mVolume = (4 / 3) * PI * (mRadius * mRadius * mRadius);
	mDensity = mMass / mVolume;
	mSurfaceArea = 4 * PI * (mRadius * mRadius);

	

}

// Called when the game starts or when spawned
void APO_Sphere::BeginPlay()
{
	Super::BeginPlay();
	Displacement = GetActorLocation();
	//Velocity = InitialVelocity;
	this->SetActorScale3D({ mRadius / startRadius, mRadius / startRadius, mRadius / startRadius });
	Force += FindGravityForce();

	/*FString tempString = FString::SanitizeFloat(mVolume) + " Volume";
	GEngine->AddOnScreenDebugMessage(150, 15.0f, FColor::Yellow, *tempString);
	tempString = FString::SanitizeFloat(mDensity) + " Density";
	GEngine->AddOnScreenDebugMessage(151, 15.0f, FColor::Yellow, *tempString);
	tempString = FString::SanitizeFloat(mSurfaceArea) + " SA";
	GEngine->AddOnScreenDebugMessage(152, 15.0f, FColor::Yellow, *tempString);*/
}

// Called every frame
void APO_Sphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Calculate Velocity and displacement
	if (isMovingSphere)
	{
		StepSimulation();
	}
}


bool APO_Sphere::CheckForSphereCollision(FVector centreToCentreVector, float otherRadius)
{
	if (Velocity.Size() <= 0.0f || !isMovingSphere) return false;
	//define V and |V|
	float magnitudeV = Velocity.Size();
	//define A and |A|
	float magnitudeA = centreToCentreVector.Size();


	//If magnitude of A or V is 0 then no collision (object is stationary)
	if (magnitudeA <= 0.0f || magnitudeV <= 0.0f)
	{
		return false;
	}

	//FVector A = centreToCentreVector;
	float q = FVector::DotProduct(centreToCentreVector, Velocity); // get dot product between A and V
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
		FVector VC = Velocity.GetSafeNormal() * VCMagnitude;


		FVector collisionPoint = Displacement + VC;

		if (VCMagnitude <= 0) // collision has taken place
		{
			GEngine->AddOnScreenDebugMessage(10, 15.0f, FColor::Yellow, TEXT("Collision Detect Sphere"));

			// add impulse

			return true;
		}
	
	}


	return false;
}

bool APO_Sphere::CheckForPlaneCollision(FVector KToSphereVector, FVector surfaceNormalOfPlane)
{

	if (Velocity.Size() <= 0.0f || !isMovingSphere) return false;
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

	float d = FVector::DotProduct(N, KToSphereVector) / magnitudeN; // find magnitude of d 

	d = sqrt(d * d); // absolute value of d

	float r = mRadius;

	float magnitudeV = Velocity.Size();

	// find angle between V and -N
	float s = FVector::DotProduct(Velocity, -surfaceNormalOfPlane);

	if (s / magnitudeN * magnitudeV > 1.0f) s = acos(1.0f);
	s = acos(s / (magnitudeN * magnitudeV));

	s = s * (180 / PI);
	
	d -= r; // constant 24 found by trial and error (a radius of 24 gives the perfect collision so modulate based on this value)
	//float vcOffset = r / (cos(s)); // amount to take away from magnitudeVC to get accurate magnitude
	float magnitudeVC = d / cos(s); // find magnitude of VC using pythagorean theorem (cos(theta) = adjacent/Hypoteneus)

	magnitudeVC = sqrt(magnitudeVC * magnitudeVC);
	//vcOffset = sqrt(vcOffset * vcOffset);

	//magnitudeVC -= vcOffset;

	FString tempString = FString::SanitizeFloat(d) + " d";
	GEngine->AddOnScreenDebugMessage(10, 15.0f, FColor::Yellow, *tempString);

	if (magnitudeVC <= magnitudeV || d < 0.0f) // if true a collision will accur and VC will give the point of collision
	{

		// enforce d as 0
		//magnitudeVC = 0.0f / cos(s);

		GEngine->AddOnScreenDebugMessage(14, 15.0f, FColor::Yellow, TEXT("Collision Detect Plane"));
		FVector VC = Velocity.GetSafeNormal() * magnitudeVC; // VC and V are in same direction so we can use length of VC to find VC

		// Add impulse
		//SetActorLocation(GetActorLocation() + VC);
		FVector UnitVAfterCollision = 2 * surfaceNormalOfPlane * 
			(FVector::DotProduct(surfaceNormalOfPlane, -Velocity.GetSafeNormal())) + Velocity.GetSafeNormal(); // get unit vector of velocity after collision
		FVector VAfterCollision = UnitVAfterCollision * Velocity.Size() * mCoefficientOfRestitution; // find velocity vector after collision using unit vector & size of velocity vector before collision
		Velocity = VAfterCollision;
		if (VAfterCollision.Size() < 1.0f) {
			isMovingSphere = false;
		}
		return true;
	}

	return false;
}

FVector APO_Sphere::FindGravityForce()
{
	long double G = 6.67430 * powf(10, -11);
	float EarthMass = 5.98 * powf(10, 24);
	long double FGrav = G * mMass * EarthMass;
	float DistanceToEarthCentre = 6.38 * powf(10, 6);
	FGrav /= (DistanceToEarthCentre * DistanceToEarthCentre);
	return FVector(0.0f, 0.0f, -FGrav); // Force of gravity acts downwards
}

FVector APO_Sphere::FindDragForce(FVector velocity)
{
	float FDrag = mCoefficientOfDrag * mDensity * ((velocity.Size() * velocity.Size()) / 2) * mSurfaceArea; // find drag force
	FVector DragVector = -velocity.GetSafeNormal(); // drag acts opposite to direction of movement
	DragVector *= FDrag; // FDrag value defines magnitude of drag force

	//GEngine->AddOnScreenDebugMessage(100, 15.0f, FColor::Yellow, DragVector.ToString() + ": Drag Vector");

	return DragVector;
}

void APO_Sphere::StepSimulation()
{
	FVector Vnew; // new velocity at time t + dt
	FVector Snew; // new position at time t + dt
	FVector k1, k2;
	FVector F; // total force
	FVector A; // acceleration


	// find forces acting on object
	F = FindGravityForce() + FindDragForce(Velocity);

	A = F / mMass;

	k1 = dt * A;

	F = FindGravityForce() + FindDragForce(Velocity + k1);

	A = F / mMass;

	k2 = dt * A;

	/*FString tempString = FString::SanitizeFloat(dt) + " Timestep (in seconds)";
	GEngine->AddOnScreenDebugMessage(15, 15.0f, FColor::Yellow, *tempString);*/

	// calculate new velocity at time t + dt

	Vnew = Velocity + (k1 + k2) / 2;

	// calculate new displacement at time t + dt

	Snew = Displacement + Vnew * 1000 * dt;
	//GEngine->AddOnScreenDebugMessage(13, 15.0f, FColor::Yellow, Vnew.ToString() + ": Velocity");
	//GEngine->AddOnScreenDebugMessage(14, 15.0f, FColor::Yellow, Snew.ToString() + ": Displacement");

	// update old velocity and displacement with the new ones

	Velocity = Vnew;
	Displacement = Snew;

	SetActorLocation(Displacement); // convert to centimeters

}

