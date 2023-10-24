// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PO_Boat.generated.h"

UCLASS()
class MYPROJECT_API APO_Boat : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APO_Boat();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoatProperties)
		float BaseSideLength { 25.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoatProperties)
		float mMass{ 1.0f }; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoatProperties)
		float dt{ 0.0f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BoatProperties)
		float mCoefficientOfDrag{ 0.1 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoatProperties)
		float mCoefficientOfRestitution{ 1.0f }; // perfectly elastic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoatProperties)
		FVector WindSpeed { 0.0f, 0.0f, 0.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoatProperties)
		float mFluidDensity{ 0.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoatProperties)
		float mSubmersedLength{ 0.0f };


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	FVector mCentreOfGravity{ 0.0f, 0.0f, 0.0f };
	float mVolume{ 0.0f };
	float mDensity{ 0.0f };
	float mSurfaceArea{ 0.0f };

	FVector Velocity{ 0.0f, 0.0f, 0.0f };
	FVector Displacement{ 0.0f, 0.0f, 0.0f };
	float g{ -9.8 };

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FVector FindGravityForce();
	FVector FindDragForce(FVector velocity);
	FVector FindWindForce(FVector windSpeed);

	void StepSimulation();

	float FindVolumeSubmerged(float submersedLength);
	FVector FindBuoyantForce(float fluidDensity);

};
