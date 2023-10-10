// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MyProject/PO_Sphere.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodePO_Sphere() {}
// Cross Module References
	MYPROJECT_API UClass* Z_Construct_UClass_APO_Sphere_NoRegister();
	MYPROJECT_API UClass* Z_Construct_UClass_APO_Sphere();
	ENGINE_API UClass* Z_Construct_UClass_AActor();
	UPackage* Z_Construct_UPackage__Script_MyProject();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FVector();
	ENGINE_API UClass* Z_Construct_UClass_UStaticMeshComponent_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_USphereComponent_NoRegister();
// End Cross Module References
	DEFINE_FUNCTION(APO_Sphere::execCheckForCollision)
	{
		P_GET_STRUCT(FVector,Z_Param_centreToCentreVector);
		P_GET_PROPERTY(FFloatProperty,Z_Param_otherRadius);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->CheckForCollision(Z_Param_centreToCentreVector,Z_Param_otherRadius);
		P_NATIVE_END;
	}
	void APO_Sphere::StaticRegisterNativesAPO_Sphere()
	{
		UClass* Class = APO_Sphere::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "CheckForCollision", &APO_Sphere::execCheckForCollision },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics
	{
		struct PO_Sphere_eventCheckForCollision_Parms
		{
			FVector centreToCentreVector;
			float otherRadius;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_centreToCentreVector;
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_otherRadius;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_centreToCentreVector = { "centreToCentreVector", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(PO_Sphere_eventCheckForCollision_Parms, centreToCentreVector), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_otherRadius = { "otherRadius", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(PO_Sphere_eventCheckForCollision_Parms, otherRadius), METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((PO_Sphere_eventCheckForCollision_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(PO_Sphere_eventCheckForCollision_Parms), &Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_centreToCentreVector,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_otherRadius,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::Function_MetaDataParams[] = {
		{ "Category", "Collision" },
		{ "ModuleRelativePath", "PO_Sphere.h" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_APO_Sphere, nullptr, "CheckForCollision", nullptr, nullptr, sizeof(PO_Sphere_eventCheckForCollision_Parms), Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04820401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_APO_Sphere_CheckForCollision()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_APO_Sphere_CheckForCollision_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_APO_Sphere_NoRegister()
	{
		return APO_Sphere::StaticClass();
	}
	struct Z_Construct_UClass_APO_Sphere_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_mSphereVisual_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_mSphereVisual;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_mSphere_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_mSphere;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_mRadius_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_mRadius;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_APO_Sphere_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AActor,
		(UObject* (*)())Z_Construct_UPackage__Script_MyProject,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_APO_Sphere_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_APO_Sphere_CheckForCollision, "CheckForCollision" }, // 1214122186
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_APO_Sphere_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "PO_Sphere.h" },
		{ "ModuleRelativePath", "PO_Sphere.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphereVisual_MetaData[] = {
		{ "Category", "PO_Sphere" },
		{ "Comment", "// Static mesh component for visible sphere\n" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "PO_Sphere.h" },
		{ "ToolTip", "Static mesh component for visible sphere" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphereVisual = { "mSphereVisual", nullptr, (EPropertyFlags)0x00100000000b0009, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(APO_Sphere, mSphereVisual), Z_Construct_UClass_UStaticMeshComponent_NoRegister, METADATA_PARAMS(Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphereVisual_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphereVisual_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphere_MetaData[] = {
		{ "Category", "PO_Sphere" },
		{ "Comment", "//USphereComponent for actual sphere collision & physics calculations\n" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "PO_Sphere.h" },
		{ "ToolTip", "USphereComponent for actual sphere collision & physics calculations" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphere = { "mSphere", nullptr, (EPropertyFlags)0x00100000000a001d, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(APO_Sphere, mSphere), Z_Construct_UClass_USphereComponent_NoRegister, METADATA_PARAMS(Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphere_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphere_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_APO_Sphere_Statics::NewProp_mRadius_MetaData[] = {
		{ "Category", "SphereProperties" },
		{ "Comment", "// Sphere properties\n" },
		{ "ModuleRelativePath", "PO_Sphere.h" },
		{ "ToolTip", "Sphere properties" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_APO_Sphere_Statics::NewProp_mRadius = { "mRadius", nullptr, (EPropertyFlags)0x0010000000020015, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(APO_Sphere, mRadius), METADATA_PARAMS(Z_Construct_UClass_APO_Sphere_Statics::NewProp_mRadius_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_APO_Sphere_Statics::NewProp_mRadius_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_APO_Sphere_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphereVisual,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_APO_Sphere_Statics::NewProp_mSphere,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_APO_Sphere_Statics::NewProp_mRadius,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_APO_Sphere_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<APO_Sphere>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_APO_Sphere_Statics::ClassParams = {
		&APO_Sphere::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		Z_Construct_UClass_APO_Sphere_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		UE_ARRAY_COUNT(Z_Construct_UClass_APO_Sphere_Statics::PropPointers),
		0,
		0x009000A4u,
		METADATA_PARAMS(Z_Construct_UClass_APO_Sphere_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_APO_Sphere_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_APO_Sphere()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_APO_Sphere_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(APO_Sphere, 1333756364);
	template<> MYPROJECT_API UClass* StaticClass<APO_Sphere>()
	{
		return APO_Sphere::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_APO_Sphere(Z_Construct_UClass_APO_Sphere, &APO_Sphere::StaticClass, TEXT("/Script/MyProject"), TEXT("APO_Sphere"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(APO_Sphere);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
