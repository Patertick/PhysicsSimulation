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

	friend bool operator==(Edge& self, Edge& other)
	{
		if (self.a == other.a || self.a == other.b) {
			if (self.b == other.a || self.b == other.b) return true;
		}
		return false;
	}

	friend bool operator==(Edge& self, const Edge& other)
	{
		if (self.a == other.a || self.a == other.b) {
			if (self.b == other.a || self.b == other.b) return true;
		}
		return false;
	}

	friend bool operator==(const Edge& self, Edge& other)
	{
		if (self.a == other.a || self.a == other.b) {
			if (self.b == other.a || self.b == other.b) return true;
		}
		return false;
	}

	friend bool operator==(const Edge& self, const Edge& other)
	{
		if (self.a == other.a || self.a == other.b) {
			if (self.b == other.a || self.b == other.b) return true;
		}
		return false;
	}
};

struct Face {
	Edge a;
	Edge b;
	Edge c;
	bool isNew{ true };
	friend bool operator==(Face& self, Face& other)
	{
		if (self.a == other.a || self.a == other.b || self.a == other.c)
		{
			if (self.b == other.a || self.b == other.b || self.b == other.c)
			{
				if (self.c == other.a || self.c == other.b || self.c == other.c) return true;
			}
		}
		return false;
	}

	friend bool operator==(const Face& self, Face& other)
	{
		if (self.a == other.a || self.a == other.b || self.a == other.c)
		{
			if (self.b == other.a || self.b == other.b || self.b == other.c)
			{
				if (self.c == other.a || self.c == other.b || self.c == other.c) return true;
			}
		}
		return false;
	}

	friend bool operator==(Face& self, const Face& other)
	{
		if (self.a == other.a || self.a == other.b || self.a == other.c)
		{
			if (self.b == other.a || self.b == other.b || self.b == other.c)
			{
				if (self.c == other.a || self.c == other.b || self.c == other.c) return true;
			}
		}
		return false;
	}

	friend bool operator==(const Face& self, const Face& other)
	{
		if (self.a == other.a || self.a == other.b || self.a == other.c)
		{
			if (self.b == other.a || self.b == other.b || self.b == other.c)
			{
				if (self.c == other.a || self.c == other.b || self.c == other.c) return true;
			}
		}
		return false;
	}
};

struct ConvexHull {
	TArray<Face> faces; // faces that make up convex hull of point sets
	TArray<FVector> points; // all vertices that make up the hull
	FVector centroid; // centre of hull
	FVector startPos;
	FVector offsetFromStart{ 0.0f, 0.0f, 0.0f };
	FVector addWorldOffset;

	friend bool operator==(ConvexHull& self, ConvexHull& other)
	{
		if (self.faces == other.faces) {
			if (self.points == other.points) {
				if (self.centroid == other.centroid) return true;
			}
		}
		return false;
	}

	friend bool operator==(ConvexHull& self, const ConvexHull& other)
	{
		if (self.faces == other.faces) {
			if (self.points == other.points) {
				if (self.centroid == other.centroid) return true;
			}
		}
		return false;
	}

	friend bool operator==(const ConvexHull& self, ConvexHull& other)
	{
		if (self.faces == other.faces) {
			if (self.points == other.points) {
				if (self.centroid == other.centroid) return true;
			}
		}
		return false;
	}

	friend bool operator==(const ConvexHull& self, const ConvexHull& other)
	{
		if (self.faces == other.faces) {
			if (self.points == other.points) {
				if (self.centroid == other.centroid) return true;
			}
		}
		return false;
	}
};

struct Plane {
	FVector normal;
	FVector pointOnPlane;

	friend bool operator==(Plane& self, Plane& other)
	{
		if (self.normal == other.normal) {
			if (self.pointOnPlane == other.pointOnPlane) {
				return true;
			}
		}
		return false;
	}

	friend bool operator==(Plane& self, const Plane& other)
	{
		if (self.normal == other.normal) {
			if (self.pointOnPlane == other.pointOnPlane) {
				return true;
			}
		}
		return false;
	}

	friend bool operator==(const Plane& self, Plane& other)
	{
		if (self.normal == other.normal) {
			if (self.pointOnPlane == other.pointOnPlane) {
				return true;
			}
		}
		return false;
	}

	friend bool operator==(const Plane& self, const Plane& other)
	{
		if (self.normal == other.normal) {
			if (self.pointOnPlane == other.pointOnPlane) {
				return true;
			}
		}
		return false;
	}
};

struct Tensor {
	float tensor[9]; // 3x3 matrix	
};

struct Quaternion {
	float x;
	float y;
	float z;
	float w;

	inline Quaternion QuatQuatMultiply(Quaternion q1, Quaternion q2)
	{
		Quaternion newQuat;
		newQuat.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
		newQuat.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
		newQuat.y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
		newQuat.z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;
		return newQuat;
	}

