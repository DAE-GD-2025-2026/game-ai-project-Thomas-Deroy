#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	if (pFlock->GetNrOfNeighbors() == 0) return SteeringOutput{};
    
	FVector2D targetPos = pFlock->GetAverageNeighborPos();
	this->SetTarget(FSteeringParams{ targetPos });
    
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)

SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	pFlock->RegisterNeighbors(&pAgent);

	SteeringOutput steering = {};
	if (pFlock->GetNrOfNeighbors() == 0) return steering;

	FVector2D agentPos = pAgent.GetPosition();
	for (int i = 0; i < pFlock->GetNrOfNeighbors(); ++i)
	{
		FVector2D neighborPos = pFlock->GetNeighbors()[i]->GetPosition();
		FVector2D toAgent = agentPos - neighborPos;
		float distance = toAgent.Size();
		
		if (distance > 0.001f)
		{
			steering.LinearVelocity += (toAgent / distance) / distance * pAgent.GetMaxLinearSpeed();
		}
	}
	steering.IsValid = true;
	return steering;
}
//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};
	if (pFlock->GetNrOfNeighbors() == 0) return steering;

	const FVector2D averageNeighborVelocity = pFlock->GetAverageNeighborVelocity();
	steering.LinearVelocity = averageNeighborVelocity;
	steering.IsValid = true;
	return steering;
}
