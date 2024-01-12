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
	
	for (int i = 0; i < mMeshes.Num(); i++)
	{
		mMeshes[i].startPos = GetActorLocation();
		mMeshes[i].offsetFromStart = GetActorLocation();
	}

	GenerateSphere();

	// Find a predicted sphere from generated sphere
	mPredictedSphere.center = mSphere.center;
	mPredictedSphere.radius = mSphere.radius;

	// set visuals
	FVector oldPos = GetActorLocation();
	SetActorLocation(mSphere.center);
	mBoundingVisualSphere->SetSphereRadius(mSphere.radius);
	FVector newPos = GetActorLocation();

	FVector offset = newPos - oldPos;

	if (!offset.IsNearlyZero()) mObjectMesh->SetWorldLocation(GetActorLocation() - offset);

	mDisplacement = mSphere.center;

	float calcRadius = mSphere.radius / 100; // convert to meters

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

		Face newFace;
		Edge newEdgeA, newEdgeB, newEdgeC;
		newEdgeA.a = P2UVector(vertices[0]);
		newEdgeA.b = P2UVector(vertices[1]);
		newEdgeA.edgeVector = newEdgeA.b - newEdgeA.a;
		newEdgeB.a = P2UVector(vertices[0]);
		newEdgeB.b = P2UVector(vertices[2]);
		newEdgeB.edgeVector = newEdgeB.b - newEdgeB.a;
		newEdgeC.a = P2UVector(vertices[2]);
		newEdgeC.b = P2UVector(vertices[1]);
		newEdgeC.edgeVector = newEdgeC.b - newEdgeC.a;
		newFace.a = newEdgeA;
		newFace.b = newEdgeB;
		newFace.c = newEdgeC;
		mFaceGeometry.Add(newFace);


		for (PxU32 i = 0; i < vertexCount; i++) {
			mVertexPositions.Add(transform.TransformPosition(P2UVector(vertices[i])));
		}
	}


	return true;
}

FVector APhysicsObject::FindCentreOfGravity()
{
	FVector centre{ 0.0f, 0.0f, 0.0f };
	for (int i = 0; i < mMeshes.Num(); i++)
	{
		centre += mMeshes[i].centroid;
	}
	centre /= mMeshes.Num();
	return centre;
}

Tensor APhysicsObject::FindInertialTensor()
{
	float iXX{ 0 }, iYY{ 0 }, iZZ{ 0 }, iXY{ 0 }, iXZ{ 0 }, iYZ{ 0 };


	// set up i values for inertia tensor
	for (int i = 0; i < mMeshes.Num(); i++)
	{
		Tensor localInertia = FindClosestMatchingTensor(mMeshes[i]);

		FVector localGravCoords = mMeshes[i].centroid - mCentreOfGravity; // vector from the centre of gravity to the centroid of the mesh

		iXX += localInertia.tensor[0] + (mMass / mMeshes.Num()) * ((localGravCoords.Y * localGravCoords.Y) + (localGravCoords.Z * localGravCoords.Z));
		iYY += localInertia.tensor[4] + (mMass / mMeshes.Num()) * ((localGravCoords.Z * localGravCoords.Z) + (localGravCoords.X * localGravCoords.X));
		iZZ += localInertia.tensor[8] + (mMass / mMeshes.Num()) * ((localGravCoords.X * localGravCoords.X) + (localGravCoords.Y * localGravCoords.Y));
		iXY += (mMass / mMeshes.Num()) * (localGravCoords.X * localGravCoords.Y);
		iXZ += (mMass / mMeshes.Num()) * (localGravCoords.X * localGravCoords.Z);
		iYZ += (mMass / mMeshes.Num()) * (localGravCoords.Y * localGravCoords.Z);
	}

	Tensor finalTensor;
	finalTensor.tensor[0] = iXX;
	finalTensor.tensor[1] = -iXY;
	finalTensor.tensor[2] = -iXZ;
	finalTensor.tensor[3] = -iXY;
	finalTensor.tensor[4] = iYY;
	finalTensor.tensor[5] = -iYZ;
	finalTensor.tensor[6] = -iXZ;
	finalTensor.tensor[7] = -iYZ;
	finalTensor.tensor[8] = iZZ;


	return finalTensor;
}

