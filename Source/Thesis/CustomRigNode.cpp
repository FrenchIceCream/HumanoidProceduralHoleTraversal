// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomRigNode.h"
#include "Engine/EngineTypes.h"
#include "Units/Collision/RigUnit_WorldCollision.h"

void FRigUnit_CustomRigNode::ApplyConstraints(const TArrayView<const FName, int32> BoneConstraintsNames, const TArrayView<const FVector, int32> ConstraintAnglesMin, const TArrayView<const FVector, int32> ConstraintAnglesMax, const TArrayView<const FName, int32> BonesCheckForSelfCollisions, URigHierarchy* Hierarchy, FRigElementKey Bone, float BoneLength, float SegmentRadius, bool shouldAvoidCollisions, bool shouldAvoidSelfCollisions, TArray<FRigElementKey> BoneChain)
{
	if (BoneConstraintsNames.Contains(Bone.Name))
	{
		int ConstraintIndex = BoneConstraintsNames.IndexOfByKey(Bone.Name);

		FTransform ResultingTransform = Hierarchy->GetLocalTransform(Bone);;

		FQuat Rotation = ResultingTransform.GetRotation();
		FVector EulerRotation = Rotation.Euler();
		FVector Min = ConstraintAnglesMin[ConstraintIndex];
		FVector Max = ConstraintAnglesMax[ConstraintIndex];

		EulerRotation = FVector(FMath::Clamp(EulerRotation.X, Min.X, Max.X),
			FMath::Clamp(EulerRotation.Y, Min.Y, Max.Y),
			FMath::Clamp(EulerRotation.Z, Min.Z, Max.Z));

		ResultingTransform.SetRotation(FQuat::MakeFromEuler(EulerRotation));
		Hierarchy->SetLocalTransform(Bone, ResultingTransform, false, false);
	}

	FTransform BoneTransform = Hierarchy->GetGlobalTransform(Bone);

	if (shouldAvoidCollisions)
	{
		FVector ForwardVector = FRigUnit_CustomRigNode::GetBoneForwardVector(Hierarchy, Bone);
		ForwardVector *= BoneLength;

		FRigUnit_SphereTraceWorld TraceNode = FRigUnit_SphereTraceWorld();
		TraceNode.Start = BoneTransform.GetLocation();
		TraceNode.End = TraceNode.Start + ForwardVector;
		TraceNode.Radius = SegmentRadius;

		TraceNode.Execute();

		if (TraceNode.bHit)
		{
			FVector HitLocation = TraceNode.HitLocation;

			float CollisionDepth = FVector::Dist(BoneTransform.GetLocation(), HitLocation);
			
			FVector VectorTowardsTargetPos = TraceNode.HitNormal * (BoneLength - CollisionDepth);
			VectorTowardsTargetPos -= BoneTransform.GetLocation();
			VectorTowardsTargetPos.Normalize();

			ForwardVector.Normalize();
			FQuat Angle = FQuat::FindBetweenNormals(VectorTowardsTargetPos, ForwardVector);

			FTransform ResultingTransform = BoneTransform;
			ResultingTransform.SetRotation(ResultingTransform.GetRotation() * Angle);
			Hierarchy->SetGlobalTransform(Bone, ResultingTransform, false, false);
			BoneTransform = Hierarchy->GetGlobalTransform(Bone);
		}
	}

	if (shouldAvoidSelfCollisions)
	{
		TArray<FName> BonesToCheck;
		//Ignoring bones in chain
		for (int i = 0; i < BonesCheckForSelfCollisions.Num(); i++)
		{
			bool bAddBoneToSelfCollisionCheck;
			for (int j = 0; j < BoneChain.Num(); j++)
			{
				if (BonesCheckForSelfCollisions[i] == BoneChain[j].Name)
				{
					bAddBoneToSelfCollisionCheck = false;
					break;
				}
			}
			
			if (bAddBoneToSelfCollisionCheck)
				BonesToCheck.Add(BonesCheckForSelfCollisions[i]);
		}
		for (int i = 0; i < BonesToCheck.Num(); i++)
		{
			FRigElementKey OtherBone(BonesToCheck[i], ERigElementType::Bone);
			FTransform OtherBoneTransform = Hierarchy->GetGlobalTransform(OtherBone);

			FVector ForwardVector = FRigUnit_CustomRigNode::GetBoneForwardVector(Hierarchy, Bone);
			FVector OtherBoneForwardVector = FRigUnit_CustomRigNode::GetBoneForwardVector(Hierarchy, OtherBone);

			TArray<FVector> PointsToCheckInCurrentBone;
			TArray<FVector> PointsToCheckInOtherBone;

			PointsToCheckInCurrentBone.Add(BoneTransform.GetLocation());
			PointsToCheckInCurrentBone.Add(BoneTransform.GetLocation() + ForwardVector * (BoneLength / 2));
			PointsToCheckInCurrentBone.Add(BoneTransform.GetLocation() + ForwardVector * BoneLength);

			PointsToCheckInOtherBone.Add(OtherBoneTransform.GetLocation());
			PointsToCheckInOtherBone.Add(OtherBoneTransform.GetLocation() + OtherBoneForwardVector * (BoneLength / 2));
			PointsToCheckInOtherBone.Add(OtherBoneTransform.GetLocation() + OtherBoneForwardVector * BoneLength);

			for (int p = 0; p < 3; p++)
			{
				for (int q = 0; q < 3; q++)
				{
					if (FVector::Dist(PointsToCheckInCurrentBone[p], PointsToCheckInOtherBone[q]) < SegmentRadius * 2)
					{
						FTransform ResultingTransform = BoneTransform;
						FVector VectorTowardsPosition = OtherBoneTransform.GetLocation() - BoneTransform.GetLocation();
						VectorTowardsPosition.Normalize();
						FQuat Angle = FQuat::FindBetweenNormals(ForwardVector, VectorTowardsPosition);
						ResultingTransform.SetRotation(ResultingTransform.GetRotation() * Angle);
						Hierarchy->SetGlobalTransform(Bone, ResultingTransform, false, false);
						BoneTransform = Hierarchy->GetGlobalTransform(Bone);
						ForwardVector = FRigUnit_CustomRigNode::GetBoneForwardVector(Hierarchy, Bone);
					}
				}
			}
		}		
	}
}

