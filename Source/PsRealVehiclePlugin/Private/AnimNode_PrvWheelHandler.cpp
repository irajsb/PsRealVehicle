// Copyright 2016 Pushkin Studio. All Rights Reserved.

#include "AnimNode_PrvWheelHandler.h"

#include "Animation/AnimInstanceProxy.h"
#include "AnimationRuntime.h"

#include "PrvVehicle.h"
#include "PrvVehicleMovementComponent.h"

/////////////////////////////////////////////////////
// FAnimNode_PrvWheelHandler

FAnimNode_PrvWheelHandler::FAnimNode_PrvWheelHandler()
{
	VehicleSimComponent = nullptr;
}

void FAnimNode_PrvWheelHandler::GatherDebugData(FNodeDebugData& DebugData)
{
#if UE_SERVER
	Super::GatherDebugData(DebugData);
#else
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += "(";
	AddDebugNodeData(DebugLine);
	DebugLine += ")";

	DebugData.AddDebugItem(DebugLine);
	for (const auto& WheelSim : WheelSimulators)
	{
		if (WheelSim.BoneReference.BoneIndex != INDEX_NONE)
		{
			DebugLine = FString::Printf(TEXT(" [Wheel Index : %d] Bone: %s , Rotation Offset : %s, Location Offset : %s"),
				WheelSim.WheelIndex, *WheelSim.BoneReference.BoneName.ToString(), *WheelSim.RotOffset.ToString(), *WheelSim.LocOffset.ToString());
		}
		else
		{
			DebugLine = FString::Printf(TEXT(" [Wheel Index : %d] Bone: %s (invalid bone)"),
				WheelSim.WheelIndex, *WheelSim.BoneReference.BoneName.ToString());
		}

		DebugData.AddDebugItem(DebugLine);
	}

	ComponentPose.GatherDebugData(DebugData);
#endif
}

void FAnimNode_PrvWheelHandler::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
#if !UE_SERVER
	USkeletalMeshComponent* SkeletalMeshComponent = Output.AnimInstanceProxy->GetSkelMeshComponent();
	FCSPose<FCompactPose>& MeshBases = Output.Pose;

	if (MeshBases.GetComponentSpaceFlags().Num() <= 0)
	{
		return;
	}

	const FBoneContainer& BoneContainer = MeshBases.GetPose().GetBoneContainer();
	for (const auto& WheelSim : WheelSimulators)
	{
		if (WheelSim.BoneReference.IsValidToEvaluate(BoneContainer))
		{
			FCompactPoseBoneIndex WheelSimBoneIndex = WheelSim.BoneReference.GetCompactPoseIndex(BoneContainer);

			if (!MeshBases.GetPose().IsValidIndex(WheelSimBoneIndex))
			{
				continue;
			}

			// the way we apply transform is same as FMatrix or FTransform
			// we apply scale first, and rotation, and translation
			// if you'd like to translate first, you'll need two nodes that first node does translate and second nodes to rotate.
			FTransform NewBoneTM = MeshBases.GetComponentSpaceTransform(WheelSimBoneIndex);

			// Apply loc offset
			NewBoneTM.AddToTranslation(WheelSim.LocOffset);

			// Save suspension transform before rotation
			FTransform NewSuspBoneTM = NewBoneTM;

			// Apply rotation offset
			const FQuat BoneQuat(WheelSim.RotOffset);
			NewBoneTM.SetRotation(BoneQuat * NewBoneTM.GetRotation());

			MeshBases.SetComponentSpaceTransform(WheelSimBoneIndex, NewBoneTM);

			// Update suspension
			if (WheelSim.SuspReference.IsValidToEvaluate(BoneContainer))
			{
				FCompactPoseBoneIndex SuspSimBoneIndex = WheelSim.SuspReference.GetCompactPoseIndex(BoneContainer);
				MeshBases.GetComponentSpaceTransform(SuspSimBoneIndex); // for recalculation bone tree
				MeshBases.SetComponentSpaceTransform(SuspSimBoneIndex, NewSuspBoneTM);
			}
		}
	}
#endif
}

