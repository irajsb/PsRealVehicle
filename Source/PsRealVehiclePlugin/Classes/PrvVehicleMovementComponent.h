// Copyright 2016 Pushkin Studio. All Rights Reserved.

#pragma once

#include "Curves/CurveFloat.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "AI/Navigation/NavigationAvoidanceTypes.h"
#include "AI/RVOAvoidanceInterface.h"
#include "PrvVehicleMovementComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGearChange, int, index, bool, Gearup);
/** Rigid body error correction data */


USTRUCT()
struct FPIDController
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "PID")
	float Proportional;

	UPROPERTY(EditAnywhere, Category = "PID")
	float Integral;

	UPROPERTY(EditAnywhere, Category = "PID")
	float Derivative;

	UPROPERTY(EditAnywhere, Category = "PID")
	float ErrorMin;

	UPROPERTY(EditAnywhere, Category = "PID")
	float ErrorMax;

	float ErrorSum;
	float LastPosition;

	FPIDController() {}

	FPIDController(float P, float I, float D, float ErrorMin, float ErrorMax)
	{
		Proportional = P;
		Integral = I;
		Derivative = D;
		this->ErrorMin = ErrorMin;
		this->ErrorMax = ErrorMax;
	}

	float CalcNewInput(float Error, float Position);
};

USTRUCT()
struct FOldRigidBodyErrorCorrection
{
	GENERATED_USTRUCT_BODY()

	/** max squared position difference to perform velocity adjustment */
	UPROPERTY()
	float LinearDeltaThresholdSq;

	/** strength of snapping to desired linear velocity */
	UPROPERTY()
	float LinearInterpAlpha;

	/** inverted duration after which linear velocity adjustment will fix error */
	UPROPERTY()
	float LinearRecipFixTime;

	/** max squared angle difference (in radians) to perform velocity adjustment */
	UPROPERTY()
	float AngularDeltaThreshold;

	/** strength of snapping to desired angular velocity */
	UPROPERTY()
	float AngularInterpAlpha;

	/** inverted duration after which angular velocity adjustment will fix error */
	UPROPERTY()
	float AngularRecipFixTime;

	/** min squared body speed to perform velocity adjustment */
	UPROPERTY()
	float BodySpeedThresholdSq;

	FOldRigidBodyErrorCorrection()
		: LinearDeltaThresholdSq(5.0f)
		, LinearInterpAlpha(0.2f)
		, LinearRecipFixTime(1.0f)
		, AngularDeltaThreshold(0.2f * PI)
		, AngularInterpAlpha(0.1f)
		, AngularRecipFixTime(1.0f)
		, BodySpeedThresholdSq(0.2f)
	{
	}
};

USTRUCT(BlueprintType)
struct FSuspensionInfo
{
	GENERATED_USTRUCT_BODY()

	/** If yes, suspension location and rotation will be extracted from the bone transofrm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bInheritWheelBoneTransform;

	/** Bone name to get the wheel transform */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension /*, meta = (EditCondition = "bInheritWheelBoneTransform")*/)
	FName BoneName;

	/** Suspension location in Actor space */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "!bInheritWheelBoneTransform", DisplayName = "Suspension Location"))
	FVector Location;

	/** Suspension rotation in Actor space */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (EditCondition = "!bInheritWheelBoneTransform", DisplayName = "Suspension Rotation"))
	FRotator Rotation;

	/** Should wheel bone be animated with suspension compression offset? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Animation)
	bool bAnimateBoneOffset;

	/** Should wheel bone be animated with wheel rotation? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Animation)
	bool bAnimateBoneRotation;

	/** Is wheel belongs to the right track or the left one? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension)
	bool bRightTrack;

	/** Is wheel generates driving force [ignored by tracked vehicles] */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bDrivingWheel;

	/** Is wheel is influenced by steering [ignored by tracked vehicles] */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bSteeringWheel;

	/** Should the dust or not */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bSpawnDust;

	/** If yes, wheel will use settings from below */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension)
	bool bCustomWheelConfig;

	/** Wheel relative offset from its bone.
	 * Attn.! Ignored when suspension is not inherited from bone */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig", DisplayName = "Wheel Offset"))
	FVector WheelBoneOffset;

	/** How far the wheel can go above the resting position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig", DisplayName = "Suspension Length"))
	float Length; /** SuspensionMaxRaise */

	/** How far the wheel can drop below the resting position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig", DisplayName = "Suspension Max Drop"))
	float MaxDrop;

	/** Wheel [collision] radius */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig"))
	float CollisionRadius;

	/** Wheel [collision] width. Set 0.f to use spherical collision */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig"))
	float CollisionWidth;

	/** Wheel relative bone offset for animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig"))
	FVector VisualOffset;

	/** How strong wheel reacts to compression [N/cm] */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig"))
	float Stiffness;

	/** How fast wheel becomes stable on compression [N/(cm/s)] */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig"))
	float CompressionDamping;

	/** How fast wheel becomes stable on decompression [N/(cm/s)] */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Suspension, meta = (EditCondition = "bCustomWheelConfig"))
	float DecompressionDamping;

	/** Defaults */
	FSuspensionInfo()
	{
		bInheritWheelBoneTransform = true;

		Location = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;

		bAnimateBoneOffset = true;
		bAnimateBoneRotation = true;

		bRightTrack = false;
		bDrivingWheel = true;
		bSteeringWheel = false;
		bSpawnDust = false;

		bCustomWheelConfig = false;

		WheelBoneOffset = FVector::ZeroVector;
		Length = 25.f;
		MaxDrop = 10.f;
		CollisionRadius = 36.f;
		CollisionWidth = 20.f;
		VisualOffset = FVector::ZeroVector;
		Stiffness = 4000000.f;			  // [N/cm]
		CompressionDamping = 4000000.f;	  // [N/(cm/s)]
		DecompressionDamping = 4000000.f; // [N/(cm/s)]
	}
};