Tensor APhysicsObject::FindClosestMatchingTensor(ConvexHull convexHull)
{
	float averageAngularDifference = 0.0f; // find average angles between faces of the hull 
	int32 num{ 0 };
	for (int i = 0; i < convexHull.faces.Num(); i ++)
	{
		TArray<Face> sharingFaces;
		for (int j = 1; j < convexHull.faces.Num(); j++)
		{
			if (i == j) continue; // dont test faces against themselves
			// create an array of all faces that share an edge with faces[i]
			if (convexHull.faces[i].a == convexHull.faces[j].a || convexHull.faces[i].a == convexHull.faces[j].b ||
				convexHull.faces[i].a == convexHull.faces[j].c) {
				if (!sharingFaces.Contains(convexHull.faces[j])) 
					sharingFaces.Add(convexHull.faces[j]); // make sure we dont have redundant faces
				continue;
			}
			else if (convexHull.faces[i].b == convexHull.faces[j].a || convexHull.faces[i].b == convexHull.faces[j].b || 
				convexHull.faces[i].b == convexHull.faces[j].c) {
				if (!sharingFaces.Contains(convexHull.faces[j])) sharingFaces.Add(convexHull.faces[j]);
				continue;
			}
			else if (convexHull.faces[i].c == convexHull.faces[j].a || convexHull.faces[i].c == convexHull.faces[j].b || 
				convexHull.faces[i].c == convexHull.faces[j].c) {
				if (!sharingFaces.Contains(convexHull.faces[j])) sharingFaces.Add(convexHull.faces[j]);
				continue;
			}
		}

		for (int j = 0; j < sharingFaces.Num(); j++)
		{
			FVector firstNormal, secondNormal;
			firstNormal = FVector::CrossProduct(convexHull.faces[i].a.edgeVector, convexHull.faces[i].b.edgeVector);
			secondNormal = FVector::CrossProduct(convexHull.faces[i].a.edgeVector, convexHull.faces[i].b.edgeVector);

			float angleBetweenNormals;

			angleBetweenNormals = (FVector::DotProduct(firstNormal, secondNormal)) / (firstNormal.Size() * secondNormal.Size());
			if (fabs(angleBetweenNormals) > 0.999999) continue;
			angleBetweenNormals = acos(angleBetweenNormals);
			angleBetweenNormals = (angleBetweenNormals * 180.0f) / PI; // convert to degrees
			averageAngularDifference += angleBetweenNormals;
			num++;
		}
	}

	averageAngularDifference /= num;

	int minY{ 0 }, maxY{ 0 }, minX{ 0 }, maxX{ 0 }, minZ{ 0 }, maxZ{ 0 };
	FVector largestRadius{ 0.0f, 0.0f, 0.0f };
	
	for (int i = 0; convexHull.points.Num(); i++)
	{
		// find extremal x and y then use the distances as length and width
		if (convexHull.points[i].X < convexHull.points[minX].X) minX = i;
		if (convexHull.points[i].X > convexHull.points[maxX].X) maxX = i;
		if (convexHull.points[i].Y < convexHull.points[minY].Y) minY = i;
		if (convexHull.points[i].Y > convexHull.points[minY].Y) maxY = i;
		if (convexHull.points[i].Z < convexHull.points[minZ].Z) minZ = i;
		if (convexHull.points[i].Z > convexHull.points[minZ].Z) maxZ = i;

		float newDistance = FVector::Distance(convexHull.points[i], convexHull.centroid);
		float oldDistance = FVector::Distance(largestRadius, convexHull.centroid);

		if (newDistance > oldDistance) largestRadius = convexHull.points[i];
		

	}

	float width = abs(FVector::Distance(convexHull.points[minY], convexHull.points[maxY]));
	float length = abs(FVector::Distance(convexHull.points[minX], convexHull.points[maxX]));
	float height = abs(FVector::Distance(convexHull.points[minZ], convexHull.points[maxZ]));
	float radius = abs(FVector::Distance(convexHull.centroid, largestRadius));

	Tensor newTensor;

	if (averageAngularDifference >= 90.0f) {
		// use cuboid tensor
		newTensor.tensor[0] = (1 / 12) * mMass * ((width * width) * (length * length)); // IXX
		newTensor.tensor[4] = (1 / 12) * mMass * ((height * height) * (length * length)); // IYY
		newTensor.tensor[8] = (1 / 12) * mMass * ((width * width) * (height * height)); // IZZ
	}
	else {
		// use sphere tensor
		newTensor.tensor[0] = (2 / 5) * mMass * (radius * radius);
		newTensor.tensor[4] = (2 / 5) * mMass * (radius * radius);
		newTensor.tensor[8] = (2 / 5) * mMass * (radius * radius);
	}

	return newTensor;
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
	mSphere.center = sphereCenter;
	mSphere.radius = sphereRadius;


}

void APhysicsObject::SphereOfSphereAndPoint(FVector point)
{
	// find distance between input point and center of bounding sphere
	FVector distanceVector = point - mSphere.center;
	float squareDistance = FVector::DotProduct(distanceVector, distanceVector);

	if (squareDistance > mSphere.radius * mSphere.radius) {
		// point is outside sphere so update sphere bounds
		float distance = sqrt(squareDistance);
		float newRadius = (mSphere.radius + distance) * 0.5f;
		float k = (newRadius - mSphere.radius) / distance;
		mSphere.radius = newRadius;
		mSphere.center += distanceVector * k;
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

ConvexHull APhysicsObject::CreateConvexHull(const TArray<FVector> &points)
{
	TArray<FVector> tempPointSet = points; // only check points from this point set
	ConvexHull newHull;
	if (points.Num() <= 0) return newHull; // avoid trying to create a non zero convex hull
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
				float newDistance = abs(FVector::DotProduct(tempPointSet[i], checkFace[j].normal));
				float oldDistance = abs(FVector::DotProduct(extremalPoints[j], checkFace[j].normal));

				if (newDistance > oldDistance) extremalPoints[j] = tempPointSet[i]; // if new distance is greater than old distance, set new extremal point
			}
		}
	}


	for (int i = 0; i < extremalPoints.Num(); i++)
	{
		// add extremal points to hull (these points will definitely be a part of the final hull)
		newHull.points.Add(extremalPoints[i]);
	}
	newHull.faces = ConstructFaces(newHull.points);

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
		if (numFacesBehind >= newHull.faces.Num()) tempPointSet.RemoveAt(i); // if the point is behind all faces, remove from consideration
	}
	
	extremalPoints.Empty();
	// keep adding new points to hull until no points exist outside of hull
	bool pointsExist = true;
	int count = 0;
	do {
		// find extremal points using each face as a new plane
		count++;
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
						float newDistance = abs(FVector::DotProduct(tempPointSet[i], checkFace[j].normal));
						float oldDistance = abs(FVector::DotProduct(extremalPoints[j], checkFace[j].normal));

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

		newHull.faces = ConstructFaces(newHull.points);

		for (int i = 0; i < tempPointSet.Num(); i++)
		{
			int32 numFacesBehind = 0;
			for (int j = 0; j < newHull.faces.Num(); j++)
			{
				Plane temp;
				temp.normal = FVector::CrossProduct(newHull.faces[j].a.edgeVector, newHull.faces[j].b.edgeVector);
				temp.normal.Normalize();
				temp.pointOnPlane = newHull.faces[j].a.b;
				if (!IsInFrontOfPlane(tempPointSet[i], temp)) numFacesBehind++;
			}
			if (numFacesBehind >= newHull.faces.Num()) tempPointSet.RemoveAt(i);
		}

	} while (pointsExist);

	FVector centrePoint{ 0.0f, 0.0f, 0.0f };
	for (int i = 0; i < newHull.points.Num(); i++) 
	{
		centrePoint += newHull.points[i];
	}
	centrePoint = centrePoint / newHull.points.Num();

	newHull.centroid = centrePoint;

	return newHull;
}

