// Fill out your copyright notice in the Description page of Project Settings.


#include "FindFootTargetPositionNode.h"
#include "Engine/EngineTypes.h"
#include "Units/Collision/RigUnit_WorldCollision.h"
#include "Units/Highlevel/Hierarchy/RigUnit_AimBone.h"

FRigUnit_FindFootTargetPositionNode_Execute()
{
	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy && FootIndex != -1)
	{
		FTransform FootTransform = Hierarchy->GetGlobalTransform(FootReference);

		FVector VelocityScaled = RigSpaceCalculatedVelocity * ((SwingTimeAsPercent - PerFootCyclePercent[FootIndex]) * CycleLengthInSeconds);
		FVector StartTrace = FVector(FootTransform.GetLocation().X, FootTransform.GetLocation().Y, PreviousPelvisPosition.Z);

		FVector EndTrace = FootTransform.GetLocation() + VelocityScaled + RigSpaceCalculatedVelocity * ((CycleLengthInSeconds - SwingTime) / 2) - FVector(0, 0, 40);


		FRigUnit_SphereTraceWorld TraceNode = FRigUnit_SphereTraceWorld();
		TraceNode.Start = StartTrace;
		TraceNode.End = EndTrace;
		TraceNode.Radius = 0.5f;
		TraceNode.Execute();

		FRigUnit_AimBoneMath AimMathNode = FRigUnit_AimBoneMath();
		FTransform InputTransform = FTransform::Identity;
		InputTransform.SetLocation(TraceNode.HitLocation);
		AimMathNode.InputTransform = InputTransform;
		AimMathNode.Primary.Axis = FVector(0.f, 0.f, 1.f);
		AimMathNode.Primary.Kind = EControlRigVectorKind::Direction;
		AimMathNode.Secondary.Axis = FVector(0.f, 0.f, 1.f);
		AimMathNode.Secondary.Kind = EControlRigVectorKind::Direction;
		AimMathNode.Execute();


		FTransform ResultingTransform = FTransform::Identity;
		ResultingTransform.SetLocation(TraceNode.HitLocation - VelocityScaled + FVector(0, 0, FootGroundOffset));
		ResultingTransform.SetRotation(AimMathNode.Result.GetRotation());

		
		FootTargetLocation = ResultingTransform;
	}
}