USTRUCT(BlueprintType)
struct FSuspensionState
{
	GENERATED_USTRUCT_BODY()

	/** Wheel initial suspension config */
	UPROPERTY(BlueprintReadWrite)
	FSuspensionInfo SuspensionInfo;

	/** Effective suspension length on last tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PreviousLength;

	/** Suspension length for visuals (including MaxDrop interval) */
	UPROPERTY(BlueprintReadOnly, Category = Visuals)
	float VisualLength;

	/** Current wheel rotation angle (pitch) */
	UPROPERTY(BlueprintReadOnly, Category = Visuals)
	float RotationAngle;

	/** Current wheel steering angle (yaw) */
	UPROPERTY(BlueprintReadOnly, Category = Visuals)
	float SteeringAngle;

	/** Force that was generated by suspension compression */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector SuspensionForce;

	/**  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector WheelCollisionLocation;

	/**  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector WheelCollisionNormal;

	/**  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector PreviousWheelCollisionVelocity;

	/**  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float WheelLoad;

	/** Is wheel engaged into physics simulation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool WheelTouchedGround;

	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	/** */
	UPROPERTY(Transient)
	UParticleSystemComponent* DustPSC;

	/** Defaults */
	FSuspensionState()
	{
		PreviousLength = 0.f;
		VisualLength = 0.f;

		RotationAngle = 0.f;
		SteeringAngle = 0.f;

		SuspensionForce = FVector::ZeroVector;
		WheelCollisionLocation = FVector::ZeroVector;
		WheelCollisionNormal = FVector::UpVector;
		PreviousWheelCollisionVelocity = FVector::ZeroVector;

		WheelLoad = 0.f;
		WheelTouchedGround = false;

		SurfaceType = EPhysicalSurface::SurfaceType_Default;
		DustPSC = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FGearInfo
{
	GENERATED_USTRUCT_BODY()

	/** Determines the amount of torque multiplication */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Ratio;

	/** Value of engineRevs/maxEngineRevs that is low enough to gear down */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float DownRatio;

	/** Value of engineRevs/maxEngineRevs that is high enough to gear up */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float UpRatio;

	/** Defaults */
	FGearInfo()
	{
		Ratio = 0.f;
		DownRatio = 0.15f;
		UpRatio = 0.9f;
	}
};

USTRUCT(BlueprintType)
struct FTrackInfo
{
	GENERATED_USTRUCT_BODY()

	/**  */
	UPROPERTY(Transient)
	float Input;

	/**  */
	UPROPERTY(Transient)
	float TorqueTransfer;

	/**  */
	UPROPERTY(Transient)
	float LinearSpeed;

	/** Driving angular velocity */
	UPROPERTY(Transient)
	float AngularSpeed;

	/**  */
	UPROPERTY(Transient)
	float DriveTorque;

	/**  */
	

	/**  */
	

	/**  */
	UPROPERTY(Transient)
	float BrakeRatio;

	/**  */
	UPROPERTY(Transient)
	FVector DriveForce;

	/** Defaults */
	FTrackInfo()
	{
		Input = 0.f;
		TorqueTransfer = 0.f;

		LinearSpeed = 0.f;
		AngularSpeed = 0.f;

		DriveTorque = 0.f;
	

		BrakeRatio = 0.f;
		DriveForce = FVector::ZeroVector;
	}
};

USTRUCT()
struct FRepCosmeticData
{
	GENERATED_USTRUCT_BODY()

	/** Engine RPM */
	UPROPERTY()
	uint8 EngineRPM;

	/** Speed for tracks animation [left] */
	UPROPERTY()
	int8 LeftTrackEffectiveAngularSpeed;

	/** Speed for tracks animation [right] */
	UPROPERTY()
	int8 RightTrackEffectiveAngularSpeed;

	/** Speed for wheels animation */
	UPROPERTY()
	int8 EffectiveSteeringAngularSpeed;

	FRepCosmeticData()
	{
		EngineRPM = 0;
		LeftTrackEffectiveAngularSpeed = 0;
		RightTrackEffectiveAngularSpeed = 0;
		EffectiveSteeringAngularSpeed = 0;
	}
};

struct FAnimNode_PrvWheelHandler;

/**
 * Component that uses Torque and Force to move tracked vehicles
 */
UCLASS(config = Game, defaultconfig)
class PSREALVEHICLEPLUGIN_API UPrvVehicleMovementComponent : public UPawnMovementComponent,public IRVOAvoidanceInterface
{
	GENERATED_UCLASS_BODY()

	// Let direct access for animation nodes
	friend FAnimNode_PrvWheelHandler;

protected:
	//////////////////////////////////////////////////////////////////////////
	// Initialization

	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//////////////////////////////////////////////////////////////////////////
	// Physics initialization

	void InitMesh();
	void InitBodyPhysics();
	void InitSuspension();
	void InitGears();
	void CalculateMOI();

	//////////////////////////////////////////////////////////////////////////
	// Physics simulation

	bool IsSleeping(float DeltaTime);
	void ResetSleep();

	/** [client/server] */
	UFUNCTION()
	void OnRep_IsSleeping();

	void UpdateSteering(float DeltaTime);
	void UpdateThrottle(float DeltaTime);
	void UpdateGearBox();
	void UpdateBrake(float DeltaTime);

	void UpdateTracksVelocity(float DeltaTime);
	void UpdateHullVelocity(float DeltaTime);

	/** Calculate value of Start extra power */
	void UpdateEngineStartExtraPower(float DeltaTime);

	void UpdateEngine();
	void UpdateDriveForce();
	void UpdateSound(float DeltaTime);

	/** Tick of anti-rollover system */
	void UpdateAntiRollover(float DeltaTime);

	void UpdateSuspension(float DeltaTime);

	/** Trace just to put wheels on the ground, don't calculate physics (used for proxy actors) */
	void UpdateSuspensionVisualsOnly(float DeltaTime);

	void UpdateFriction(float DeltaTime);
	void UpdateLinearVelocity(float DeltaTime);
	void UpdateAngularVelocity(float DeltaTime);

	void AnimateWheels(float DeltaTime);

	float ApplyBrake(float DeltaTime, float AngularVelocity, float BrakeRatio);
	

	/** Shift gear up or down 
	 * Attn.! It doesn't think about why it happend, so it should be done externally!) */
	void ShiftGear(bool bShiftUp);
	/*Same but for timers*/
	void ShiftGearByTimer();

	//////////////////////////////////////////////////////////////////////////
	// Network

	/** Pass current state to server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerUpdateState(uint16 InQuantizeInput);



	/*
	 * AI
	 */
	public:

	UPROPERTY(EditAnywhere, Category = "PID")
	FPIDController ThrottleController = FPIDController(0.f, 0.f, 0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "PID")
	FPIDController SteeringController = FPIDController(0, 0, 0, 0, 0);

	FVector InitialLocation;
	FVector InitialDirection;
	bool bTurningAround = false;
	bool bTurningForward=false;
	float TempThrottle;
	float TempSteer;
	
	/** Compute steering input */
	float CalcSteeringInput();
	/** Compute throttle input */
	bool AIMoving;
	virtual float CalcThrottleInput();
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	
	virtual void StopActiveMovement() override;
	UPROPERTY(BlueprintReadOnly)
	FVector PathMoveInput;

	/*avoidance*/
	/** If set, component will use RVO avoidance */

	virtual float GetMaxSpeed() const override;
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	float MaxSpeed=1000000;
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	uint8 bUseRVOAvoidance : 1;
	//let others dodge us
	uint8 bIsPlayerRVO : 1;
	/** Was avoidance updated in this frame? */
	UPROPERTY(Transient)
	uint32 bWasAvoidanceUpdated : 1;


	// RVO Avoidance

	/** Vehicle Radius to use for RVO avoidance (usually half of vehicle width) */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	float RVOAvoidanceRadius;
	
	/** Vehicle Height to use for RVO avoidance (usually vehicle height) */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	float RVOAvoidanceHeight;
	
	/** Area Radius to consider for RVO avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite)
	float AvoidanceConsiderationRadius;

	/** Value by which to alter steering per frame based on calculated avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float RVOSteeringStep;

	/** Value by which to alter throttle per frame based on calculated avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float RVOThrottleStep;
	
	/** calculate RVO avoidance and apply it to current velocity */
	virtual void CalculateAvoidanceVelocity(float DeltaTime);

	/** No default value, for now it's assumed to be valid if GetAvoidanceManager() returns non-NULL. */
	UPROPERTY(Category = "Avoidance", VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay)
	int32 AvoidanceUID;

	/** Moving actor's group mask */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask AvoidanceGroup;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement", meta = (DeprecatedFunction, DeprecationMessage = "Please use SetAvoidanceGroupMask function instead."))
	void SetAvoidanceGroup(int32 GroupFlags);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetAvoidanceGroupMask(const FNavAvoidanceMask& GroupMask);
	virtual void BeginPlay() override;
	/** Will avoid other agents if they are in one of specified groups */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask GroupsToAvoid;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement", meta = (DeprecatedFunction, DeprecationMessage = "Please use SetGroupsToAvoidMask function instead."))
	void SetGroupsToAvoid(int32 GroupFlags);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetGroupsToAvoidMask(const FNavAvoidanceMask& GroupMask);

	/** Will NOT avoid other agents if they are in one of specified groups, higher priority than GroupsToAvoid */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FNavAvoidanceMask GroupsToIgnore;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement", meta = (DeprecatedFunction, DeprecationMessage = "Please use SetGroupsToIgnoreMask function instead."))
	void SetGroupsToIgnore(int32 GroupFlags);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetGroupsToIgnoreMask(const FNavAvoidanceMask& GroupMask);

	/** De facto default value 0.5 (due to that being the default in the avoidance registration function), indicates RVO behavior. */
	UPROPERTY(Category = "Avoidance", EditAnywhere, BlueprintReadOnly)
	float AvoidanceWeight;
	
	/** Temporarily holds launch velocity when pawn is to be launched so it happens at end of movement. */
	UPROPERTY()
	FVector PendingLaunchVelocity;
	
	/** Change avoidance state and register with RVO manager if necessary */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|WheeledVehicleMovement")
	void SetAvoidanceEnabled(bool bEnable);
	/** Update RVO Avoidance for simulation */
	void UpdateAvoidance(float DeltaTime);
		
	/** called in Tick to update data in RVO avoidance manager */
	void UpdateDefaultAvoidance();
	
	/** lock avoidance velocity */
	void SetAvoidanceVelocityLock(class UAvoidanceManager* Avoidance, float Duration);
	
	/** Calculated avoidance velocity used to adjust steering and throttle */
	FVector AvoidanceVelocity;
	
	/** forced avoidance velocity, used when AvoidanceLockTimer is > 0 */
	FVector AvoidanceLockVelocity;
	
	/** remaining time of avoidance velocity lock */
	float AvoidanceLockTimer;

	/** Handle for delegate registered on mesh component */
	FDelegateHandle MeshOnPhysicsStateChangeHandle;
	
	/** BEGIN IRVOAvoidanceInterface */
	virtual void SetRVOAvoidanceUID(int32 UID) override;
	virtual int32 GetRVOAvoidanceUID() override;
	virtual void SetRVOAvoidanceWeight(float Weight) override;
	virtual float GetRVOAvoidanceWeight() override;
	virtual FVector GetRVOAvoidanceOrigin() override;
	virtual float GetRVOAvoidanceRadius() override;
	virtual float GetRVOAvoidanceHeight() override;
	virtual float GetRVOAvoidanceConsiderationRadius() override;
	virtual FVector GetVelocityForRVOConsideration() override;
	
	virtual int32 GetAvoidanceGroupMask() override;
	virtual void SetAvoidanceGroupMask(int32 GroupFlags) override;
	 void SetGroupsToAvoidMask(int32 GroupFlags) override;
	virtual void SetGroupsToIgnoreMask(int32 GroupFlags) override;
	virtual int32 GetGroupsToAvoidMask() override;
	
	virtual int32 GetGroupsToIgnoreMask() override;
	/** END IRVOAvoidanceInterface */
	
	/**/
	// MAKE ALL CONFIG PUBLIC
public:
	/////////////////////////////////////////////////////////////////////////
	// Vehicle setup

	/** Is it a car? Tank by default */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	bool bWheeledVehicle;

	/** Distance from forward wheels to back one */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle, meta = (editcondition = "bWheeledVehicle", ClampMin = "0.001", UIMin = "0.001"))
	float TransmissionLength;

	/** If true, mass will not be automatically computed and you must set it directly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	bool bOverrideMass;

	/** Mass of the body in KG. By default we compute this based on physical material and mass scale */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle, meta = (editcondition = "bOverrideMass", ClampMin = "0.001", UIMin = "0.001", DisplayName = "Vehicle Mass"))
	float OverrideVehicleMass;

	