TArray<Face> APhysicsObject::ConstructFaces(const TArray<FVector>& points)
{
	// CONSTRUCT EDGES
	// go through every point in the hull and construct four edges for each vertex (using closest vertices)
	TArray<Edge> edges; // store all edges in an array
	for (int i = 0; i < points.Num(); i++)
	{
		TArray<FVector> closestPoints;
		// only construct edges from 1st vertex onwards (first vertex will not form an edge with itself)
		for (int j = 1; j < points.Num(); j++)
		{
			if (points[i] == points[j]) continue;

			if (closestPoints.Num() < 4) closestPoints.Add(points[j]);
			else {
				for (int k = 0; k < closestPoints.Num(); k++)
				{
					// find distances between current closest point and new point
					float newDistance = FVector::Distance(points[j], points[i]);
					float oldDistance = FVector::Distance(closestPoints[k], points[i]);

					// the lower distance is the new closest point
					if (newDistance < oldDistance) { 
						closestPoints[k] = points[j]; 
						continue;
					}
				}
			}
		}
		for (int k = 0; k < closestPoints.Num(); k++)
		{
			// add new edges for each closest point computed
			Edge newEdge;
			newEdge.a = points[i];
			newEdge.b = closestPoints[k];
			if (!edges.Contains(newEdge)) edges.Add(newEdge);
		}
	}

	for (int i = 0; i < edges.Num(); i++)
	{
		edges[i].edgeVector = edges[i].b - edges[i].a;
	}

	// CONSTRUCT FACES

	TArray<Face> faces;

	for (int i = 0; i < edges.Num(); i++)
	{
		FVector V1 = edges[i].a;
		FVector V2 = edges[i].b;
		TArray<FVector> potentialV3;
		TArray<Edge> potentialV3Edge;
		for (int j = 1; j < edges.Num(); j++)
		{
			if (edges[j].a == V2)
			{
				potentialV3.Add(edges[j].b);
				potentialV3Edge.Add(edges[j]);
			}
			else if (edges[j].b == V2)
			{
				potentialV3.Add(edges[j].a);
				potentialV3Edge.Add(edges[j]);
			}
		}
		// check for edges that contain V1-V3
		for (int j = 0; j < potentialV3.Num(); j++)
		{
			for (int k = 1; k < edges.Num(); k++)
			{
				if (edges[k].a == potentialV3[j] && edges[k].b == V1) {
					// construct new face using V1, V2 & V3
					Face newFace;
					newFace.a = edges[i];
					newFace.b = edges[k];
					newFace.c = potentialV3Edge[j];
					if (!faces.Contains(newFace)) faces.Add(newFace);
				}
				else if (edges[k].b == potentialV3[j] && edges[k].a == V1) {
					Face newFace;
					newFace.a = edges[i];
					newFace.b = edges[k];
					newFace.c = potentialV3Edge[j];
					if (!faces.Contains(newFace)) faces.Add(newFace);
				}
				
			}
		}

		// repeat prior steps for V1
		potentialV3.Empty();
		potentialV3Edge.Empty();

		for (int j = 1; j < edges.Num(); j++)
		{
			if (edges[j].a == V1)
			{
				potentialV3.Add(edges[j].b);
				potentialV3Edge.Add(edges[j]);
			}
			else if (edges[j].b == V1)
			{
				potentialV3.Add(edges[j].a);
				potentialV3Edge.Add(edges[j]);
			}
		}
		// check for edges that contain V2-V3
		for (int j = 0; j < potentialV3.Num(); j++)
		{
			for (int k = 1; k < edges.Num(); k++)
			{
				if (edges[k].a == potentialV3[j] && edges[k].b == V2) {
					// construct new face using V1, V2 & V3
					Face newFace;
					newFace.a = edges[i];
					newFace.b = edges[k];
					newFace.c = potentialV3Edge[j];
					if (!faces.Contains(newFace)) faces.Add(newFace);
				}
				else if (edges[k].b == potentialV3[j] && edges[k].a == V2) {
					Face newFace;
					newFace.a = edges[i];
					newFace.b = edges[k];
					newFace.c = potentialV3Edge[j];
					if(!faces.Contains(newFace)) faces.Add(newFace);
				}

			}
		}
	}
	return faces;
}

