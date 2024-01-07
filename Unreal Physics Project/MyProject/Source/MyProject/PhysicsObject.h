// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsObject.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UCapsuleComponent;

struct Sphere {
	FVector center;
	float radius;
};

struct Capsule {
	FVector startPoint; // A
	FVector endPoint; // B
	float radius;
};

struct Ray {
	FVector origin;
	FVector direction;
	float t;
};

struct Edge {
	FVector a;
	FVector b;
	FVector edgeVector;

	bool operator==(Edge& other)
	{
		if (a == other.a || a == other.b) {
			if (b == other.a || b == other.b) return true;
		}
		return false;
	}
};

struct Face {
	Edge a;
	Edge b;
	Edge c;
	bool isNew{ true };
	bool operator==(Face& other)
	{
		if (a == other.a || a == other.b || a == other.c)
		{
			if (b == other.a || b == other.b || b == other.c)
			{
				if (c == other.a || c == other.b || c == other.c) return true;
			}
		}
		return false;
	}
};

struct ConvexHull {
	TArray<Face> faces; // faces that make up convex hull of point sets
	TArray<FVector> points; // all vertices that make up the hull
	FVector centroid; // centre of hull
};

struct Plane {
	FVector normal;
	FVector pointOnPlane;
};



UCLASS()
class MYPROJECT_API APhysicsObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APhysicsObject();

	// Static mesh component
	UPROPERTY(VisibleDefaultsOnly)
		UStaticMeshComponent* mObjectMesh;

	// sub meshes
	// used primarily to generate OBBs rather than for visuals, therefore, a mesh data structure is not used so as not to store redundant data
	TArray<ConvexHull> mMeshes; // may have one or multiple elements (dependant on if starting mesh is convex or concave) 

	//USphereComponent for initial capsule motion tests
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USphereComponent* mBoundingVisualSphere;
	// physics properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PhysicsProperties)
		float mCoefficientOfDrag{ 0.1 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicsProperties)
		float mMass{ 10.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicsProperties)
		float mTimeStep{ 0.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicsProperties)
		float mCoefficientOfRestitution{ 1.0f }; // perfectly elastic
	// other actor properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlaneProperties)
		FVector mArbritraryPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlaneProperties)
		FVector mSurfaceNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicsProperties)
		FVector mWindSpeed { 0.0f, 0.0f, 0.0f };

	// collision modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CollisionModifiers)
		float mMaxConcavity{ 0.5 }; // for use in approximate convex hull creation

private:
	Sphere mSphere; // bounding sphere covering whole mesh

	Sphere mPredictedSphere;

	Capsule mCapsule;

	

	TArray<FVector> mVertexPositions;

	TArray<ConvexHull> mHitPosition;

	// Physics properties
	FVector mForce{ 0.0f, 0.0f, 0.0f }; // Force vector, aggregate all forces in this vector
	FVector mAcceleration{ 0.0f, 0.0f, 0.0f }; // Acceleration measured in m/s/s
	FVector mVelocity{ 0.0f, 0.0f, 0.0f }; // velocity (measured in m/s)
	FVector mDisplacement = GetActorLocation(); // Displacement from orgin (measured in meters)
	FVector mNormalForce{ 0.0f, 0.0f, 0.0f };

	// Object properties
	float mVolume{ 0.0f };
	float mDensity{ 0.0f };
	float mSurfaceArea{ 0.0f };

	float mDefaultTimeStep{ 0.5f };

	bool Hit{ false };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool GetVertexPositions();

	// Set of functions for finding bounding sphere
	void FindMostSeparatedPoints(int& min, int& max);
	void SphereFromDistantPoints();
	void SphereOfSphereAndPoint(FVector point);
	void GenerateSphere();

	// Functions for use in creation of convex sub meshes and resultant OBBs
	ConvexHull CreateConvexHull(const TArray<FVector> &points);
	TArray<Face> ConstructFaces(ConvexHull convexHull);
	void DecomposeMesh(const TArray<FVector> &points);
	bool IsInFrontOfPlane(FVector point, Plane plane);
	Plane FindInflexFacePlane(const TArray<FVector> &points);
	float FindConcavity(ConvexHull convexHull, const TArray<FVector> &points);

	// Collision check functions
	FVector CheckForPlaneCollision();
	TArray<ConvexHull> SeparatingAxisTest(const TArray<ConvexHull>& other);
	bool SphereOnSphereCheck(const Sphere& other);

	// Simulation functions
	FVector FindGravityForce();
	FVector FindDragForce(FVector velocity);
	FVector FindWindForce(FVector windSpeed);
	void StepSimulation(); // to find displacement & velocity from acceleration
	UFUNCTION(BlueprintCallable, Category = Getter)
		FVector GetDisplacement() { return mDisplacement; }
	UFUNCTION(BlueprintCallable, Category = Getter)
		FVector GetSphereVelocity() { return mVelocity; }

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	Sphere GetSphere() { return mSphere; }

};