	/** */
	

	/** */




	


	/** 'Drag' force added to reduce linear movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vehicle)
	float LinearDamping;

	/** 'Drag' force added to reduce angular movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vehicle)
	float AngularDamping;

	/** User specified offset for the center of mass of this object, from the calculated location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vehicle, meta = (DisplayName = "Center Of Mass Offset"))
	FVector COMOffset;

	/** Kg */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	float SprocketMass;

	/** Cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	float SprocketRadius;

	/** Kg */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	float TrackMass;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	float SleepLinearVelocity;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	float SleepAngularVelocity;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	float SleepDelay;

	/** Whether gravity is disabled for ROLE_SimulatedProxy */
	bool bDisableGravityForSimulated;

	/** Force this surface type to spawn dust */
	TEnumAsByte<EPhysicalSurface> ForceSurfaceType;

	/////////////////////////////////////////////////////////////////////////
	// Anti rollover

	/** Enable anti rollover */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	bool bEnableAntiRollover;

	/** Anti rollover value threshold */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle, meta = (EditCondition = "bEnableAntiRollover"))
	float AntiRolloverValueThreshold;

	/** Relation between sine of Z axis delta angle and anti-rollover force applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vehicle, meta = (editcondition = "bEnableAntiRollover"))
	FRuntimeFloatCurve AntiRolloverForceCurve;

	/** Value of sine alpha in the last tick */
	UPROPERTY(Transient)
	float LastAntiRolloverValue;

	/////////////////////////////////////////////////////////////////////////
	// Suspension setup

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	TArray<FSuspensionInfo> SuspensionSetup;

	/** Wheel relative offset from its bone.
	* Attn.! Ignored when suspension is not inherited from bone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	FVector DefaultWheelBoneOffset;

	/** How far the wheel can go above the resting position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DefaultLength; /** SuspensionMaxRaise */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float 	AntiSlipFactor;
	/** How far the wheel can drop below the resting position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DefaultMaxDrop;

	/** Wheel [collision] radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DefaultCollisionRadius;

	/** Wheel [collision] width. Set 0.f to use spherical collision */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DefaultCollisionWidth;

	/** Wheel relative bone offset for animation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	FVector DefaultVisualOffset;

	/** Wheel collision radius for animation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float VisualCollisionRadius;

	/** How strong wheel reacts to compression */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DefaultStiffness;

	/** How fast wheel becomes stable on compression */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DefaultCompressionDamping;

	/** How fast wheel becomes stable on decompression */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DefaultDecompressionDamping;

	/** Global factor that applied to all wheels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float StiffnessFactor;

	/** Global factor that applied to all wheels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float CompressionDampingFactor;

	/** Global factor that applied to all wheels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DecompressionDampingFactor;

	/** Discrete damping correction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	bool bCustomDampingCorrection;

	/** Discrete damping correction factor: 0 - disabled correction, 1 - hypercorrection  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension, meta = (EditCondition = "bCustomDampingCorrection", ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float DampingCorrectionFactor;

	/** If yes, damping will consider active friction points number */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	bool bAdaptiveDampingCorrection;

	/** How fast wheels are animated while going down */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	float DropFactor;

	/**	Should 'Hit' events fire when this object collides during physics simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Suspension, meta = (DisplayName = "Simulation Generates Hit Events"))
	bool bNotifyRigidBodyCollision;

	/** If set, trace will run on complex collisions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Suspension)
	bool bTraceComplex;

	/** Default suspension trace query */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Suspension)
	TEnumAsByte<ETraceTypeQuery> SuspensionTraceTypeQuery;

	/** Clamp SuspensionForce above zero */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	bool bClampSuspensionForce;

	/** Suspension use line trace always (only for dedicated server) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	bool bSimplifiedSuspension;

	/** Suspension use line trace if Throttle == 0 (only for dedicated server) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	bool bSimplifiedSuspensionWithoutThrottle;

	/** Suspension use line trace by camera (only for client) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Suspension)
	bool bSimplifiedSuspensionByCamera;

public:
	/////////////////////////////////////////////////////////////////////////
	// Engine setup

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float ThrottleUpRatio;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float ThrottleDownRatio;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float DifferentialRatio;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float TransmissionEfficiency;

	/** Torque (Nm) at a given RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	FRuntimeFloatCurve EngineTorqueCurve;


	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float CustomTorqueMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float CustomForceMuliplier;
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float EngineExtraPowerRatio;

	/** Extra power for rear drive */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float EngineRearExtraPowerRatio;

	/** Extra power applied for a period of time after movement start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float StartExtraPowerRatio;

	/** Extra power after start duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float StartExtraPowerDuration;

	/** Extra power after start cooldown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	float StartExtraPowerCooldown;

	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = EngineSetup)
	float TorqueTransferThrottleFactor;

	/** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = EngineSetup)
	float TorqueTransferSteeringFactor;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	bool bLimitMaxSpeed;

	/** MaxSpeed (Cm/s) at a given angular speed (Yaw) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup, meta = (editcondition = "bLimitMaxSpeed"))
	FRuntimeFloatCurve MaxSpeedCurve;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EngineSetup)
	bool bScaleForceToActiveFrictionPoints;

	/////////////////////////////////////////////////////////////////////////
	// Gear Box setup

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GearBox)
	TArray<FGearInfo> GearSetup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GearBox)
	bool bAutoGear;

	/** Minimum time it takes the automatic transmission to initiate a gear change (seconds)*/
	UPROPERTY(EditAnywhere, Category = GearBox, meta = (editcondition = "bAutoGear", ClampMin = "0.0", UIMin = "0.0"))
	float GearAutoBoxLatency;

	/////////////////////////////////////////////////////////////////////////
	// Steering setup

	/** Use angular velocity hack instead of torque transfer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup)
	bool bAngularVelocitySteering;

	/** Steering rotation angular speed (basic, before throttle ratio is applied) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bAngularVelocitySteering", ClampMin = "0.0", UIMin = "0.0"))
	float SteeringAngularSpeed;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bAngularVelocitySteering", ClampMin = "0.0", UIMin = "0.0"))
	float SteeringUpRatio;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bAngularVelocitySteering", ClampMin = "0.0", UIMin = "0.0"))
	float SteeringDownRatio;

	/** Threshold of setting friction for TargetSteeringVelocity to 1.f */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bAngularVelocitySteering", ClampMin = "0.0", UIMin = "0.0"))
	float AngularSteeringFrictionThreshold;

	/** Whether to use steering curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup)
	bool bUseSteeringCurve;

	/** Maximum steering at given forward speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bUseSteeringCurve"))
	FRuntimeFloatCurve SteeringCurve;

	/** Whether we use SteeringCurve(0) steering without throttle input */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bUseSteeringCurve"))
	bool bMaximizeZeroThrottleSteering;

	/** Use ActiveDrivenFrictionPoints in steering or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bAngularVelocitySteering"))
	bool bUseActiveDrivenFrictionPoints;

	/** Threshold of using autobrake for "full-steering-zero-throttle" situation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bAngularVelocitySteering", ClampMin = "0.0", UIMin = "0.0"))
	float AutoBrakeSteeringThreshold;

	/** Steering rotation angular speed modificator for SteeringCurve and MaxSpeedCurve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup, meta = (editcondition = "bAngularVelocitySteering", ClampMin = "0.0", UIMin = "0.0"))
	float TurnRateModAngularSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SteeringSetup)
	float AirControl;

	/////////////////////////////////////////////////////////////////////////
	// Brake system

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem)
	float BrakeForce;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem)
	bool bAutoBrake;

	/** AutoBrakeUpRatio at given speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (editcondition = "bAutoBrake"))
	FRuntimeFloatCurve AutoBrakeUpRatio;

	/** How much brake applied by auto-brake system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (editcondition = "bAutoBrake"))
	float AutoBrakeFactor;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (editcondition = "!bAngularVelocitySteering"))
	float SteeringBrakeTransfer;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (editcondition = "!bAngularVelocitySteering"))
	float SteeringBrakeFactor;

	/** Attn.! Has almost no effect with Angular Velocity Steering System */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem)
	bool bSteeringStabilizer;

	/** Flag if steering stabilizer is active */
	bool bSteeringStabilizerActiveLeft;

	/** Flag if steering stabilizer is active */
	bool bSteeringStabilizerActiveRight;

	/** Minimum amount (ABS) of Hull angular velocity to use steering stabilizer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (EditCondition = "bSteeringStabilizer"))
	float SteeringStabilizerMinimumHullVelocity;

	/** Minimum difference in speed of tracks to activate SteeringStabilizer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (EditCondition = "bSteeringStabilizer"))
	float AutoBrakeActivationDelta;

	/** How much brake should be applied when stabilizer is working */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (EditCondition = "bSteeringStabilizer"))
	float SteeringStabilizerBrakeFactor;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (EditCondition = "bSteeringStabilizer", ClampMin = "0.0", UIMin = "0.0"))
	float SteeringStabilizerBrakeUpRatio;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (editcondition = "bLimitMaxSpeed", ClampMin = "0.0", UIMin = "0.0"))
	float SpeedLimitBrakeFactor;

	/** */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BrakeSystem, meta = (editcondition = "bLimitMaxSpeed", ClampMin = "0.0", UIMin = "0.0"))
	float SpeedLimitBrakeUpRatio;




	