void APhysicsObject::DecomposeMesh(const TArray<FVector> &points)
{
	// create an approximate sub set of convex hulls that define the meshes collision volumes

	ConvexHull tempHull = CreateConvexHull(points); // create a convex hull of full mesh
	
	float concavity = FindConcavity(tempHull, points); // find concavity
	

	if (concavity <= mMaxConcavity) { // if starting mesh is already convex, we can simply push it to the mesh vector as its only element
		mMeshes.Add(tempHull);


		return; // no need to continue function past this point
	}
	else { 
		// find an inflexive vertex
		Plane hyperPlane = FindInflexFacePlane(points);
		if (hyperPlane.normal.IsZero()) return; // we dont want to try to create new hulls from zero planes (this means the above function couldn't find
												// an inflex face
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
	FVector P = point - plane.pointOnPlane;

	float dist = FVector::DotProduct(plane.normal, P) / P.Size();

	if (dist > 0.0f) return true; // point is in front of plane
	return false; // point is behind or on plane (either way return false, its not in front of plane if its on the plane technically)
}

Plane APhysicsObject::FindInflexFacePlane(const TArray<FVector> &points)
{
	// for each face
	// find every face that share edges
	// find surface normal of each face and reverse it
	// if the angle between any of the normals is higher than 180 degrees, return current face plane

	for (int i = 0; i < mFaceGeometry.Num(); i++)
	{
		TArray<Face> sharingFaces;
		for (int j = 1; j < mFaceGeometry.Num(); j++)
		{
			if (i == j) continue; // dont test faces against themselves
			// create an array of all faces that share an edge with faces[i]
			if (mFaceGeometry[i].a == mFaceGeometry[j].a || mFaceGeometry[i].a == mFaceGeometry[j].b || mFaceGeometry[i].a == mFaceGeometry[j].c) {
				if (!sharingFaces.Contains(mFaceGeometry[j])) sharingFaces.Add(mFaceGeometry[j]); // make sure we dont have redundant faces
				continue;
			}
			else if (mFaceGeometry[i].b == mFaceGeometry[j].a || mFaceGeometry[i].b == mFaceGeometry[j].b || mFaceGeometry[i].b == mFaceGeometry[j].c) {
				if (!sharingFaces.Contains(mFaceGeometry[j])) sharingFaces.Add(mFaceGeometry[j]);
				continue;
			}
			else if (mFaceGeometry[i].c == mFaceGeometry[j].a || mFaceGeometry[i].c == mFaceGeometry[j].b || mFaceGeometry[i].c == mFaceGeometry[j].c) {
				if (!sharingFaces.Contains(mFaceGeometry[j])) sharingFaces.Add(mFaceGeometry[j]);
				continue;
			}
		}
		Plane inflexPlane;
		for (int j = 0; j < sharingFaces.Num(); j++)
		{
			Plane first, second;
			first.normal = FVector::CrossProduct(mFaceGeometry[i].a.edgeVector, mFaceGeometry[i].b.edgeVector).GetSafeNormal();
			first.pointOnPlane = mFaceGeometry[i].a.a;

			second.normal = FVector::CrossProduct(sharingFaces[j].a.edgeVector, sharingFaces[j].b.edgeVector).GetSafeNormal();
			second.pointOnPlane = sharingFaces[j].a.a;

			first.normal *= -1;
			second.normal *= -1;

			// find angle between normals

			float cosX = FVector::DotProduct(first.normal, second.normal); // both normals are unit vectors so we dont divide by magnitude

			if (cosX > 1.0f || cosX < 0.0f) continue; // avoid NAN values
			float angle = acos(cosX); // angle in radians

			angle = angle * 180.0f;
			angle = angle / PI;

			if (angle > 180.0f)
			{
				inflexPlane = first;
				return inflexPlane;
			}

		}
	}
	Plane errorPlane;
	errorPlane.normal = { 0.0f, 0.0f, 0.0f };
	errorPlane.pointOnPlane = { 0.0f, 0.0f, 0.0f };
	return errorPlane;
}

float APhysicsObject::FindConcavity(ConvexHull convexHull, const TArray<FVector> &points)
{
	// go through each point

	float mostConcavePointValue = 0.0f;
	for (int i = 0; i < mFaceGeometry.Num(); i ++) // go through every triplet of points (triangles)
	{
		// get surface normal using cross product
		FVector surfaceNormal = FVector::CrossProduct(mFaceGeometry[i].a.edgeVector, mFaceGeometry[i].b.edgeVector);
		surfaceNormal.Normalize();

		
		Ray surfaceRay;
		surfaceRay.direction = surfaceNormal;
		surfaceRay.origin = (mFaceGeometry[i].a.edgeVector + mFaceGeometry[i].b.edgeVector + mFaceGeometry[i].c.edgeVector) / 3; // use centre of surface as origin
		surfaceRay.origin = surfaceRay.origin + mFaceGeometry[i].a.a;
		
		for (int j = 0; j < convexHull.faces.Num(); j++)
		{
			Plane facePlane;
			facePlane.normal = FVector::CrossProduct(convexHull.faces[j].a.edgeVector, convexHull.faces[j].b.edgeVector);
			facePlane.normal.Normalize();
			facePlane.pointOnPlane = convexHull.faces[j].a.a;

			if (IsInFrontOfPlane(surfaceRay.origin, facePlane)) continue; // rays can only project forwards, so if plane is behind origin, ray will never intersect
			
			float projection = FVector::DotProduct(facePlane.normal, surfaceRay.direction);
			if (abs(projection) > 0.0001f) // make sure projection is not zero, this means the ray is perpendicular to the plane and never intersects
			{
				surfaceRay.t = FVector::DotProduct(facePlane.pointOnPlane - surfaceRay.origin, facePlane.normal) / projection;
				if (surfaceRay.t != 0) continue;

				// we have the intersect point so we can now find points concavity measure

				FVector intersectPoint = surfaceRay.origin + (surfaceRay.direction * surfaceRay.t);

				for (int k = 0; k < 3; k++) // find most concave point in the triplet
				{
					float concavity = FVector::Distance(points[i + k], intersectPoint);
					if (concavity > mostConcavePointValue) mostConcavePointValue = concavity;
				}
			}
		}

	}

	return mostConcavePointValue; // return the most concave surface in the set of points
}

// Called every frame
void APhysicsObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//if (Hit) return;

	StepSimulation(DeltaTime);
	//mNormalForce = { 0.0f, 0.0f ,0.0f };
	//mNormalForce += CheckForPlaneCollision();

	GEngine->AddOnScreenDebugMessage(1000, 15.0f, FColor::Yellow, mMeshes[0].addWorldOffset.ToString() + " Offset");

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APhysicsObject::StaticClass(), FoundActors);
	for (int i = 0; i < FoundActors.Num(); i++)
	{
		if (Cast<APhysicsObject>(FoundActors[i]) == nullptr || FoundActors[i] == this) continue;
		//initial bounding sphere check
		if (SphereOnSphereCheck(Cast<APhysicsObject>(FoundActors[i])->GetSphere())) {
			// if bounding spheres are intersecting check for intersecting hulls
			mHitPosition = SeparatingAxisTest(Cast<APhysicsObject>(FoundActors[i])->mMeshes);
			if (mHitPosition.Num() <= 0) continue; // no collisions
			else {
				// collision response here
				GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::Yellow, "Collision");
				FindCollisionResponseMove(Cast<APhysicsObject>(FoundActors[i]));
			}

		}
	}

}

