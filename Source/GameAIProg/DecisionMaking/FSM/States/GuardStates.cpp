#include "GuardStates.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DecisionMaking/GameAIController.h"
#include "Kismet/GameplayStatics.h"

const FName BBKey_TargetLocation = FName("TargetLocation");
const FName BBKey_LastKnownLocation = FName("LastKnownLocation");
const FName BBKey_PatrolIndex = FName("PatrolIndex");
const FName BBKey_SearchTimer = FName("SearchTimer");
const FName BBKey_PlayerActor = FName("PlayerActor");

namespace GameAI::FSM
{
    // ========================================================================
    // PATROL STATE LOGIC
    // ========================================================================
    void PatrolState::OnEnter(UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard is now PATROLLING."));
        
        // Ensure we have a valid patrol index starting at 0
        if (!Blackboard->IsVectorValueSet(BBKey_TargetLocation))
        {
            Blackboard->SetValueAsInt(BBKey_PatrolIndex, 0);
        }
    }

    void PatrolState::Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        // 1. In a real game, you'd fetch an array of patrol points from the Controller or Level.
        // For this assignment, we'll simulate 3 hardcoded points to patrol between.
        static const TArray<FVector> PatrolPoints = {
            FVector(0, 0, 0),
            FVector(500, 0, 0),
            FVector(500, 500, 0)
        };

        int CurrentIndex = Blackboard->GetValueAsInt(BBKey_PatrolIndex);
        FVector CurrentTarget = PatrolPoints[CurrentIndex];

        // 2. Tell the agent/controller to move to the target location
        Blackboard->SetValueAsVector(BBKey_TargetLocation, CurrentTarget);
        
        // (Assuming your controller reads BBKey_TargetLocation and passes it to your SteeringAgent)
        
        // 3. Check if we reached the waypoint
        FVector AgentLocation = Controller->GetPawn()->GetActorLocation();
        float DistanceToTarget = FVector::Dist(AgentLocation, CurrentTarget);

        if (DistanceToTarget < 100.0f) // If we are within 100 units
        {
            // Loop back to the start if we hit the end of the patrol route[cite: 3]
            CurrentIndex++;
            if (CurrentIndex >= PatrolPoints.Num())
            {
                CurrentIndex = 0;
            }
            Blackboard->SetValueAsInt(BBKey_PatrolIndex, CurrentIndex);
        }
    }

    void PatrolState::OnExit(UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        // Leaving patrol (likely because we saw the player!)
    }

    // ========================================================================
    // CHASE STATE LOGIC
    // ========================================================================
    void ChaseState::OnEnter(UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard spotted player! CHASING."));
    }

    void ChaseState::Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        // 1. Get the player object from the Blackboard
        if (AActor* Player = Cast<AActor>(Blackboard->GetValueAsObject(BBKey_PlayerActor)))
        {
            // 2. Set our Target Location directly to the player's current location[cite: 3]
            FVector PlayerLocation = Player->GetActorLocation();
            Blackboard->SetValueAsVector(BBKey_TargetLocation, PlayerLocation);
        }
    }

    void ChaseState::OnExit(UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard lost sight of player."));
        
        // Save the exact spot we last saw the player so the Search state can investigate it!
        FVector LastKnown = Blackboard->GetValueAsVector(BBKey_TargetLocation);
        Blackboard->SetValueAsVector(BBKey_LastKnownLocation, LastKnown);
    }

    // ========================================================================
    // SEARCH STATE LOGIC
    // ========================================================================
    void SearchState::OnEnter(UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard is SEARCHING."));
        
        // Reset our search timer
        Blackboard->SetValueAsFloat(BBKey_SearchTimer, 0.0f);

        // Tell the agent to immediately head to where we last saw the player[cite: 3]
        FVector LastKnown = Blackboard->GetValueAsVector(BBKey_LastKnownLocation);
        Blackboard->SetValueAsVector(BBKey_TargetLocation, LastKnown);
    }

    void SearchState::Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        // 1. Tick the search timer up
        float CurrentTimer = Blackboard->GetValueAsFloat(BBKey_SearchTimer);
        CurrentTimer += DeltaTime;
        Blackboard->SetValueAsFloat(BBKey_SearchTimer, CurrentTimer);

        // 2. Logic: Move to Last Known Location, then Wander[cite: 3]
        FVector AgentLocation = Controller->GetPawn()->GetActorLocation();
        FVector TargetLocation = Blackboard->GetValueAsVector(BBKey_TargetLocation);
        
        if (FVector::Dist(AgentLocation, TargetLocation) < 100.0f)
        {
            // We reached the last known location. 
            // In a full implementation, you would generate a random "Wander" point around here
            // and set it as the new TargetLocation.
        }
        
        // (Note: The actual transition back to Patrol is handled by your Transition conditions, 
        // which will check if `CurrentTimer > MaxSearchTime`).
    }

    void SearchState::OnExit(UBlackboardComponent* Blackboard, AGameAIController* Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard is done searching."));
    }
}