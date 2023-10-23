// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PO_Sphere.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class MYPROJECT_API APO_Sphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APO_Sphere();


	// Static mesh component for visible sphere
	UPROPERTY(VisibleDefaultsOnly)
		UStaticMeshComponent* mSphereVisual;
	//USphereComponent for actual sphere collision & physics calculations
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USphereComponent* mSphere;



	// Sphere properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SphereProperties)
		float mRadius{ 32.0f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = SphereProperties)
		float mVolume;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = SphereProperties)
		float mDensity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = SphereProperties)
		float mSurfaceArea;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SphereProperties)
		float mCoefficientOfDrag{ 0.1 }; // assumed sphere is smooth
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SphereProperties)
		float mMass{ 10.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SphereProperties)
		float dt{ 0.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SphereProperties)
		float mCoefficientOfRestitution{ 1.0f }; // perfectly elastic
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SphereProperties)
		FVector WindSpeed { 0.0f, 0.0f, 0.0f };

	float startRadius;
	

	UPROPERTY(EditAnywhere, Category = SphereProperties)
		bool isMovingSphere{ true };


private:
	// Physics properties
	FVector Force{ 0.0f, 0.0f, 0.0f }; // Force vector, aggregate all forces in this vector
	FVector Acceleration{ 0.0f, 0.0f, 0.0f }; // Acceleration measured in m/s/s
	FVector Velocity{ 0.0f, 0.0f, 0.0f }; // velocity (measured in m/s)
	FVector Displacement = GetActorLocation(); // Displacement from orgin (measured in meters)


	bool IsNormalForce{ false };
	FVector NormalForceVector;
	bool IsImpulseLocked{ false };
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = Collision)
		bool CheckForSphereCollision(FVector centreToCentreVector, float otherRadius);
	UFUNCTION(BlueprintCallable, Category = Collision)
		bool CheckForPlaneCollision(FVector KToSphereVector, FVector surfaceNormalOfPlane);
	UFUNCTION(BlueprintCallable, Category = Collision)
		bool CheckForMovingSphereCollision(FVector otherVelocity, float otherRadius, FVector otherLocation, float otherMass);

	FVector FindGravityForce();
	FVector FindDragForce(FVector velocity);
	FVector FindWindForce(FVector windSpeed);
	void StepSimulation(); // to find displacement & velocity from acceleration
	UFUNCTION(BlueprintCallable, Category = Getter)
		FVector GetDisplacement() { return Displacement; }
	UFUNCTION(BlueprintCallable, Category = Getter)
		FVector GetSphereVelocity() { return Velocity; }

	FVector FindImpulseMovingSphere(FVector otherVelocity, FVector selfVelocity, FVector otherLocation, FVector selfLocation, float otherMass);
};
