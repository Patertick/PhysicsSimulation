// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsObject.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsPublic.h"
#include "PhysXPublicCore.h"
#include "PhysXIncludes.h"
#include "PxTriangleMesh.h"
#include "PxSimpleTypes.h"
#include <String>
#include <Kismet/GameplayStatics.h>

// Sets default values
APhysicsObject::APhysicsObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mBoundingVisualSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Visual Bounding Sphere"));
	RootComponent = mBoundingVisualSphere;

	mObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Physics Mesh"));
	//mObjectMesh->GetStaticMesh()->bAllowCPUAccess = true;   NOTE: ABSOLUTELY DEADLY CODE THAT CRASHES UNREAL
	mObjectMesh->SetOwnerNoSee(true);
	
	mObjectMesh->AttachToComponent(mBoundingVisualSphere, FAttachmentTransformRules::KeepRelativeTransform);
	
}



// Called when the game starts or when spawned
void APhysicsObject::BeginPlay()
{
	Super::BeginPlay();
	if (GetVertexPositions() == false) {
		if (GEngine == nullptr) return;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("No vertices found"));
		return;
	}


	GenerateSphere();

	// Find a predicted sphere from generated sphere
	mPredictedSphere.sCenter = mSphere.sCenter;
	mPredictedSphere.sRadius = mSphere.sRadius;

	// set visuals
	FVector oldPos = GetActorLocation();
	SetActorLocation(mSphere.sCenter);
	mBoundingVisualSphere->SetSphereRadius(mSphere.sRadius);
	FVector newPos = GetActorLocation();

	FVector offset = newPos - oldPos;

	if (!offset.IsNearlyZero()) mObjectMesh->SetWorldLocation(GetActorLocation() - offset);

	mDisplacement = mSphere.sCenter;

	float calcRadius = mSphere.sRadius / 100; // convert to meters

	mVolume = (4 / 3) * PI * (calcRadius * calcRadius * calcRadius);
	mDensity = mMass / mVolume;
	mSurfaceArea = 4 * PI * (calcRadius * calcRadius);
	FString tempString = "Volume: " + FString::SanitizeFloat(mVolume);
	GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::Yellow, *tempString);
	tempString = "Density: " + FString::SanitizeFloat(mDensity);
	GEngine->AddOnScreenDebugMessage(2, 15.0f, FColor::Yellow, *tempString);
	tempString = "SA: " + FString::SanitizeFloat(mSurfaceArea);
	GEngine->AddOnScreenDebugMessage(3, 15.0f, FColor::Yellow, *tempString);


	mDefaultTimeStep = mTimeStep;
}

bool APhysicsObject::GetVertexPositions()
{
	if (!mObjectMesh) {
		if (GEngine == nullptr) return false;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Invalid Mesh"));
		return false;
	}
	if (!mObjectMesh->IsValidLowLevel()) {
		if (GEngine == nullptr) return false;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Invalid Mesh2"));
		return false;
	}

	// static component transform
	FTransform transform = mObjectMesh->GetComponentTransform();

	// test for valid body set up
	UBodySetup* body = mObjectMesh->GetBodySetup();

	if (!body) {
		if (GEngine == nullptr) return false;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Invalid Body"));
		return false;
	}
	if(!body->IsValidLowLevel()) {
		if (GEngine == nullptr) return false;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Invalid Body2"));
		return false;
	}

	for (PxTriangleMesh* eachTriMesh : body->TriMeshes) {
		if (!eachTriMesh) return false;

		// Number of vertices
		PxU32 vertexCount = eachTriMesh->getNbVertices();

		// Vertex array
		const PxVec3* vertices = eachTriMesh->getVertices();


		for (PxU32 i = 0; i < vertexCount; i++) {
			mVertexPositions.Add(transform.TransformPosition(P2UVector(vertices[i])));
		}
	}


	return true;
}

