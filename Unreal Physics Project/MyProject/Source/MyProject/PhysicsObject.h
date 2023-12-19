// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsObject.generated.h"

class USphereComponent;
class UStaticMeshComponent;

struct Sphere {
	FVector sCenter;
	float sRadius;
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
	//USphereComponent for initial capsule motion tests
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USphereComponent* mBoundingSphere;

	Sphere mSphere;

	TArray<FVector> mVertexPositions;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool GetVertexPositions();

	// set of functions for finding bounding sphere
	void FindMostSeparatedPoints(int& min, int& max);
	void SphereFromDistantPoints();
	void SphereOfSphereAndPoint(FVector point);
	void GenerateSphere();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