protected:
	float FinalMOI;

	TArray<FSuspensionState> SuspensionData;
	int32 LastGear;
	int32 NeutralGear;
	int32 CurrentGear;
	FTimerHandle GearChangeHandle;
	UPROPERTY(EditAnyWhere,Category=Gearbox)
	float fGearboxLatency;
	UPROPERTY(EditAnyWhere, Category = Gearbox)
	bool bZeroTorqueWhenShifting;
	bool bGearTimer;
	bool bPendingShiftUp;
	UPROPERTY(BlueprintReadOnly)
	bool bHasEngineLoad;
	bool bReverseGear;

	float LastAutoGearShiftTime;
	float LastAutoGearHullSpeed;

	FTrackInfo LeftTrack;
	FTrackInfo RightTrack;

	float RightTrackTorque;
	float LeftTrackTorque;

	float HullAngularSpeed;

	/** Cached RPM limits */
	float MinEngineRPM;
	float MaxEngineRPM;

	/** Engine RPM */
	float EngineRPM;

	float EngineTorque;
	float DriveTorque;

	/** Current value of Start extra power */
	float StartExtraPower;

	/** Time of activation of Start extra power */
	float StartExtraPowerActivationTime;

	/** Start extra power moving flag last value */
	bool bStartExtraPowerMovingLast;

	/** Angular steering speed */
	float TargetSteeringAngularSpeed;

	/** Used for wheels animation */
	float EffectiveSteeringAngularSpeed;

	/** Vector computed from EffectiveSteeringAngularSpeed */
	FVector EffectiveSteeringVelocity;

	/** How many wheels are touched the ground */
	int32 ActiveFrictionPoints;

	/** How many drive wheels are touched the ground */
	int32 ActiveDrivenFrictionPoints;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_IsSleeping)
	uint32 bIsSleeping : 1;

	float SleepTimer;

	float LastSteeringStabilizerBrakeRatio;
	float LastSpeedLimitBrakeRatio;



	/** The time we applied a small correction to body's Position or Orientation */
	float CorrectionBeganTime;

	/** The time correction to body's Position or Orientation ends */
	float CorrectionEndTime;

	/** If the body is under effect of Position or Orientation correction */
	bool bCorrectionInProgress;

	/** Target state of ongoing correction */
	FRigidBodyState CorrectionEndState;

	/** Correction thresholds cached info */
	FOldRigidBodyErrorCorrection ErrorCorrectionData;

	/** Whether we are in "full steering" situation and speed is above AutoBrakeSteeringThreshold */
	bool bAutoBrakeSteering;