void APhysicsObject::FindMostSeparatedPoints(int& min, int& max)
{
	// compute most optimal bounding sphere using vertex positions

	int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;

	// find extremal points on mesh (up to a maximum of six but not necessarily all unique points)
	for (int32 i = 0; i < mVertexPositions.Num(); i++) {
		if (mVertexPositions[i].X < mVertexPositions[minX].X) minX = i;
		if (mVertexPositions[i].X > mVertexPositions[maxX].X) maxX = i;
		if (mVertexPositions[i].Y < mVertexPositions[minY].Y) minY = i;
		if (mVertexPositions[i].Y > mVertexPositions[minY].Y) maxY = i;
		if (mVertexPositions[i].Z < mVertexPositions[minZ].Z) minZ = i;
		if (mVertexPositions[i].Z > mVertexPositions[minZ].Z) maxZ = i;
	}

	// find square distances for XYZ pairs
	float distanceX = FVector::DotProduct(mVertexPositions[maxX] - mVertexPositions[minX],
		mVertexPositions[maxX] - mVertexPositions[minX]);
	float distanceY = FVector::DotProduct(mVertexPositions[maxY] - mVertexPositions[minY],
		mVertexPositions[maxY] - mVertexPositions[minY]);
	float distanceZ = FVector::DotProduct(mVertexPositions[maxZ] - mVertexPositions[minZ],
		mVertexPositions[maxZ] - mVertexPositions[minZ]);

	// select axis values largest distance between min and max to be used in algorithm
	min = minX;
	max = maxX;
	if (distanceY > distanceX && distanceY >= distanceZ) {
		min = minY;
		max = maxY;		
	}
	else if (distanceZ > distanceX && distanceZ >= distanceY) {
		min = minZ;
		max = maxZ;
	}
}

void APhysicsObject::SphereFromDistantPoints()
{
	// find the most separated pair of points on mesh
	int min, max;
	FindMostSeparatedPoints(min, max);

	// set up bounding sphere to just encompass two points
	FVector sphereCenter = (mVertexPositions[min] + mVertexPositions[max]) * 0.5f;
	//mBoundingSphere->SetCenterOfMass(sphereCenter);

	float sphereRadius = FVector::DotProduct(mVertexPositions[max] - sphereCenter,
		mVertexPositions[max] - sphereCenter);
	sphereRadius = sqrt(sphereRadius);
	mSphere.sCenter = sphereCenter;
	mSphere.sRadius = sphereRadius;


}

void APhysicsObject::SphereOfSphereAndPoint(FVector point)
{
	// find distance between input point and center of bounding sphere
	FVector distanceVector = point - mSphere.sCenter;
	float squareDistance = FVector::DotProduct(distanceVector, distanceVector);

	if (squareDistance > mSphere.sRadius * mSphere.sRadius) {
		// point is outside sphere so update sphere bounds
		float distance = sqrt(squareDistance);
		float newRadius = (mSphere.sRadius + distance) * 0.5f;
		float k = (newRadius - mSphere.sRadius) / distance;
		mSphere.sRadius = newRadius;
		mSphere.sCenter += distanceVector * k;
	}
}

void APhysicsObject::GenerateSphere()
{
	// generate approximate sphere from extremal points
	SphereFromDistantPoints();
	// iterate through every point in the mesh to get a tight fitting sphere (note: reductions in cost by using a point set generated alongside AABB)
	for (int i = 0; i < mVertexPositions.Num(); i++) {
		SphereOfSphereAndPoint(mVertexPositions[i]);
	}
}

// Called every frame
void APhysicsObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Hit) return;
	StepSimulation();
	mNormalForce = { 0.0f, 0.0f ,0.0f };
	mNormalForce += CheckForPlaneCollision();


}

FVector APhysicsObject::FindGravityForce()
{
	long double G = 6.67430 * powf(10, -11);
	float EarthMass = 5.98 * powf(10, 24);
	long double FGrav = G * mMass * EarthMass;
	float DistanceToEarthCentre = 6.38 * powf(10, 6);
	FGrav /= (DistanceToEarthCentre * DistanceToEarthCentre);
	return FVector(0.0f, 0.0f, -FGrav); // Force of gravity acts downwards
}

FVector APhysicsObject::FindDragForce(FVector velocity)
{
	float FDrag = mCoefficientOfDrag * mDensity * ((velocity.Size() * velocity.Size()) / 2) * mSurfaceArea; // find drag force
	FVector DragVector = -velocity.GetSafeNormal(); // drag acts opposite to direction of movement
	DragVector *= FDrag; // FDrag value defines magnitude of drag force

	//GEngine->AddOnScreenDebugMessage(100, 15.0f, FColor::Yellow, DragVector.ToString() + ": Drag Vector");

	return DragVector;
}

FVector APhysicsObject::FindWindForce(FVector windSpeed)
{
	FVector windForce = mSurfaceArea * mDensity * mWindSpeed;
	return windForce;
}

