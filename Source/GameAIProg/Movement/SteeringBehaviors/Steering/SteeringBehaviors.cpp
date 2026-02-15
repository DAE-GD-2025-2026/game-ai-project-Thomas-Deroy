#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
#include "DrawDebugHelpers.h" 

// SEEK
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    steering.LinearVelocity = Target.Position - Agent.GetPosition(); // Move directly toward target

    // Debug rendering
    if (Agent.GetDebugRenderingEnabled())
        DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Target.Position, 0), FColor::Green, false, -1, 0, 2.f);
    
    return steering;
}

// --------------------------------------------------------------------------------------------------------

// FLEE
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    steering.LinearVelocity = Agent.GetPosition() - Target.Position; // Move directly away from target

    // Debug rendering
    if (Agent.GetDebugRenderingEnabled())
        DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Agent.GetPosition() + steering.LinearVelocity, 0), FColor::Red, false, -1, 0, 2.f);

    return steering;
}

// --------------------------------------------------------------------------------------------------------

Arrive::~Arrive()
{
    if (BoundAgent && OriginalMaxSpeed > 0.f)
    {
        BoundAgent->SetMaxLinearSpeed(OriginalMaxSpeed); // Restore original speed on destroy
    }
}

// ARRIVE
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    
    if (OriginalMaxSpeed < 0.f) // Cache original speed once
    {
        OriginalMaxSpeed = Agent.GetMaxLinearSpeed();
        BoundAgent = &Agent;
    }
    
    FVector2D ToTarget = Target.Position - Agent.GetPosition();
    float Distance = ToTarget.Size();
    
    if (Distance > SlowRadius)
    {
        Agent.SetMaxLinearSpeed(OriginalMaxSpeed); // Full speed
    }
    else if (Distance < TargetRadius)
    {
        Agent.SetMaxLinearSpeed(0.f); // Stop at target
    }
    else
    {
        // Scale speed smoothly between SlowRadius and TargetRadius
        float NewSpeed = OriginalMaxSpeed * (Distance - TargetRadius) / (SlowRadius - TargetRadius);
        Agent.SetMaxLinearSpeed(NewSpeed);
    }
    
    steering.LinearVelocity = ToTarget; // Always steer toward target
    
    // Debug rendering
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

// --------------------------------------------------------------------------------------------------------

// FACE
SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    
    FVector2D ToTarget = Target.Position - Agent.GetPosition();
    
    float TargetAngle = UKismetMathLibrary::Atan2(ToTarget.Y, ToTarget.X) * (180.f / PI);
    
    float CurrentAngle = Agent.GetRotation();
    
    float AngleDiff = TargetAngle - CurrentAngle;

    // Normalize angle to [-180, 180]
    while (AngleDiff > 180.f) AngleDiff -= 360.f;
    while (AngleDiff < -180.f) AngleDiff += 360.f;
    
    steering.AngularVelocity = AngleDiff; // Rotate toward the shortest angle
    
    // Debug rendering
    if (Agent.GetDebugRenderingEnabled())
    {
        DrawDebugLine(Agent.GetWorld(), 
            FVector(Agent.GetPosition(), 0), 
            FVector(Agent.GetPosition() + ToTarget.GetSafeNormal() * 100.f, 0), 
            FColor::Magenta, false, -1, 0, 2.f);
    }

    return steering;
}

// --------------------------------------------------------------------------------------------------------

// PURSUIT
SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    FVector2D CurrentPos = Agent.GetPosition();
    FVector2D TargetPos = Target.Position;
    
    FVector2D Direction = TargetPos - CurrentPos;
    float Distance = Direction.Size();
    
    float Speed = Agent.GetLinearVelocity().Size();
    float PredictionTime;
    
    // Predict further if slow relative to distance
    if (Speed <= Distance / MaxPredictionTime)
    {
        PredictionTime = MaxPredictionTime;
    }
    else
    {
        PredictionTime = Distance / Speed;
    }
    
    FVector2D PredictedPosition = TargetPos + (Target.LinearVelocity * PredictionTime);
    
    steering.LinearVelocity = PredictedPosition - CurrentPos; // Seek predicted future position
    
    // Debug rendering
    if (Agent.GetDebugRenderingEnabled())
    {
        UWorld* World = Agent.GetWorld();
        DrawDebugSphere(World, FVector(PredictedPosition, 0), 25.f, 8, FColor::Cyan, false, -1, 0, 1.f);
        DrawDebugLine(World, FVector(TargetPos, 0), FVector(PredictedPosition, 0), FColor::White, false, -1, 0, 1.f);
    }

    return steering;
}

// --------------------------------------------------------------------------------------------------------

// EVADE
SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput steering{};
    FVector2D CurrentPos = Agent.GetPosition();
    FVector2D TargetPos = Target.Position;
    
    FVector2D Direction = TargetPos - CurrentPos;
    float Distance = Direction.Size();
    
    float Speed = Agent.GetLinearVelocity().Size();
    float PredictionTime;
    
    // Same prediction logic as Pursuit
    if (Speed <= Distance / MaxPredictionTime)
    {
        PredictionTime = MaxPredictionTime;
    }
    else
    {
        PredictionTime = Distance / Speed;
    }
    
    FVector2D PredictedPosition = TargetPos + (Target.LinearVelocity * PredictionTime);
    
    steering.LinearVelocity = CurrentPos - PredictedPosition; // Flee predicted position

    // Debug rendering
    if (Agent.GetDebugRenderingEnabled())
    {
        UWorld* World = Agent.GetWorld();
        DrawDebugSphere(World, FVector(PredictedPosition, 0), 25.f, 8, FColor::Red, false, -1, 0, 1.f);
        DrawDebugLine(World, FVector(TargetPos, 0), FVector(PredictedPosition, 0), FColor::White, false, -1, 0, 1.f);
    }

    return steering;
}

// --------------------------------------------------------------------------------------------------------

// WANDER
SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    float Jitter = (static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f) * MaxAngleChange; // Small random angle change
    WanderAngle += Jitter;
    
    float AgentRotationDeg = Agent.GetRotation(); 
    FVector2D ForwardVector;
    ForwardVector.X = UKismetMathLibrary::Cos(AgentRotationDeg * (PI / 180.f));
    ForwardVector.Y = UKismetMathLibrary::Sin(AgentRotationDeg * (PI / 180.f));
    
    FVector2D CircleCenter = Agent.GetPosition() + (ForwardVector * Offset); // Project circle in front of agent
    
    float TotalAngle = AgentRotationDeg + WanderAngle;
    
    FVector2D TargetOffset;
    TargetOffset.X = Radius * UKismetMathLibrary::Cos(TotalAngle * (PI / 180.f));
    TargetOffset.Y = Radius * UKismetMathLibrary::Sin(TotalAngle * (PI / 180.f));
    
    FVector2D WanderTarget = CircleCenter + TargetOffset; // Final wander target
    
    FTargetData TempTarget;
    TempTarget.Position = WanderTarget;
    this->SetTarget(TempTarget);

    // Debug Rendering
    if (Agent.GetDebugRenderingEnabled())
    {
        UWorld* World = Agent.GetWorld();
        FVector Center3D = FVector(CircleCenter, 0.f);
        DrawDebugCircle(World, Center3D, Radius, 32, FColor::Cyan, false, -1, 0, 1.f, FVector(0,1,0), FVector(1,0,0));
        DrawDebugSphere(World, FVector(WanderTarget, 0.f), 15.f, 8, FColor::Green, false, -1, 0, 1.f);
    }

    return Seek::CalculateSteering(DeltaT, Agent); // Reuse Seek toward wander target
}
