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

	DecomposeMesh(mVertexPositions);
	

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

ConvexHull APhysicsObject::CreateConvexHull(const TArray<FVector> points)
{
	if (points.Num() <= 0) return; // avoid trying to create a non zero convex hull
	TArray<FVector> tempPointSet = points; // only check points from this point set
	ConvexHull newHull;
	FVector centroid{ 0.0f, 0.0f, 0.0f };
	// find centre point
	for (int i = 0; i < tempPointSet.Num(); i++)
	{
		centroid += tempPointSet[i];
	}

	centroid = centroid / tempPointSet.Num();

	newHull.centroid = centroid;

	// find six extremal points

	Plane checkFace[6];
	checkFace[0].normal = FVector{0.0f, 0.0f, 1.0f}; // up
	checkFace[0].pointOnPlane = newHull.centroid;
	checkFace[1].normal = FVector{ 0.0f, 0.0f, -1.0f }; // down
	checkFace[1].pointOnPlane = newHull.centroid;
	checkFace[2].normal = FVector{ 0.0f, 1.0f, 0.0f }; // front
	checkFace[2].pointOnPlane = newHull.centroid;
	checkFace[3].normal = FVector{ 0.0f, -1.0f, 0.0f }; // behind
	checkFace[3].pointOnPlane = newHull.centroid;
	checkFace[4].normal = FVector{ 1.0f, 0.0f, 0.0f }; // right
	checkFace[4].pointOnPlane = newHull.centroid;
	checkFace[5].normal = FVector{ -1.0f, 0.0f, 0.0f }; // left
	checkFace[5].pointOnPlane = newHull.centroid;

	TArray<FVector> extremalPoints;
	for (int i = 0; i < 6; i++)
	{
		extremalPoints.Add(newHull.centroid);
	}

	for (int i = 0; i < tempPointSet.Num(); i++)
	{
		for (int j = 0; j < extremalPoints.Num(); j++)
		{
			if (IsInFrontOfPlane(tempPointSet[i], checkFace[j])) {
				// find if new point is further than currently checked point
				FVector P = tempPointSet[i] - checkFace[j].pointOnPlane;
				float newDistance = abs(FVector::DotProduct(P, checkFace[j].normal));
				P = extremalPoints[j] - checkFace[j].pointOnPlane;
				float oldDistance = abs(FVector::DotProduct(P, checkFace[j].normal));

				if (newDistance > oldDistance) extremalPoints[j] = tempPointSet[i]; // if new distance is greater than old distance, set new extremal point
			}
		}
	}

	for (int i = 0; i < extremalPoints.Num(); i++)
	{
		// add extremal points to hull (these points will definitely be a part of the final hull)
		newHull.points.Add(extremalPoints[i]);
	}

	ConstructFaces(newHull);

	// remove points inside current hull from inspection
	for (int i = 0; i < tempPointSet.Num(); i++)
	{
		int numFacesBehind = 0;
		for (int j = 0; j < newHull.faces.Num(); j++)
		{
			// create a plane from face
			Plane temp;
			temp.normal = FVector::CrossProduct(newHull.faces[j].a.edgeVector, newHull.faces[j].b.edgeVector);
			temp.normal.Normalize();
			temp.pointOnPlane = newHull.faces[j].a.b;
			if (!IsInFrontOfPlane(tempPointSet[i], temp)) numFacesBehind++;
		}
		if (numFacesBehind >= newHull.faces.Num()) tempPointSet.Remove(tempPointSet[i]); // if the point is behind all faces, remove from consideration
	}

	extremalPoints.Empty();
	// keep adding new points to hull until no points exist outside of hull
	bool pointsExist;
	do {
		// find extremal points using each face as a new plane

		for (int i = 0; i < newHull.faces.Num(); i++) 
		{
			extremalPoints.Add(newHull.faces[i].a.b);
		}

		for (int i = 0; i < tempPointSet.Num(); i++)
		{
			// go through each new face
			for (int j = 0; j < newHull.faces.Num(); j++)
			{
				// only check newly created faces (checking the same face twice for an extremal point is redundant and could create a non convex hull)
				if (newHull.faces[j].isNew) {
					newHull.faces[j].isNew = false;
					Plane temp;
					temp.normal = FVector::CrossProduct(newHull.faces[j].a.edgeVector, newHull.faces[j].b.edgeVector);
					temp.normal.Normalize();
					temp.pointOnPlane = newHull.faces[j].a.b;
					if (IsInFrontOfPlane(tempPointSet[i], temp)) {
						FVector P = tempPointSet[i] - checkFace[j].pointOnPlane;
						float newDistance = abs(FVector::DotProduct(P, checkFace[j].normal));
						P = extremalPoints[j] - checkFace[j].pointOnPlane;
						float oldDistance = abs(FVector::DotProduct(P, checkFace[j].normal));

						if (newDistance > oldDistance) extremalPoints[j] = tempPointSet[i];
					}
				}
			}
		}

		int numberOfPointsInHull = newHull.points.Num();

		// add extremal points to hull
		for (int i = 0; i < extremalPoints.Num(); i++)
		{
			if (extremalPoints[i] == newHull.faces[i].a.b) continue;
			newHull.points.Add(extremalPoints[i]);
		}

		// there are no extremal points to add, hence we have a completed hull
		if (numberOfPointsInHull == newHull.points.Num()) {
			pointsExist = false;
			continue;
		}

		ConstructFaces(newHull);

		for (int i = 0; i < tempPointSet.Num(); i++)
		{
			int numFacesBehind = 0;
			for (int j = 0; j < newHull.faces.Num(); j++)
			{
				Plane temp;
				temp.normal = FVector::CrossProduct(newHull.faces[j].a.edgeVector, newHull.faces[j].b.edgeVector);
				temp.normal.Normalize();
				temp.pointOnPlane = newHull.faces[j].a.b;
				if (!IsInFrontOfPlane(tempPointSet[i], temp)) numFacesBehind++;
			}
			if (numFacesBehind >= newHull.faces.Num()) tempPointSet.Remove(tempPointSet[i]);
		}

	} while (pointsExist);
	
	return newHull;
}