public:
	/** Velocity for tracks animation [left] */
	float LeftTrackEffectiveAngularSpeed;

	/** Velocity for tracks animation [right] */
	float RightTrackEffectiveAngularSpeed;

	//////////////////////////////////////////////////////////////////////////
	// Custom physics handling

public:
	/** @todo #58 Error correction data for replicating simulated physics (rigid bodies) */
	UPROPERTY()
	FOldRigidBodyErrorCorrection PhysicErrorCorrection;

public:
	bool ConditionalApplyRigidBodyState(FRigidBodyState& UpdatedState, const FOldRigidBodyErrorCorrection& ErrorCorrection, FVector& OutDeltaPos, FName BoneName = NAME_None);

protected:
	bool ApplyRigidBodyState(const FRigidBodyState& NewState, const FOldRigidBodyErrorCorrection& ErrorCorrection, FVector& OutDeltaPos, FName BoneName = NAME_None);

	//////////////////////////////////////////////////////////////////////////
	// Vehicle control

public:

	
	/** Set the user input for the vehicle throttle */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void SetThrottleInput(float Throttle);

	/** Set the user input for the vehicle steering */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void SetSteeringInput(float Steering);

	/** Set the user input for handbrake */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void SetHandbrakeInput(bool bNewHandbrake);

	/** Make movement possible */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void EnableMovement();

	/** Make movement impossible */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void DisableMovement();

	/** Is vehicle trying to move (movenent is enabled and it has player input) */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	bool IsMoving() const;

	/** [server value] Get last user steering input */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	int32 GetLastUserSteeringInput() const;

