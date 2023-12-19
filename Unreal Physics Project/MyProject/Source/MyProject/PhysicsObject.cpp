// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsObject.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsPublic.h"
#include "PhysXPublicCore.h"
#include "PhysXIncludes.h"
#include "PxTriangleMesh.h"
#include "PxSimpleTypes.h"
#include <String>

// Sets default values
APhysicsObject::APhysicsObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	

	mBoundingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Physics Sphere"));
	mBoundingSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	mObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Physics Mesh"));
	//mObjectMesh->GetStaticMesh()->bAllowCPUAccess = true;   NOTE: ABSOLUTELY DEADLY CODE THAT CRASHES UNREAL
	mObjectMesh->SetOwnerNoSee(true);
	
	mObjectMesh->AttachToComponent(mBoundingSphere, FAttachmentTransformRules::KeepRelativeTransform);

	
	
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
	for (int32 i = 1; i < mVertexPositions.Num(); i++) {
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
	if (distanceY > distanceX && distanceZ > distanceZ) {
		min = minY;
		max = maxY;		
	}
	else if (distanceZ > distanceX && distanceZ > distanceY) {
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
	for (int i = 1; i < mVertexPositions.Num(); i++) {
		SphereOfSphereAndPoint(mVertexPositions[i]);
	}

	RootComponent->AddLocalOffset(FVector(0, 0, mSphere.sRadius * 2));
}

// Called every frame
void APhysicsObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	mObjectMesh->SetWorldLocation(mSphere.sCenter);
	mBoundingSphere->SetSphereRadius(mSphere.sRadius);

}