void APhysicsObject::ConstructFaces(ConvexHull convexHull)
{
}

void APhysicsObject::DecomposeMesh(const TArray<FVector> points)
{
	// create an approximate sub set of convex hulls that define the meshes collision volumes
	ConvexHull tempHull = CreateConvexHull(points); // create a convex hull of full mesh
	float concavity = FindConcavity(tempHull, points); // find concavity
	if (concavity <= mMaxConcavity) { // if starting mesh is already convex, we can simply push it to the mesh vector as its only element
		mMeshes.push_back(tempHull);
		// create OBB?


		return; // no need to continue function past this point
	}
	else { 
		// find an inflexive vertex
		Plane hyperPlane = FindInflexFacePlane(tempHull, points);
		TArray<FVector> first, second; // two new point sets will be used to create two new convex hulls recursively

		// find all points in front of hyperplane, add to first point set

		for (int i = 0; i < points.Num(); i++)
		{
			if (IsInFrontOfPlane(points[i], hyperPlane)) first.Add(points[i]);
		}

		// find all points behind the hyperplane, add to second point set

		hyperPlane.normal *= -1; // reverse direction of plane normal (so we can use same in front of plane checking function for behind the plane)

		for (int i = 0; i < points.Num(); i++)
		{
			if (IsInFrontOfPlane(points[i], hyperPlane)) second.Add(points[i]);
		}

		// call function for each point set

		DecomposeMesh(first);
		DecomposeMesh(second);
		return; // function can finish here
	}
}

bool APhysicsObject::IsInFrontOfPlane(FVector point, Plane plane)
{
	return false;
}

Plane APhysicsObject::FindInflexFacePlane(ConvexHull convexHull, const TArray<FVector> points)
{

	return;
}

float APhysicsObject::FindConcavity(ConvexHull convexHull, const TArray<FVector> points)
{

	return false;
}

OBB APhysicsObject::GenerateOBB(ConvexHull convexHull)
{
	return OBB();
}

void APhysicsObject::OrientOBB()
{
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


