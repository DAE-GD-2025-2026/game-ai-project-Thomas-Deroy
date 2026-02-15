#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// --------------------------------------------------------------------------------------------------------

// SteeringBehaviors 
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	// steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// --------------------------------------------------------------------------------------------------------

class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() override = default;

	// steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// --------------------------------------------------------------------------------------------------------

class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() override;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	
	float SlowRadius = 600.f;
	float TargetRadius = 100.f;
	float OriginalMaxSpeed = -1.f;
	ASteeringAgent* BoundAgent = nullptr;
};

// --------------------------------------------------------------------------------------------------------

class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// --------------------------------------------------------------------------------------------------------

class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

	float MaxPredictionTime = 2.0f; 
};

// --------------------------------------------------------------------------------------------------------

class Evade : public ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

	float MaxPredictionTime = 2.0f; 
};

// --------------------------------------------------------------------------------------------------------

class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	
	void SetWanderOffset(float offset){ Offset = offset; }
	void SetWanderRadius(float radius){ Radius = radius; }
	void SetMaxAngleChange(float angle){ MaxAngleChange = angle; }

protected:
	float Offset = 400.f;          
	float Radius = 200.f;          
	float MaxAngleChange = 45.f;   
	float WanderAngle = 0.f;       
};