void APhysicsObject::StepSimulation()
{
	FVector Vnew; // new velocity at time t + dt
	FVector Snew; // new position at time t + dt
	FVector k1, k2;
	FVector F; // total force
	FVector A; // acceleration

	// Actual step simulation
	
	F = FindGravityForce() + mNormalForce + FindWindForce(mWindSpeed) + FindDragForce(mVelocity);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, mNormalForce.ToString());

	A = F / mMass;

	k1 = mTimeStep * A;

	F = FindGravityForce() + mNormalForce + FindWindForce(mWindSpeed) + FindDragForce(mVelocity + k1);

	A = F / mMass;

	k2 = mTimeStep * A;

	Vnew = mVelocity + ((k1 + k2) / 2);

	Snew = mDisplacement + (Vnew * mTimeStep * 100); // convert to centimeters

	mVelocity = Vnew;
	mDisplacement = Snew;

	SetActorLocation(mDisplacement);
	mSphere.sCenter = mDisplacement;

	




	// find forces acting on object
	F = FindGravityForce() + mNormalForce + FindWindForce(mWindSpeed) + FindDragForce(mVelocity) + mForce; // global forces + aggregate forces


	// Normal force vector is - surface normal -N, this is a unit vector, to find desired magnitude, find amount of force exerted in direction of
	// normal force vector and since this force acts in opposite direction to current force, take it away from Force vector
	//F -= NormalForceVector * FVector::DotProduct(F, NormalForceVector);

	A = F / mMass;

	k1 = mTimeStep * A;

	F = FindGravityForce() + mNormalForce + FindWindForce(mWindSpeed) + FindDragForce(mVelocity + k1) + mForce;

	//F -= NormalForceVector * FVector::DotProduct(F, NormalForceVector);


	A = F / mMass;

	k2 = mTimeStep * A;

	mForce = { 0.0f, 0.0f, 0.0f };

	/*FString tempString = FString::SanitizeFloat(dt) + " Timestep (in seconds)";
	GEngine->AddOnScreenDebugMessage(15, 15.0f, FColor::Yellow, *tempString);*/

	// calculate new velocity at time t + dt

	Vnew = mVelocity + ((k1 + k2) / 2);

	// calculate new displacement at time t + dt

	Snew = mDisplacement + (Vnew * mTimeStep * 100);

	// predict approximately where the bounding sphere will be in the next timestep assuming mTimeStep is a constant value
	mPredictedSphere.sCenter = Snew;
	mPredictedSphere.sRadius = mSphere.sRadius;

	// find capsule swept volume
	mCapsule.sStartPoint = mSphere.sCenter;
	mCapsule.sEndPoint = mPredictedSphere.sCenter;
	mCapsule.sRadius = mSphere.sRadius;
	
	if (mTimeStep != mDefaultTimeStep) {
		mTimeStep = mDefaultTimeStep; // reset time step
	}

}

FVector APhysicsObject::CheckForPlaneCollision()
{

	if (mVelocity.Size() <= 0.0f) return { 0.0f,0.0f,0.0f };
	float magnitudeNormal = mSurfaceNormal.Size();

	FVector planeToSelfVector = GetActorLocation() - mArbritraryPoint;

	float d = FVector::DotProduct(mSurfaceNormal, planeToSelfVector) / magnitudeNormal; // find magnitude of d 

	FVector capsuleLength = mCapsule.sEndPoint - mCapsule.sStartPoint; // start point to end point vector


	if (d < capsuleLength.Size() + mCapsule.sRadius) // if true a collision will accur and VC will give the point of collision
	{
		// if this is a direct collision don't bother finding timestep
		if (d <= mSphere.sRadius)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Collision"));
			FVector UnitVAfterCollision = 2 * mSurfaceNormal *
				(FVector::DotProduct(mSurfaceNormal, -mVelocity.GetSafeNormal())) + mVelocity.GetSafeNormal(); // get unit vector of velocity after collision
			FVector VAfterCollision = UnitVAfterCollision * mVelocity.Size() * mCoefficientOfRestitution; // find velocity vector after collision using unit vector & size of velocity vector before collision
			

			if (!VAfterCollision.IsNearlyZero()) mVelocity = VAfterCollision;
			return -FindGravityForce();
		}
		FVector target = mArbritraryPoint - GetActorLocation();
		target -= mVelocity.GetSafeNormal() * mSphere.sRadius;
		target /= 100;		
		float VC = FVector::DotProduct(target, mVelocity) / mVelocity.Size(); // magnitude of VC (velocity needed to reach plane)
		mTimeStep = abs(mTimeStep * (VC / mVelocity.Size()));
		if (mTimeStep < 0.0f || mTimeStep > 0.5f) mTimeStep = mDefaultTimeStep; // no collision this step
		
		
		return FVector{ 0.0f,0.0f,0.0f };
	}

	return FVector{ 0.0f,0.0f,0.0f };
}