FVector APhysicsObject::FindGravityForce()
{
	if (mAffectedByGravity)
	{
		long double G = 6.67430 * powf(10, -11);
		float EarthMass = 5.98 * powf(10, 24);
		long double FGrav = G * mMass * EarthMass;
		float DistanceToEarthCentre = 6.38 * powf(10, 6);
		FGrav /= (DistanceToEarthCentre * DistanceToEarthCentre);
		return FVector(0.0f, 0.0f, -FGrav); // Force of gravity acts downwards
	}
	return FVector{ 0.0f, 0.0f ,0.0f };
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

void APhysicsObject::StepSimulation(float deltaTime)
{
	FVector Vnew; // new velocity at time t + dt
	FVector Snew; // new position at time t + dt
	FVector k1, k2;
	FVector F; // total force
	FVector A; // acceleration
	float dt;

	dt = mTimeStep * deltaTime;

	// Actual step simulation

	// movement
	
	F = FindGravityForce() + mNormalForce + FindWindForce(mWindSpeed) + FindDragForce(mVelocity);

	A = F / mMass;

	k1 = dt * A;

	F = FindGravityForce() + mNormalForce + FindWindForce(mWindSpeed) + FindDragForce(mVelocity + k1);

	A = F / mMass;

	k2 = dt * A;

	Vnew = mVelocity + ((k1 + k2) / 2);

	Snew = mDisplacement + (Vnew * dt * 100); // convert to centimeters

	mVelocity = Vnew;
	mDisplacement = Snew;

	SetActorLocation(mDisplacement);
	mSphere.center = mDisplacement;
	for (int i = 0; i < mMeshes.Num(); i++)
	{
		mMeshes[i].offsetFromStart += (Vnew* dt * 100);
		mMeshes[i].addWorldOffset = mMeshes[i].offsetFromStart - mMeshes[i].startPos;
	}

	// rotation

	float mag;

	FMatrix interiaMat;
	interiaMat.SetColumn(0, FVector(mIntertialTensor.tensor[0], mIntertialTensor.tensor[3], mIntertialTensor.tensor[6]));
	interiaMat.SetColumn(1, FVector(mIntertialTensor.tensor[1], mIntertialTensor.tensor[4], mIntertialTensor.tensor[7]));
	interiaMat.SetColumn(2, FVector(mIntertialTensor.tensor[2], mIntertialTensor.tensor[5], mIntertialTensor.tensor[8]));

	mAngularVelocity += interiaMat.Inverse().TransformVector((mMoments - (mAngularVelocity ^ (interiaMat.TransformVector(mAngularVelocity)))) * (0.5f * dt));
		

	Quaternion q;
	q.QuatVecMultiply(mOrientation, mAngularVelocity * (0.5f * dt));
	mOrientation = mOrientation + q;

	mag = q.Magnitude();
	if (mag != 0) q = q / mag;

	FRotator u; // find euler angles
	u = QuaternionToEuler(q).ToOrientationRotator();
	SetActorRotation(u);

}

FVector APhysicsObject::CheckForPlaneCollision()
{

	if (mVelocity.Size() <= 0.0f) return { 0.0f,0.0f,0.0f };
	float magnitudeNormal = mSurfaceNormal.Size();

	FVector planeToSelfVector = GetActorLocation() - mArbritraryPoint;

	float d = FVector::DotProduct(mSurfaceNormal, planeToSelfVector) / magnitudeNormal; // find magnitude of d 

	FVector capsuleLength = mCapsule.endPoint - mCapsule.startPoint; // start point to end point vector


	if (d < capsuleLength.Size() + mCapsule.radius) // if true a collision will accur and VC will give the point of collision
	{
		// if this is a direct collision don't bother finding timestep
		if (d <= mSphere.radius)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Collision"));
			FVector UnitVAfterCollision = 2 * mSurfaceNormal *
				(FVector::DotProduct(mSurfaceNormal, -mVelocity.GetSafeNormal())) + mVelocity.GetSafeNormal(); // get unit vector of velocity after collision
			FVector VAfterCollision = UnitVAfterCollision * mVelocity.Size() * mCoefficientOfRestitution; // find velocity vector after collision using unit vector & size of velocity vector before collision
			

			if (!VAfterCollision.IsNearlyZero()) mVelocity = VAfterCollision;
			return -FindGravityForce();
		}
		FVector target = mArbritraryPoint - GetActorLocation();
		target -= mVelocity.GetSafeNormal() * mSphere.radius;
		target /= 100;		
		float VC = FVector::DotProduct(target, mVelocity) / mVelocity.Size(); // magnitude of VC (velocity needed to reach plane)
		mTimeStep = abs(mTimeStep * (VC / mVelocity.Size()));
		if (mTimeStep < 0.0f || mTimeStep > 0.5f) mTimeStep = mDefaultTimeStep; // no collision this step
		
		
		return FVector{ 0.0f,0.0f,0.0f };
	}

	return FVector{ 0.0f,0.0f,0.0f };
}

