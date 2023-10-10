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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = SphereProperties)
		float mRadius;


private:
	// Physics properties
	float g = -9.8; // acceleration in m/s/s
	FVector Velocity{ 0.0f, 0.0f, 0.0f }; // Initial velocity is 0 (measured in m/s)
	FVector Displacement{ 0.0f, 0.0f, 0.0f}; // Displacement from orgin (measured in meters)

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = Collision)
	bool CheckForCollision(FVector centreToCentreVector, float otherRadius);
};