protected:
	/** */
	UPROPERTY(Transient, Replicated)
	bool bIsMovementEnabled;

	/** [server] Last user steering input */
	int32 LastUserSteeringInput;

	/////////////////////////////////////////////////////////////////////////
	// Animation control

public:
	/** Should be wheels animated for non-authority actors or not */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void SetWheelsAnimationEnabled(bool bAnimateWheels);

protected:
	/** State used for non-authority actors */
	bool bShouldAnimateWheels;

	//////////////////////////////////////////////////////////////////////////
	// Vehicle stats

public:
	/** How fast the vehicle is moving forward */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetForwardSpeed() const;

	/** Get current real throttle value */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetThrottle() const;

	/** Get current engine's rotation speed */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetEngineRotationSpeed() const;

	/** Get current engine's max rotation speed */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetEngineMaxRotationSpeed() const;

	/** Get current engine torque */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetEngineTorque() const;

	/** Get left track drive torque */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetDriveTorqueLeft() const;

	/** Get right track drive torque */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetDriveTorqueRight() const;

	/** Get left track angular velocity */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetAngularVelocityLeft() const;

	/** Get right track angular velocity */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetAngularVelocityRight() const;

	/** Get left track brake ratio */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetBrakeRatioLeft() const;

	/** Get left track brake ratio */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	float GetBrakeRatioRight() const;

	/** Is vehicle is touching ground with wheels */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	bool HasTouchGround() const;

	//////////////////////////////////////////////////////////////////////////
	// Tracks info

