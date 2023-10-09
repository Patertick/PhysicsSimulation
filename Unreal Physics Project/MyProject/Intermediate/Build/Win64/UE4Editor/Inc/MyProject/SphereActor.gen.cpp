// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MyProject/SphereActor.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSphereActor() {}
// Cross Module References
	MYPROJECT_API UClass* Z_Construct_UClass_ASphereActor_NoRegister();
	MYPROJECT_API UClass* Z_Construct_UClass_ASphereActor();
	ENGINE_API UClass* Z_Construct_UClass_AActor();
	UPackage* Z_Construct_UPackage__Script_MyProject();
// End Cross Module References
	void ASphereActor::StaticRegisterNativesASphereActor()
	{
	}
	UClass* Z_Construct_UClass_ASphereActor_NoRegister()
	{
		return ASphereActor::StaticClass();
	}
	struct Z_Construct_UClass_ASphereActor_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_ASphereActor_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AActor,
		(UObject* (*)())Z_Construct_UPackage__Script_MyProject,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASphereActor_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "SphereActor.h" },
		{ "ModuleRelativePath", "SphereActor.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_ASphereActor_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ASphereActor>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_ASphereActor_Statics::ClassParams = {
		&ASphereActor::StaticClass,
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
		METADATA_PARAMS(Z_Construct_UClass_ASphereActor_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_ASphereActor_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_ASphereActor()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_ASphereActor_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(ASphereActor, 4195911437);
	template<> MYPROJECT_API UClass* StaticClass<ASphereActor>()
	{
		return ASphereActor::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_ASphereActor(Z_Construct_UClass_ASphereActor, &ASphereActor::StaticClass, TEXT("/Script/MyProject"), TEXT("ASphereActor"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(ASphereActor);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
