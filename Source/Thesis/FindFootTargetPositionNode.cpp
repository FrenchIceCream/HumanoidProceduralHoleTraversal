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

		Start = StartTrace;
		End = EndTrace;
		FVector HitLocation = FRigUnit_FindFootTargetPositionNode::GetHit(Hierarchy, Start, End, 10.5f);


		FRigUnit_AimBoneMath AimMathNode = FRigUnit_AimBoneMath();
		FTransform InputTransform = FTransform::Identity;
		InputTransform.SetLocation(HitLocation);
		AimMathNode.InputTransform = InputTransform;
		AimMathNode.Primary.Axis = FVector(0.f, 0.f, 1.f);
		AimMathNode.Primary.Kind = EControlRigVectorKind::Direction;
		AimMathNode.Primary.Target = /*TraceNode.HitNormal*/ FVector(0.f, 0.f, 1.f);
		AimMathNode.Secondary.Axis = FVector(0.f, 0.f, 1.f);
		AimMathNode.Secondary.Kind = EControlRigVectorKind::Direction;
		AimMathNode.Secondary.Target = /*TraceNode.HitNormal*/ FVector(0.f, 0.f, 1.f);
		AimMathNode.Execute();
		//End = TraceNode.HitLocation;

		FootTargetLocation = HitLocation - VelocityScaled + FVector(0, 0, FootGroundOffset);
		FootTargetRotation = AimMathNode.Result.GetRotation();
	}
}

FVector FRigUnit_FindFootTargetPositionNode::GetHit(URigHierarchy* Hierarchy, FVector Start, FVector End, float Radius)
{
	FRigUnit_SphereTraceByTraceChannel TraceNode = FRigUnit_SphereTraceByTraceChannel();
	TraceNode.Start = Start;
	TraceNode.End = End;
	TraceNode.Radius = Radius;

	TraceNode.Execute();
	return TraceNode.HitLocation;
}