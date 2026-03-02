#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
// COHESION
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	// No neighbors -> no cohesion
	if (pFlock->GetNrOfNeighbors() == 0) return SteeringOutput{};
    
	// Move toward average neighbor position
	FVector2D targetPos = pFlock->GetAverageNeighborPos();
	this->SetTarget(FSteeringParams{ targetPos });
    
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*********************
// SEPARATION
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};

	// No neighbors -> no separation
	if (pFlock->GetNrOfNeighbors() == 0) return steering;

	FVector2D agentPos = pAgent.GetPosition();

	// Push away from each neighbor
	for (int i = 0; i < pFlock->GetNrOfNeighbors(); ++i)
	{
		FVector2D neighborPos = pFlock->GetNeighbors()[i]->GetPosition();
		FVector2D toAgent = agentPos - neighborPos;
		float distance = toAgent.Size();
		
		if (distance > 0.001f)
		{
			// Stronger push when closer
			steering.LinearVelocity += (toAgent / distance) / distance * pAgent.GetMaxLinearSpeed();
		}
	}

	steering.IsValid = true;
	return steering;
}

//*************************
// VELOCITY MATCH (ALIGNMENT)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};

	// No neighbors -> no alignment
	if (pFlock->GetNrOfNeighbors() == 0) return steering;

	// Match average neighbor velocity
	const FVector2D averageNeighborVelocity = pFlock->GetAverageNeighborVelocity();
	steering.LinearVelocity = averageNeighborVelocity;
	steering.IsValid = true;

	return steering;
}