	inline Quaternion QuatVecMultiply(Quaternion q, FVector v)
	{
		Quaternion newQuat;
		newQuat.w = -(q.x * v.X + q.y * v.Y + q.z * v.Z);
		newQuat.x = q.w * v.X + q.y * v.Z - q.z * v.Y;
		newQuat.y = q.w * v.Y + q.z * v.X - q.x * v.Z;
		newQuat.z = q.w * v.Z + q.x * v.Y - q.y * v.X;
		return newQuat;
	}

	inline Quaternion VecQuatMultiply(FVector v, Quaternion q)
	{
		Quaternion newQuat;
		newQuat.w = -(q.x * v.X + q.y * v.Y + q.z * v.Z);
		newQuat.x = q.w * v.X + q.z * v.Y - q.y * v.Z;
		newQuat.y = q.w * v.Y + q.x * v.Z - q.z * v.X;
		newQuat.z = q.w * v.Z + q.y * v.X - q.x * v.Y;
		return newQuat;
	}

	inline Quaternion operator+(Quaternion q)
	{
		Quaternion newQuat;
		newQuat.w = q.w + w;
		newQuat.x = q.x + x;
		newQuat.y = q.y + y;
		newQuat.z = q.z + z;
		return newQuat;
	}

	inline Quaternion operator/(float f)
	{
		Quaternion newQuat;
		newQuat.w = w / f;
		newQuat.x = x / f;
		newQuat.y = y / f;
		newQuat.z = z / f;
		return newQuat;
	}

	float Magnitude()
	{
		return sqrt((w * w) + (x * x) + (y * y) + (z * z));
	}
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicsProperties)
		bool mAffectedByGravity{ true };
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

	Quaternion mOrientation; // current orientation

	// used for collision response

	Tensor mIntertialTensor;

	TArray<Tensor> mLocalInertias; // these will be simplified

	FVector mCentreOfGravity;

	// mesh data

	TArray<FVector> mVertexPositions;

	TArray<Face> mFaceGeometry;

	TArray<ConvexHull> mHitPosition;

	// Physics properties
	FVector mForce{ 0.0f, 0.0f, 0.0f }; // Force vector, aggregate all forces in this vector
	FVector mAcceleration{ 0.0f, 0.0f, 0.0f }; // Acceleration measured in m/s/s
	FVector mVelocity{ 0.0f, 0.0f, 0.0f }; // velocity (measured in m/s)
	FVector mDisplacement = GetActorLocation(); // Displacement from orgin (measured in meters)
	FVector mAngularVelocity{ 0.0f, 0.0f, 0.0f }; // angular velocity
	FVector mMoments{ 0.0f, 0.0f, 0.0f };
	FVector mNormalForce{ 0.0f, 0.0f, 0.0f };

	// Object properties
	float mVolume{ 0.0f };
	float mDensity{ 0.0f };
	float mSurfaceArea{ 0.0f };

	float mDefaultTimeStep{ 0.5f };

	bool mHit{ false };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool GetVertexPositions();

	// initial physics shape set up
	FVector FindCentreOfGravity();
	Tensor FindInertialTensor();
	Tensor FindClosestMatchingTensor(ConvexHull convexHull);

	// Set of functions for finding bounding sphere
	void FindMostSeparatedPoints(int& min, int& max);
	void SphereFromDistantPoints();
	void SphereOfSphereAndPoint(FVector point);
	void GenerateSphere();

	// Functions for use in creation of convex sub meshes and resultant OBBs
	ConvexHull CreateConvexHull(const TArray<FVector> &points);
	TArray<Face> ConstructFaces(const TArray<FVector> &points);
	void DecomposeMesh(const TArray<FVector> &points);
	bool IsInFrontOfPlane(FVector point, Plane plane);
	Plane FindInflexFacePlane(const TArray<FVector> &points);
	float FindConcavity(ConvexHull convexHull, const TArray<FVector> &points);

	// Collision check functions
	FVector CheckForPlaneCollision();
	TArray<ConvexHull> SeparatingAxisTest(const TArray<ConvexHull>& other);
	bool SphereOnSphereCheck(const Sphere& other);

	// quartenion functions

	Quaternion EulerToQuaternion(FVector rot);
	FVector QuaternionToEuler(Quaternion quaternion);

	// Response functions

	void FindCollisionResponseMove(APhysicsObject* other);
	Quaternion FindCollisionResponseRotation();
	void Hit();


	// Simulation functions
	FVector FindGravityForce();
	FVector FindDragForce(FVector velocity);
	FVector FindWindForce(FVector windSpeed);
	void StepSimulation(float deltaTime); // to find displacement & velocity from acceleration
	UFUNCTION(BlueprintCallable, Category = Getter)
		FVector GetDisplacement() { return mDisplacement; }
	UFUNCTION(BlueprintCallable, Category = Getter)
		FVector GetSphereVelocity() { return mVelocity; }

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	Sphere GetSphere() { return mSphere; }

};
