#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
#include "DrawDebugHelpers.h" 
#include "imgui.h"           

// SEEK
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    steering.LinearVelocity = Target.Position - Agent.GetPosition();

    // Line to target
    if (Agent.GetDebugRenderingEnabled())
        DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Target.Position, 0), FColor::Green, false, -1, 0, 2.f);
    
    return steering;
}

// FLEE
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    steering.LinearVelocity = Agent.GetPosition() - Target.Position; 

    // Line away from target
    if (Agent.GetDebugRenderingEnabled())
        DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Agent.GetPosition() + steering.LinearVelocity, 0), FColor::Red, false, -1, 0, 2.f);

    return steering;
}

// ARRIVE
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    
    if (OriginalMaxSpeed < 0.f)
    {
        OriginalMaxSpeed = Agent.GetMaxLinearSpeed();
    }
    
    FVector2D ToTarget = Target.Position - Agent.GetPosition();
    float Distance = ToTarget.Size();
    
    if (Distance > SlowRadius)
    {
        Agent.SetMaxLinearSpeed(OriginalMaxSpeed);
    }
    else if (Distance < TargetRadius)
    {
        Agent.SetMaxLinearSpeed(0.f);
    }
    else
    {
        float NewSpeed = OriginalMaxSpeed * (Distance - TargetRadius) / (SlowRadius - TargetRadius);
        Agent.SetMaxLinearSpeed(NewSpeed);
    }
    
    steering.LinearVelocity = ToTarget;
    
    if (Agent.GetDebugRenderingEnabled())
    {
        UWorld* World = Agent.GetWorld();
        FVector Center = FVector(Target.Position, 0.f);
        
        DrawDebugCircle(World, Center, SlowRadius, 32, FColor::Yellow, false, -1, 0, 2.f, FVector(0,1,0), FVector(1,0,0));
        DrawDebugCircle(World, Center, TargetRadius, 32, FColor::Red, false, -1, 0, 2.f, FVector(0,1,0), FVector(1,0,0));
        
        DrawDebugLine(World, FVector(Agent.GetPosition(), 0), Center, FColor::Cyan, false, -1, 0, 2.f);
    }

    return steering;
}