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
	//FVector P = KToSphereVector;
	//float magnitudeP = KToSphereVector.Size();
	// find angle between N and P
	//float q1 = FVector::DotProduct(surfaceNormalOfPlane, KToSphereVector);

	//q1 = acos(q1 / (magnitudeN * magnitudeP));

	//q1 = q1 * (180 / PI); // convert to degrees

	//float q2 = 90.0f - q1; // q1 + q2 should equal 90 degrees so we find q2 by taking 90 from q1

	float d = FVector::DotProduct(N, KToSphereVector) / magnitudeN; // find magnitude of d 

	//d = abs(d); // absolute value of d

	float r = mRadius;

	//float magnitudeV = Velocity.Size();

	// find angle between V and -N
	//float s = FVector::DotProduct(Velocity, -surfaceNormalOfPlane);

	//if (s / magnitudeN * magnitudeV > 1.0f) s = acos(1.0f);
	//s = acos(s / (magnitudeN * magnitudeV));

	//s = s * (180 / PI);
	
	//d -= r; // constant 24 found by trial and error (a radius of 24 gives the perfect collision so modulate based on this value)
	//float vcOffset = r / (cos(s)); // amount to take away from magnitudeVC to get accurate magnitude
	//float magnitudeVC = (d - r) / cos(s); // find magnitude of VC using pythagorean theorem (cos(theta) = adjacent/Hypoteneus)

	//magnitudeVC = sqrt(magnitudeVC * magnitudeVC);
	//vcOffset = sqrt(vcOffset * vcOffset);

	//magnitudeVC -= vcOffset;

	if (d < r) // if true a collision will accur and VC will give the point of collision
	{


		//GEngine->AddOnScreenDebugMessage(2190, 15.0f, FColor::Yellow, TEXT("Collision Detect Plane"));
		//FVector VC = Velocity.GetSafeNormal() * magnitudeVC; // VC and V are in same direction so we can use length of VC to find VC
		//GEngine->AddOnScreenDebugMessage(2191, 15.0f, FColor::Yellow, VC.ToString() + " VC");
		if(!IsNormalForce) Displacement.Z -= d - r + 1.0; // only do this on first collisions (not when using normal force)

		// Add impulse
		//SetActorLocation(GetActorLocation() + VC);
		FVector UnitVAfterCollision = 2 * surfaceNormalOfPlane * 
			(FVector::DotProduct(surfaceNormalOfPlane, -Velocity.GetSafeNormal())) + Velocity.GetSafeNormal(); // get unit vector of velocity after collision
		FVector VAfterCollision = UnitVAfterCollision * Velocity.Size() * mCoefficientOfRestitution; // find velocity vector after collision using unit vector & size of velocity vector before collision
		
		if(!VAfterCollision.IsNearlyZero()) Velocity = VAfterCollision; // don't bother with low impulses
		
		NormalForceVector = -surfaceNormalOfPlane;
		return true;
	}
	if(NormalForceVector == -surfaceNormalOfPlane) NormalForceVector = { 0.0f,0.0f,0.0f };
	return false;
}

FVector APO_Sphere::FindImpulseMovingSphere(FVector otherVelocity, FVector selfVelocity, FVector otherLocation, FVector selfLocation, float otherMass)
{
	// Velocity - V1
	// otherVelocity - V2

	// Displacement - S1
	// otherLocation - S2

	FVector lineBetweenSpheres = selfLocation - otherLocation;

	// find angle q1/2 (angle between V1/2 and lineBetweenSpheres)
	float q1 = FVector::DotProduct(selfVelocity, lineBetweenSpheres);

	q1 = acos(q1 / (selfVelocity.Size() * lineBetweenSpheres.Size()));

	q1 = q1 * (180 / PI); // convert to degrees

	float q2 = FVector::DotProduct(otherVelocity, otherLocation - selfLocation);

	q2 = acos(q2 / (otherVelocity.Size() * (otherLocation - selfLocation).Size()));

	q2 = q2 * (180 / PI); // convert to degrees

	q2 = abs(q2);

	q1 = abs(q1);

	FVector Z1 = lineBetweenSpheres / lineBetweenSpheres.Size();

	float B1 = (cos(q2) * otherVelocity.Size() * otherMass) / mMass;

	FVector Z2 = otherLocation - selfLocation;
	Z2 = Z2 / Z2.Size();

	float B2 = (cos(q1) * selfVelocity.Size() * mMass) / otherMass;

	FVector newVelocity = selfVelocity + (B1 * Z1) - (B2 * Z2);

	return newVelocity * mCoefficientOfRestitution;

}