TArray<ConvexHull> APhysicsObject::SeparatingAxisTest(const TArray<ConvexHull>& other)
{
	// go through each convex hull of both meshes and conduct separating axis test to find overlapping hulls
	// test every edge-edge permutation as possible collision axes
	TArray<Plane> testPlanes;
	TArray<Edge> firstMeshEdges;
	TArray<Edge> secondMeshEdges;
	// get all unique edges from first mesh
	for (int i = 0; i < mMeshes.Num(); i++)
	{
		for (int j = 0; j < mMeshes[i].faces.Num(); j++)
		{
			if (!firstMeshEdges.Contains(mMeshes[i].faces[j].a)) firstMeshEdges.Add(mMeshes[i].faces[j].a);
			if (!firstMeshEdges.Contains(mMeshes[i].faces[j].b)) firstMeshEdges.Add(mMeshes[i].faces[j].b);
			if (!firstMeshEdges.Contains(mMeshes[i].faces[j].c)) firstMeshEdges.Add(mMeshes[i].faces[j].c);
		}
	}

	// get all unique edges from second mesh
	for (int i = 0; i < other.Num(); i++)
	{
		for (int j = 0; j < other[i].faces.Num(); j++)
		{
			if (!secondMeshEdges.Contains(other[i].faces[j].a)) secondMeshEdges.Add(other[i].faces[j].a);
			if (!secondMeshEdges.Contains(other[i].faces[j].b)) secondMeshEdges.Add(other[i].faces[j].b);
			if (!secondMeshEdges.Contains(other[i].faces[j].c)) secondMeshEdges.Add(other[i].faces[j].c);
		}
	}

	// get all unique planes from edge cross products
	for (int i = 0; i < firstMeshEdges.Num(); i++)
	{
		for (int j = 0; j < secondMeshEdges.Num(); j++)
		{
			Plane newPlane;
			newPlane.normal = FVector::CrossProduct(firstMeshEdges[i].b - firstMeshEdges[i].a, secondMeshEdges[j].b - secondMeshEdges[j].a);
			newPlane.normal.Normalize();
			newPlane.pointOnPlane = FVector(0.0f, 0.0f, 0.0f);
			// in SAT (Separating Axis Test) normals are the only thing being tested, hence all planes containing the same normal value will be ignored
			if (!testPlanes.Contains(newPlane) && !newPlane.normal.IsZero()) {
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, newPlane.normal.ToString() + " Plane normal");
				newPlane.pointOnPlane = firstMeshEdges[i].a;
				testPlanes.Add(newPlane);
			}
		}
	}
	

	TArray<ConvexHull> overlappingHulls;
	for (int i = 0; i < mMeshes.Num(); i++)
	{
		for (int j = 0; j < other.Num(); j++)
		{
			for (int p = 0; p < testPlanes.Num(); p++)
			{
				// use normal as perpendicular axis to plane (as its the normal)

				// go through points to find supporting point that is farthest distance from centroid along perpendicular axis

				int32 indexFarthestPoint = 0;
				FVector farthestPoint{ 0.0f, 0.0f, 0.0f };
				FVector centroidA{ 0.0f, 0.0f, 0.0f };
				for (int k = 0; k < mMeshes[i].points.Num(); k++)
				{
					// calculate projected vector onto plane normal
					centroidA += (mMeshes[i].points[k] + mMeshes[i].addWorldOffset);
					FVector projectedVector = testPlanes[p].normal * (FVector::DotProduct((mMeshes[i].points[k] + mMeshes[i].addWorldOffset), testPlanes[p].normal) /
						FVector::DotProduct(testPlanes[p].normal, testPlanes[p].normal));
					if (!projectedVector.IsNearlyZero() && projectedVector.Size() > farthestPoint.Size()) { 
						farthestPoint = projectedVector;
						indexFarthestPoint = k;
					}
				}
				centroidA = centroidA / mMeshes[i].points.Num();
				// projected centroid onto normal
				FVector projectedCentroidA = testPlanes[p].normal * (FVector::DotProduct(centroidA, testPlanes[p].normal) /
					FVector::DotProduct(testPlanes[p].normal, testPlanes[p].normal));

				// first radius is the distance between projected centroid and farthest point
				//FVector rA = mMeshes[i].points[indexFarthestPoint] - centroidA;
				//rA = testPlanes[p].normal* (FVector::DotProduct(rA, testPlanes[p].normal) /
				//	FVector::DotProduct(testPlanes[p].normal, testPlanes[p].normal));

				float rA = FVector::Distance(projectedCentroidA, farthestPoint);

				farthestPoint = { 0.0f, 0.0f, 0.0f };
				FVector centroidB = { 0.0f, 0.0f,0.0f };
				for (int k = 0; k < other[j].points.Num(); k++)
				{
					centroidB += (other[j].points[k] + other[j].addWorldOffset);
					FVector projectedVector = testPlanes[p].normal * (FVector::DotProduct((other[j].points[k] + other[j].addWorldOffset), testPlanes[p].normal) /
						FVector::DotProduct(testPlanes[p].normal, testPlanes[p].normal));
					if (!projectedVector.IsNearlyZero() && projectedVector.Size() > farthestPoint.Size()) {
						farthestPoint = projectedVector;
						indexFarthestPoint = k;
					}
				}
				centroidB = centroidB / other[j].points.Num();
				FVector projectedCentroidB = testPlanes[p].normal * (FVector::DotProduct(centroidB, testPlanes[p].normal) /
					FVector::DotProduct(testPlanes[p].normal, testPlanes[p].normal));

				//FVector rB = other[j].points[indexFarthestPoint] - centroidB;
				//rB = testPlanes[p].normal * (FVector::DotProduct(rB, testPlanes[p].normal) /
				//	FVector::DotProduct(testPlanes[p].normal, testPlanes[p].normal));

				float rB = FVector::Distance(projectedCentroidB, farthestPoint);

				FVector centroidToCentroid = centroidB - centroidA;

				FVector projectedCentroidToCentroid = testPlanes[p].normal * (FVector::DotProduct(centroidToCentroid, testPlanes[p].normal) /
					FVector::DotProduct(testPlanes[p].normal, testPlanes[p].normal));

				// objects are not disjoint

				if (rA + rB >= centroidToCentroid.Size()) {
					// store overlapping hulls as pairs
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "COLLISION SAT");
					overlappingHulls.Add(mMeshes[i]);
					overlappingHulls.Add(other[j]);
				}
			}
		}
	}
	return overlappingHulls;
}

