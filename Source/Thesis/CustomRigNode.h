// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Units/RigUnit.h"
#include "CustomRigNode.generated.h"

USTRUCT(meta = (DisplayName = "CustomInverseKinematics", Category = "Custom"))
struct THESIS_API FRigUnit_CustomRigNode : public FRigUnitMutable
{
    GENERATED_BODY()

    FRigUnit_CustomRigNode()
     : LengthOfChain(3), Iterations(16){}

    RIGVM_METHOD()
    virtual void Execute() override;

    UPROPERTY(meta = (Input))
    FTransform EffectorTransform;

    UPROPERTY(meta = (Input))
    FRigElementKey BasePoint;

    UPROPERTY(meta = (Input))
    FRigElementKey Effector;

    UPROPERTY(meta = (Input))
    int LengthOfChain;

    UPROPERTY(meta = (Input))
    int Iterations;

    UPROPERTY(meta = (Input))
    bool bShouldAvoidCollisions;

    UPROPERTY(meta = (Input))
    bool bShouldAvoidSelfCollisions;

    UPROPERTY(meta = (Input))
    TArray<FName> BoneConstraints;

    UPROPERTY(meta = (Input))
    TArray<FVector> ConstraintsAnglesMin;

    UPROPERTY(meta = (Input))
    TArray<FVector> ConstraintsAnglesMax;

    UPROPERTY(meta = (Input))
    TArray<FName> BonesToCheckForSelfCollisions;

    static void ApplyConstraints(const TArrayView<const FName, int32> BoneConstraintsNames, const TArrayView<const FVector, int32> ConstraintAnglesMin, const TArrayView<const FVector, int32> ConstraintAnglesMax, const TArrayView<const FName, int32> BonesCheckForSelfCollisions, URigHierarchy* Hierarchy, FRigElementKey Bone, float BoneLength, float SegmentRadius, bool shouldAvoidCollisions, bool shouldAvoidSelfCollisions, TArray<FRigElementKey> BoneChain);

    static FVector GetBoneForwardVector(URigHierarchy* Hierarchy, FRigElementKey Bone);
};
