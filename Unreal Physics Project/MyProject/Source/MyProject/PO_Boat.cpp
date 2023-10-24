// Fill out your copyright notice in the Description page of Project Settings.


#include "PO_Boat.h"

// Sets default values
APO_Boat::APO_Boat()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mCentreOfGravity = GetActorLocation() + FVector{ 0.0f, 0.0f, BaseSideLength / 2 }; // find centre of gravity

}

// Called when the game starts or when spawned
void APO_Boat::BeginPlay()
{
	Super::BeginPlay();
	float metersEdgeLength = BaseSideLength / 100; // convert to meters so we don't get errors with other functions
	mVolume = metersEdgeLength * metersEdgeLength * metersEdgeLength;
	mDensity = mMass / mVolume;
	mSurfaceArea = (metersEdgeLength * metersEdgeLength) * 6;
	Displacement = GetActorLocation();
}

// Called every frame
void APO_Boat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	mCentreOfGravity = GetActorLocation() + FVector{ 0.0f, 0.0f, BaseSideLength / 2 }; // find centre of gravity
	StepSimulation();
}

float APO_Boat::FindVolumeSubmerged(float submersedLength) // point of submersion will be point of the cube that is currently touching plane
{
	if (submersedLength <= 0.0f) return 0.0f;

	float unsubmersedLength = BaseSideLength - submersedLength;

	unsubmersedLength /= 100; // convert to meters

	float unsubmersedVolume = unsubmersedLength * unsubmersedLength * unsubmersedLength;

	float submersedVolume = mVolume - unsubmersedVolume;

	return submersedVolume; // this is also volume of liquid displaced
}

FVector APO_Boat::FindBuoyantForce(float fluidDensity)
{
	float fluidVolumeDisplaced = FindVolumeSubmerged(-mSubmersedLength);
	float BuoyantForce = fluidDensity * g * fluidVolumeDisplaced;
	return FVector(0.0f, 0.0f, -BuoyantForce);
}

FVector APO_Boat::FindGravityForce()
{
	long double G = 6.67430 * powf(10, -11);
	float EarthMass = 5.98 * powf(10, 24);
	long double FGrav = G * mMass * EarthMass;
	float DistanceToEarthCentre = 6.38 * powf(10, 6);
	FGrav /= (DistanceToEarthCentre * DistanceToEarthCentre);
	return FVector(0.0f, 0.0f, -FGrav); // Force of gravity acts downwards
}


// Called to bind functionality to input
void APO_Boat::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FVector APO_Boat::FindDragForce(FVector velocity)
{
	float FDrag = mCoefficientOfDrag * mDensity * ((velocity.Size() * velocity.Size()) / 2) * mSurfaceArea; // find drag force
	FVector DragVector = -velocity.GetSafeNormal(); // drag acts opposite to direction of movement
	DragVector *= FDrag; // FDrag value defines magnitude of drag force

	//GEngine->AddOnScreenDebugMessage(100, 15.0f, FColor::Yellow, DragVector.ToString() + ": Drag Vector");

	return DragVector;
}

FVector APO_Boat::FindWindForce(FVector windSpeed)
{
	FVector windForce = mSurfaceArea * mDensity * WindSpeed;
	return windForce;
}

void APO_Boat::StepSimulation()
{
	FVector Vnew; // new velocity at time t + dt
	FVector Snew; // new position at time t + dt
	FVector k1, k2;
	FVector F; // total force
	FVector A; // acceleration


	// find forces acting on object
	F = FindGravityForce() + FindWindForce(WindSpeed) + FindDragForce(Velocity) + FindBuoyantForce(mFluidDensity); // global forces + aggregate forces


	A = F / mMass;

	k1 = dt * A;

	F = FindGravityForce() + FindWindForce(WindSpeed) + FindDragForce(Velocity + k1) + FindBuoyantForce(mFluidDensity);

	A = F / mMass;

	k2 = dt * A;

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