bool APhysicsObject::SphereOnSphereCheck(const Sphere& other)
{
	float squareDistance = FVector::DistSquared(other.center, mSphere.center);
	float squareRadiiSum = (mSphere.radius + other.radius) * (mSphere.radius + other.radius);

	if (squareRadiiSum >= squareDistance) return true; // collision

	return false; // no collision
}

Quaternion APhysicsObject::EulerToQuaternion(FVector rot)
{
	Quaternion q;

	double roll = (rot.X * PI) / 180.0f; // convert to radians
	double pitch = (rot.Y * PI) / 180.0f;
	double yaw = (rot.Z * PI) / 180.0f;

	double cosYaw, cosPitch, cosRoll, sinYaw, sinPitch, sinRoll;
	double cosYawCosPitch, sinYawSinPitch, cosYawSinPitch, sinYawCosPitch;

	// define helper values (cos/sin roll pitch and yaw)

	cosYaw = cos(0.5f * yaw);
	cosPitch = cos(0.5f * pitch);
	cosRoll = cos(0.5f * roll);
	sinYaw = sin(0.5f * yaw);
	sinPitch = sin(0.5f * pitch);
	sinRoll = sin(0.5f * roll);

	cosYawCosPitch = cosYaw * cosPitch;
	sinYawSinPitch = sinYaw * sinPitch;
	cosYawSinPitch = cosYaw * sinPitch;
	sinYawCosPitch = sinYaw * cosPitch;

	// construct quaternion

	q.w = (float)(cosYawCosPitch * cosRoll + sinYawSinPitch * sinRoll);
	q.x = (float)(cosYawCosPitch * sinRoll - sinYawSinPitch * cosRoll);
	q.y = (float)(cosYawSinPitch * cosRoll + sinYawCosPitch * sinRoll);
	q.z = (float)(sinYawCosPitch * cosRoll - cosYawSinPitch * sinRoll);

	return q;
}