bool FAnimNode_PrvWheelHandler::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
#if UE_SERVER
	return false;
#else
	return RequiredBones.GetNumBones() > 0;
#endif
}

void FAnimNode_PrvWheelHandler::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
#if !UE_SERVER
	for (auto& WheelSim : WheelSimulators)
	{
		WheelSim.BoneReference.Initialize(RequiredBones);
		WheelSim.SuspReference.Initialize(RequiredBones);
	}

	// sort by bone indices
	WheelSimulators.Sort([](FPrvWheelSimulator L, FPrvWheelSimulator R) { return L.BoneReference.BoneIndex < R.BoneReference.BoneIndex; });
#endif
}

void FAnimNode_PrvWheelHandler::UpdateInternal(const FAnimationUpdateContext& Context)
{
#if !UE_SERVER
	if (VehicleSimComponent)
	{
		for (auto& WheelSim : WheelSimulators)
		{
			if (VehicleSimComponent->SuspensionData.IsValidIndex(WheelSim.WheelIndex))
			{
				// Zero offset by default
				WheelSim.RotOffset = FRotator::ZeroRotator;
				WheelSim.LocOffset = FVector::ZeroVector;

				const FSuspensionState& Wheel = VehicleSimComponent->SuspensionData[WheelSim.WheelIndex];

				if (Wheel.SuspensionInfo.bAnimateBoneRotation)
				{
					WheelSim.RotOffset.Pitch = FRotator::NormalizeAxis(Wheel.RotationAngle + WheelSim.WheelIndex * 250.f);
					WheelSim.RotOffset.Yaw = FRotator::NormalizeAxis(Wheel.SteeringAngle);
					WheelSim.RotOffset.Roll = 0.f;
				}

				if (Wheel.SuspensionInfo.bAnimateBoneOffset)
				{
					WheelSim.LocOffset.X = 0.f;
					WheelSim.LocOffset.Y = 0.f;
					WheelSim.LocOffset.Z = Wheel.SuspensionInfo.Length - Wheel.VisualLength;
				}

				// Apply wheen bone offset
				WheelSim.LocOffset += Wheel.SuspensionInfo.WheelBoneOffset;

				// Apply just visual offset
				WheelSim.LocOffset += Wheel.SuspensionInfo.VisualOffset;
			}
		}
	}
#endif
}

void FAnimNode_PrvWheelHandler::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
#if !UE_SERVER
	
	// UPrvVehicleAnimInstance
	APawn* Vehicle = Cast<APawn>(Context.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner());

	// we only support vehicle for this class
	if (Vehicle != nullptr)
	{
		VehicleSimComponent = Cast<UPrvVehicleMovementComponent>(Vehicle->GetMovementComponent());

		int32 NumOfwheels = VehicleSimComponent->SuspensionSetup.Num();
		if (NumOfwheels > 0)
		{
			WheelSimulators.Empty(NumOfwheels);
			WheelSimulators.AddZeroed(NumOfwheels);

			// now add wheel data
			for (int32 WheelIndex = 0; WheelIndex < WheelSimulators.Num(); ++WheelIndex)
			{
				FPrvWheelSimulator& WheelSim = WheelSimulators[WheelIndex];
				const FSuspensionInfo& WheelSetup = VehicleSimComponent->SuspensionSetup[WheelIndex];

				// set data
				WheelSim.WheelIndex = WheelIndex;
				WheelSim.BoneReference.BoneName = WheelSetup.BoneName;

				
				// WheelSim.SuspReference.BoneName = WheelSetup.SuspBoneName;
				WheelSim.SuspReference.BoneName = FName(*WheelSetup.BoneName.ToString().Replace(TEXT("Wheel"), TEXT("Suspension"), ESearchCase::Type::CaseSensitive));

				WheelSim.LocOffset = FVector::ZeroVector;
				WheelSim.RotOffset = FRotator::ZeroRotator;
			}
		}
	}
#endif
	Super::Initialize_AnyThread(Context);
}

bool FAnimNode_PrvWheelHandler::CanUpdateInWorkerThread() const
{
#if UE_SERVER
	return true;
#else
	return false;
#endif
}
