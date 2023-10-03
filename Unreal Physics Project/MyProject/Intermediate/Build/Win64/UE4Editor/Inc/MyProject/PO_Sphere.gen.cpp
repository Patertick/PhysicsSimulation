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
// End Cross Module References
	void APO_Sphere::StaticRegisterNativesAPO_Sphere()
	{
	}
	UClass* Z_Construct_UClass_APO_Sphere_NoRegister()
	{
		return APO_Sphere::StaticClass();
	}
	struct Z_Construct_UClass_APO_Sphere_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_APO_Sphere_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AActor,
		(UObject* (*)())Z_Construct_UPackage__Script_MyProject,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_APO_Sphere_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "PO_Sphere.h" },
		{ "ModuleRelativePath", "PO_Sphere.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_APO_Sphere_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<APO_Sphere>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_APO_Sphere_Statics::ClassParams = {
		&APO_Sphere::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		0,
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
	IMPLEMENT_CLASS(APO_Sphere, 171535801);
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