FVector APhysicsObject::QuaternionToEuler(Quaternion quaternion)
{
	double r11, r21, r31, r32, r33, r12, r13;
	double q00, q11, q22, q33;
	double tmp;
	FVector u;

	q00 = quaternion.w * quaternion.w;
	q11 = quaternion.x * quaternion.x;
	q22 = quaternion.y * quaternion.y;
	q33 = quaternion.z * quaternion.z;

	r11 = q00 + q11 - q22 - q33;
	r21 = 2 * (quaternion.x * quaternion.y + quaternion.w * quaternion.z);
	r31 = 2 * (quaternion.x * quaternion.z - quaternion.w * quaternion.y);
	r32 = 2 * (quaternion.y * quaternion.z + quaternion.w * quaternion.x);
	r33 = q00 - q11 - q22 + q33;

	tmp = fabs(r31);
	if (tmp > 0.999999)
	{
		r12 = 2 * (quaternion.x * quaternion.y - quaternion.w * quaternion.z);
		r13 = 2 * (quaternion.x * quaternion.z + quaternion.w * quaternion.y);

		u.X = (0.0f * 180.0f) / PI; // roll
		u.Y = (((float)(-(PI / 2) * r31 / tmp)) * 180.0f) / PI; // pitch
		u.Z = (((float)(atan2(-r12, -r31 * r13))) * 180.0f) / PI; // yaw
		return u;
	}

	u.X = (((float)(atan2(r32, r33))) * 180.0f) / PI;
	u.Y = (((float)(asin(-r31))) * 180.0f) / PI;
	u.Z = (((float)(atan2(r21, r11))) * 180.0f) / PI;

	return u;
}

void APhysicsObject::FindCollisionResponseMove(APhysicsObject* other)
{
	// add velocity for each hit position
	for (int i = 0; i < mHitPosition.Num(); i += 2)
	{
		FVector intersectionPoint;
		FVector relativeVelocity;
		FVector collisionNormal;

		// find positions on hull closest to the other
		// depending on distance this could be the point of collision
		Ray centroidToCentroidRay;
		centroidToCentroidRay.direction = ((mHitPosition[i + 1].centroid + mHitPosition[i + 1].addWorldOffset) - 
			(mHitPosition[i].centroid + mHitPosition[i].addWorldOffset)).GetSafeNormal();
		centroidToCentroidRay.origin = mHitPosition[i].centroid + mHitPosition[i].addWorldOffset;

		for (int k = 0; k < mHitPosition[i].faces.Num(); k++)
		{
			Plane facePlane;
			facePlane.normal = FVector::CrossProduct(mHitPosition[i].faces[k].a.edgeVector, mHitPosition[i].faces[k].b.edgeVector);
			facePlane.normal.Normalize();
			facePlane.pointOnPlane = mHitPosition[i].faces[k].a.a + mHitPosition[i].addWorldOffset;

			if (IsInFrontOfPlane(centroidToCentroidRay.origin, facePlane)) continue; // rays can only project forwards, so if plane is behind origin, ray will never intersect

			float projection = FVector::DotProduct(facePlane.normal, centroidToCentroidRay.direction);
			if (abs(projection) > 0.0001f) // make sure projection is not zero, this means the ray is perpendicular to the plane and never intersects
			{
				centroidToCentroidRay.t = FVector::DotProduct(facePlane.pointOnPlane - centroidToCentroidRay.origin, facePlane.normal) / projection;
				if (centroidToCentroidRay.t != 0) continue;

				// we have the possible point of contact

				intersectionPoint = centroidToCentroidRay.origin + (centroidToCentroidRay.direction * centroidToCentroidRay.t);
				collisionNormal = facePlane.normal;
			}
		}

		//for (int k = 0; k < mHitPosition[i + 1].faces.Num(); k++)
		//{
		//	Plane facePlane;
		//	facePlane.normal = FVector::CrossProduct(other->mHitPosition[j].faces[k].a.edgeVector, other->mHitPosition[j].faces[k].b.edgeVector);
		//	facePlane.pointOnPlane = other->mHitPosition[j].faces[k].a.a;

		//	if (IsInFrontOfPlane(centroidToCentroidRay.origin, facePlane)) continue; // rays can only project forwards, so if plane is behind origin, ray will never intersect

		//	float projection = FVector::DotProduct(facePlane.normal, centroidToCentroidRay.direction);
		//	if (abs(projection) > 0.0001f) // make sure projection is not zero, this means the ray is perpendicular to the plane and never intersects
		//	{
		//		centroidToCentroidRay.t = FVector::DotProduct(facePlane.pointOnPlane - centroidToCentroidRay.origin, facePlane.normal) / projection;
		//		if (centroidToCentroidRay.t != 0) continue;

		//		// we have the possible point of contact

		//		FVector pointOfContact = centroidToCentroidRay.origin + (centroidToCentroidRay.direction * centroidToCentroidRay.t);
		//		float distance = abs(FVector::Distance(pointOfContact, possiblePointOfContact));

		//		if (distance < smallestDistance) {
		//			smallestDistance = distance;
		//			intersectionPoint = pointOfContact;
		//			collisionNormal = facePlane.normal;
		//		}
		//	}
		//}

		// project velocity onto collision normal for relative velocity

		FVector centroidToPointOfCollision = intersectionPoint - (mHitPosition[i].centroid + mHitPosition[i].addWorldOffset);
		relativeVelocity = mVelocity - other->mVelocity;

		float impulseMag;

		impulseMag = ((-(1 + mCoefficientOfRestitution) * (relativeVelocity * collisionNormal)) /
			((collisionNormal * collisionNormal) * (1 / mMass + 1 / other->mMass))).Size();

		mVelocity += (impulseMag * collisionNormal) / mMass;
		other->mVelocity -= (impulseMag * collisionNormal) / other->mMass;
	}
}


Quaternion APhysicsObject::FindCollisionResponseRotation()
{


	return Quaternion();
}