public:
	/** Get left track internal data */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void GetTrackInfoLeft(FTrackInfo& OutTrack) const;

	/** Get right track internal data */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void GetTrackInfoRight(FTrackInfo& OutTrack) const;

	//////////////////////////////////////////////////////////////////////////
	// Data access

public:
	/** Get current gear */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	int32 GetCurrentGear() const;

	/** Get neutral gear */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	int32 GetNeutralGear() const;

	/** Get gear reverse state */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	bool IsCurrentGearReverse() const;

	/** Get gearbox config */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	FGearInfo GetGearInfo(int32 GearNum) const;

	/** Get current gearbox config */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	FGearInfo GetCurrentGearInfo() const;

	/** Get current suspension state */
	const TArray<FSuspensionState>& GetSuspensionData() const;

	/** Get raw steering input */
	float GetRawSteeringInput() const;

	/** Get raw throttle input */
	float GetRawThrottleInput() const;

protected:
	/** Get the mesh this vehicle is tied to */
	class USkinnedMeshComponent* GetMesh();

public:
	/** The mesh we move and update */
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient)
	USkinnedMeshComponent* UpdatedMesh;

	//////////////////////////////////////////////////////////////////////////
	// Effects

protected:
	/** */
	void UpdateWheelEffects(float DeltaTime);

	/** */
	UParticleSystemComponent* SpawnNewWheelEffect(FName InSocketName = NAME_None, FVector InSocketOffset = FVector::ZeroVector);

