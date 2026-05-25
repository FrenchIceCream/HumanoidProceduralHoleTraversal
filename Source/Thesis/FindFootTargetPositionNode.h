// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Units/RigUnit.h"
#include "FindFootTargetPositionNode.generated.h"

USTRUCT(meta = (DisplayName = "FindFootTargetPosition", Category = "Custom"))
struct THESIS_API FRigUnit_FindFootTargetPositionNode : public FRigUnitMutable
{
    GENERATED_BODY()

    FRigUnit_FindFootTargetPositionNode() : FootIndex(-1) {};

    RIGVM_METHOD()
    virtual void Execute() override;

    UPROPERTY(meta = (Input))
    FRigElementKey FootReference;

    UPROPERTY(meta = (Input))
    int FootIndex;

    UPROPERTY(meta = (Input))
    FVector PreviousPelvisPosition;

    UPROPERTY(meta = (Input))
    TArray<float> PerFootCyclePercent;

    UPROPERTY(meta = (Input))
    float SwingTimeAsPercent;

    UPROPERTY(meta = (Input))
    float CycleLengthInSeconds;

    UPROPERTY(meta = (Input))
    float SwingTime;

    UPROPERTY(meta = (Input))
    FVector RigSpaceCalculatedVelocity;

    UPROPERTY(meta = (Input))
    float FootGroundOffset;

    UPROPERTY(meta = (Output))
    FTransform FootTargetLocation;
};