FVector FRigUnit_CustomRigNode::GetBoneForwardVector(URigHierarchy* Hierarchy, FRigElementKey Bone)
{
	FVector ForwardVector(1.0, 0.0, 0.0);
	if (!(Bone.Name.ToString().Contains("thigh") || Bone.Name.ToString().Contains("calf") || Bone.Name.ToString().Contains("foot")))
		ForwardVector *= -1;

	if (Bone.Name.ToString().Contains("_l"))
		ForwardVector *= -1;

	ForwardVector = Hierarchy->GetGlobalTransform(Bone).GetRotation().RotateVector(ForwardVector);

	return ForwardVector;
}

FRigUnit_CustomRigNode_Execute()
{
	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy)
	{
		//==================SETTING VARIABLES==================
		TArray<FRigElementKey> BoneChain;
		FRigElementKey ChildBone = Effector;

		for (int i = 0; i < LengthOfChain; i++)
		{
			BoneChain.Add(ChildBone);

			const FRigBaseElement* ChildElement = Hierarchy->Find(ChildBone);

			if (ChildElement)
			{
				FRigElementKey ParentKey = Hierarchy->GetFirstParent(ChildBone);
				ChildBone = ParentKey;
			}
		}

		TArray<FRigElementKey> BoneChainReversed;
		for (int i = 1; i <= LengthOfChain; i++)
		{
			BoneChainReversed.Add(BoneChain[LengthOfChain - i]);
		}

		TArray<float> LimbLengths;

		for (int i = 0; i < LengthOfChain - 1; i++)
		{
			FTransform TransformFirst = Hierarchy->GetGlobalTransform(BoneChain[i]);
			FTransform TransformSecond = Hierarchy->GetGlobalTransform(BoneChain[i + 1]);
			
			LimbLengths.Add(FVector::Dist(TransformFirst.GetLocation(), TransformSecond.GetLocation()));
		}

		FVector BasePointPosition = Hierarchy->GetGlobalTransform(BasePoint).GetLocation();
		for (int i = 0; i < Iterations; i++)
		{
			//==================BACKWARD PASS==================
			FVector PreviousJointAPosition = Hierarchy->GetGlobalTransform(BoneChain[0]).GetLocation();
			Hierarchy->SetGlobalTransform(BoneChain[0], EffectorTransform, false, true);

			for (int j = 0; j < BoneChain.Num() - 1; j++)
			{				
				FRigElementKey JointA = BoneChain[j];
				FRigElementKey JointB = BoneChain[j + 1];

				FTransform JointATransform = Hierarchy->GetGlobalTransform(JointA);
				FTransform JointBTransform = Hierarchy->GetGlobalTransform(JointB);

				FVector OldVector = PreviousJointAPosition - JointBTransform.GetLocation();
				OldVector.Normalize();

				FVector NewVector = JointATransform.GetLocation() - JointBTransform.GetLocation();
				NewVector.Normalize();

				FQuat Angle = FQuat::FindBetweenNormals(OldVector, NewVector);


				PreviousJointAPosition = JointBTransform.GetLocation();

				FTransform ResultingTransform = JointBTransform;
				ResultingTransform.SetRotation(Angle * JointBTransform.GetRotation());

				FVector NewLocation = (JointBTransform.GetLocation() - JointATransform.GetLocation());
				NewLocation.Normalize();
				NewLocation *= LimbLengths[j];
				NewLocation += JointATransform.GetLocation();

				ResultingTransform.SetLocation(NewLocation);


				Hierarchy->SetGlobalTransform(JointB, ResultingTransform, false, false);
				FRigUnit_CustomRigNode::ApplyConstraints(BoneConstraints, ConstraintsAnglesMin, ConstraintsAnglesMax, BonesToCheckForSelfCollisions, Hierarchy, JointB, LimbLengths[j], 5.0f, bShouldAvoidCollisions, bShouldAvoidSelfCollisions, BoneChain);
			}

			//==================FORWARD PASS==================
			PreviousJointAPosition = Hierarchy->GetGlobalTransform(BoneChainReversed[0]).GetLocation();
			FTransform BaseTransform = Hierarchy->GetGlobalTransform(BoneChainReversed[0]);
			BaseTransform.SetLocation(BasePointPosition);

			Hierarchy->SetGlobalTransform(BoneChainReversed[0], BaseTransform, false, false);
			for (int j = 0; j < BoneChainReversed.Num() - 1; j++)
			{
				FRigElementKey JointA = BoneChainReversed[j];
				FRigElementKey JointB = BoneChainReversed[j + 1];

				FTransform JointATransform = Hierarchy->GetGlobalTransform(JointA);
				FTransform JointBTransform = Hierarchy->GetGlobalTransform(JointB);

				FVector OldVector = JointBTransform.GetLocation() - PreviousJointAPosition;
				OldVector.Normalize();

				FVector NewVector = JointBTransform.GetLocation() - JointATransform.GetLocation();
				NewVector.Normalize();

				FQuat Angle = FQuat::FindBetweenNormals(NewVector, OldVector);

				FTransform ResultingTransform = JointATransform;
				ResultingTransform.SetRotation(Angle * JointATransform.GetRotation());

				Hierarchy->SetGlobalTransform(JointA, ResultingTransform, false, j == LimbLengths.Num() - 1);

				PreviousJointAPosition = JointBTransform.GetLocation();

				ResultingTransform = JointBTransform;
				FVector ResultingPosition = JointATransform.GetLocation();

				FVector AddedVector = FRigUnit_CustomRigNode::GetBoneForwardVector(Hierarchy, JointA);
				AddedVector *= LimbLengths[LimbLengths.Num() - 1 - j];

				ResultingPosition += AddedVector;
				ResultingTransform.SetLocation(ResultingPosition);
				Hierarchy->SetGlobalTransform(JointB, ResultingTransform, false, j == LimbLengths.Num() - 1);
			}
		}
	}
}