protected:
	/** */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	class UPrvVehicleDustEffect* DustEffect;

	/** Use vehicle mesh rotation for dust effect, if false - it uses wheel rotation instead */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	bool bUseMeshRotationForEffect;

	//////////////////////////////////////////////////////////////////////////
	// Debug

public:
	/** Draw debug text for the wheels and suspension */
	virtual void DrawDebug(UCanvas* Canvas, float& YL, float& YPos);

	/** Draw debug primitives for the wheels and suspension */
	virtual void DrawDebugLines();

	/** */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	void ShowDebug(bool bEnableDebug) { bShowDebug = bEnableDebug; }

	/** */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	bool IsDebug() { return bShowDebug; }

public:


	/* OnGearUp*/
	UPROPERTY(BlueprintAssignable)
	FOnGearChange GearChange;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bShowDebug;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bDebugAutoGearBox;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bDebugDampingCorrection;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bDebugCustomDamping;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bDebugSuspensionLimits;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bUseCustomVelocityCalculations;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bForceNeverSleep;

	/** Make zero engine torque when MaxRPM is reached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bLimitEngineTorque;

protected:
	/** 
	 * Don't apply forces for simulated proxy locally 
	 *
	 * Attn.! Override it only in game config
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Debug)
	bool bFakeAutonomousProxy;

	//////////////////////////////////////////////////////////////////////////
	// Internal data

public:
	/** */
	UFUNCTION(BlueprintCallable, Category = "PsRealVehicle|Components|VehicleMovement")
	bool HasInput() const;

protected:
	/** Don't apply forces for simulated proxy locally */
	bool ShouldAddForce();

	/** Use line trace */
	bool UseLineTrace();

	/** Get camera vector (for client only) */
	bool GetCameraVector(FVector& RelativeCameraVector, FVector& RelativeMeshForwardVector);

protected:
	/** What the player has the steering set to. Range -1...1 */
	UPROPERTY(Transient)
	float RawSteeringInput;

	/** What the player has the accelerator set to. Range -1...1 */
	UPROPERTY(Transient)
	float RawThrottleInput;

	/** Last Raw throttle input */
	float LastRawThrottleInput;

	/** True if the player is holding the handbrake */
	UPROPERTY(Transient)
	uint32 bRawHandbrakeInput : 1;

	/** Contains: throttle (1), steering (2), handbrake(3). 
	 *  3222 2222 1111 1111
	 */
	UPROPERTY(Transient)
	uint16 QuantizeInput;

	UPROPERTY(Transient)
	float ThrottleInput;

	UPROPERTY(Transient)
	float SteeringInput;

	UPROPERTY(Transient)
	float BrakeInput;

private:
	/** Keep real value of throttle while steering stabilizer is active */
	UPROPERTY(Transient)
	float RawThrottleInputKeep;

protected:
	/** Pack cosmetic data into optimized replicated variable */
	void UpdateReplicatedCosmeticData();

	/** Replciated cosmetic data  */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_RepCosmeticData)
	FRepCosmeticData RepCosmeticData;

	UFUNCTION()
	void OnRep_RepCosmeticData();

	public:

	UPROPERTY(BlueprintReadOnly)
	float RpmRatio;
	
	//to calculate load
    float LastRPM;
	UPROPERTY(BlueprintReadOnly)
    	float TurboRatio;
	UPROPERTY(EditAnywhere,Category=Audio)
	float TurboLerpSpeed;
	UPROPERTY(BlueprintReadOnly)
	float Load;
	
	
	UPROPERTY(EditAnywhere,Category=Audio)
	float AudioInterpSpeed;
	UPROPERTY(EditAnywhere,Category=Audio)
	float LoadInterpSpeed;
	
	

/*MS boost from effectHandler*/
float MSBoost;

};


//////////////////////////////////////////////////////////////////////////
// Some helper functions for converting units

// rad/s to rev per minute
inline float PrvOmegaToRPM(float Omega)
{
	return Omega * 30.f / PI;
}