bool APO_Sphere::CheckForMovingSphereCollision(FVector otherVelocity, float otherRadius, FVector otherLocation, float otherMass)
{
	//other.Velocity - V2
	//Velocity - V1

	//other.mRadius - r2
	//mRadius - r1

	//startPosSphere1 - P1
	//startPosSphere2 - P2

	float radSum = mRadius + otherRadius;

	float distance = (otherLocation - GetActorLocation()).Size();

	distance = abs(distance);

	

	if (distance <= radSum)
	{

		if (!IsImpulseLocked) {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Collision Detect Moving Sphere"));
			FVector newVel = FindImpulseMovingSphere(otherVelocity, Velocity, otherLocation, Displacement, otherMass);
			Velocity -= newVel;
			IsImpulseLocked = true; // lock collisions for one time step after collisions
			return true;
		}
		return true;
	}
	IsImpulseLocked = false;
	// define sum values for each vector for simplicity in calculations
	float sumXp = GetActorLocation().X - otherLocation.X;
	float sumYp = GetActorLocation().Y - otherLocation.Y;
	float sumZp = GetActorLocation().Z - otherLocation.Z;


	float sumXv = Velocity.X - otherVelocity.X;
	float sumYv = Velocity.Y - otherVelocity.Y;
	float sumZv = Velocity.Z - otherVelocity.Z;


	float A = (sumXv * sumXv) + (sumYv * sumYv) + (sumZv * sumZv);

	float B = (2 * sumXp * sumXv) + (2 * sumYp * sumYv) + (2 * sumZp * sumZv);

	float C = (sumXp * sumXp) + (sumYp * sumYp) + (sumZp * sumZp) - ((mRadius + otherRadius) * (mRadius * otherRadius));


	// determinent is in square root of quadratic formula
	float Determinent = (B * B) - (4 * A * C);

	// thus if determinent is less than 0 there is no valid values for t and thus no collision has occurred
	if (Determinent < 0) {
		return false;
	}

	float t1 = (-B + sqrtf(Determinent)) / (2 * A); // first t value
	float t2 = (-B - sqrtf(Determinent)) / (2 * A); // second t value


	//if (t1 <= 0.0f && t2 <= 0.0f) GEngine->AddOnScreenDebugMessage(233, 0.1f, FColor::Yellow, TEXT("Collision Detect Moving Sphere"));

	if (t1 >= 0.0f && t2 >= 0.0f && t1 <= 1.0f && t2 <= 1.0f) // if either values are not in range 0-1 or there is only one value for t, then there is no collision (in this frame)
	{


		// collision possible
		float t;
		// find smaller value
		if (t1 < t2) {
			t = t1;
		}
		else {
			t = t2;
		}
		// collision occurred
		//dt = t;
		//StepSimulation();
		GEngine->AddOnScreenDebugMessage(18, 15.0f, FColor::Yellow, TEXT("Collision Detect Moving Sphere"));
		//isMovingSphere = false;
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

FVector APO_Sphere::FindWindForce(FVector windSpeed)
{
	FVector windForce = mSurfaceArea * mDensity * WindSpeed;
	return windForce;
}

void APO_Sphere::StepSimulation()
{
	FVector Vnew; // new velocity at time t + dt
	FVector Snew; // new position at time t + dt
	FVector k1, k2;
	FVector F; // total force
	FVector A; // acceleration


	// find forces acting on object
	F = FindGravityForce() + FindWindForce(WindSpeed) + FindDragForce(Velocity) + Force; // global forces + aggregate forces

	
	// Normal force vector is - surface normal -N, this is a unit vector, to find desired magnitude, find amount of force exerted in direction of
	// normal force vector and since this force acts in opposite direction to current force, take it away from Force vector
	F -= NormalForceVector * FVector::DotProduct(F, NormalForceVector);

	A = F / mMass;

	k1 = dt * A;

	F = FindGravityForce() + FindWindForce(WindSpeed) + FindDragForce(Velocity + k1) + Force;
	
	F -= NormalForceVector * FVector::DotProduct(F, NormalForceVector);
	

	A = F / mMass;

	k2 = dt * A;

	Force = { 0.0f, 0.0f, 0.0f };

	/*FString tempString = FString::SanitizeFloat(dt) + " Timestep (in seconds)";
	GEngine->AddOnScreenDebugMessage(15, 15.0f, FColor::Yellow, *tempString);*/

	// calculate new velocity at time t + dt

	Vnew = Velocity + (k1 + k2) / 2;

	// calculate new displacement at time t + dt

	Snew = Displacement + Vnew * dt * 100;
	//GEngine->AddOnScreenDebugMessage(13, 15.0f, FColor::Yellow, Vnew.ToString() + ": Velocity");
	//GEngine->AddOnScreenDebugMessage(14, 15.0f, FColor::Yellow, Snew.ToString() + ": Displacement");

	// update old velocity and displacement with the new ones

	Velocity = Vnew;
	Displacement = Snew;

	SetActorLocation(Displacement); // convert to centimeters

}

