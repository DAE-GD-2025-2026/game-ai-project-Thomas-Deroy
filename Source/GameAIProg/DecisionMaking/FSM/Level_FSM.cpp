// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_FSM.h"
#include "States/GuardStates.h"
#include "FSMComponent.h"
#include "DecisionMaking/GameAIController.h"
#include "BehaviorTree/BlackboardComponent.h"


// Sets default values
ALevel_FSM::ALevel_FSM()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_FSM::BeginPlay()
{
	Super::BeginPlay();
	
    // 1. Spawn the Thief (Player)
    Thief = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{-500, 0, 90}, FRotator::ZeroRotator);
    Thief->SetDebugRenderingEnabled(false);
    Thief->SetSteeringBehavior(&ThiefPathFollow); // Assign Thief's legs

    // 2. Spawn the Guard (AI)
    Guard = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{0, 0, 90}, FRotator::ZeroRotator);
    Guard->SetDebugRenderingEnabled(false);
    Guard->SetSteeringBehavior(&GuardPathFollow); // Assign Guard's legs
    
    Guard->SpawnDefaultController();

    // 3. Set up the FSM
    if (AGameAIController* AIController = Cast<AGameAIController>(Guard->GetController()))
    {
        if (UFSMComponent* FSM = Cast<UFSMComponent>(AIController->GetBrainComponent()))
        {
            // Initialize Blackboard with the Player target
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                Blackboard->SetValueAsObject(FName("PlayerActor"), Thief);
            }

            // Create the states
            auto Patrol = std::make_unique<GameAI::FSM::PatrolState>();
            auto Chase = std::make_unique<GameAI::FSM::ChaseState>();
            auto Search = std::make_unique<GameAI::FSM::SearchState>();

            // Grab raw pointers so we can link them in the transitions before moving them
            auto* PatrolPtr = Patrol.get();
            auto* ChasePtr = Chase.get();
            auto* SearchPtr = Search.get();

            // --- TRANSITIONS ---
            float DetectionRadius = 400.0f;
            float MaxSearchTime = 3.0f;

            // Patrol -> Chase (IsTargetVisible)
            FSM->AddTransition(PatrolPtr, ChasePtr, [this, DetectionRadius]() {
                return FVector::Dist(Guard->GetActorLocation(), Thief->GetActorLocation()) < DetectionRadius;
            });

            // Chase -> Search (!IsTargetVisible)
            FSM->AddTransition(ChasePtr, SearchPtr, [this, DetectionRadius]() {
                return FVector::Dist(Guard->GetActorLocation(), Thief->GetActorLocation()) >= DetectionRadius;
            });

            // Search -> Chase (IsTargetVisible)
            FSM->AddTransition(SearchPtr, ChasePtr, [this, DetectionRadius]() {
                return FVector::Dist(Guard->GetActorLocation(), Thief->GetActorLocation()) < DetectionRadius;
            });

            // Search -> Patrol (IsSearchingTooLong)
            FSM->AddTransition(SearchPtr, PatrolPtr, [AIController, MaxSearchTime]() {
                return AIController->GetBlackboardComponent()->GetValueAsFloat(FName("SearchTimer")) > MaxSearchTime;
            });

            // --- ADD STATES TO FSM ---
            // (Patrol is added first, making it the default starting state)
            FSM->AddState(std::move(Patrol));
            FSM->AddState(std::move(Chase));
            FSM->AddState(std::move(Search));

            // 4. Start the Brain!
            AIController->RunFiniteStateMachine();
        }
    }
}

// Called every frame
// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Guard)
    {
        if (AGameAIController* AIController = Cast<AGameAIController>(Guard->GetController()))
        {
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                FVector TargetLoc = Blackboard->GetValueAsVector(FName("TargetLocation"));
                
                // We use a static variable to remember the last place we told the Guard to go
                static FVector PreviousTarget = FVector::ZeroVector;
                
                // ONLY calculate a new path if the target location has actually changed!
                // (Resetting it every single frame causes the agent to freeze)
                if (FVector::Dist(PreviousTarget, TargetLoc) > 10.0f)
                {
                    PreviousTarget = TargetLoc;
                    
                    // Give the PathFollow behavior a proper starting point and ending point
                    std::vector<FVector2D> Path;
                    Path.push_back(Guard->GetPosition());
                    Path.push_back(FVector2D(TargetLoc.X, TargetLoc.Y));
                    
                    GuardPathFollow.SetPath(Path);
                }
            }
        }
    }
}

void ALevel_FSM::BindLevelInputActions()
{
    Super::BindLevelInputActions();

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (SetTargetAction)
        {
            EnhancedInputComponent->BindAction(SetTargetAction, ETriggerEvent::Triggered, this, &ALevel_FSM::SetTarget);
        }
    }
}

void ALevel_FSM::SetTarget()
{
    if (std::optional<FVector> MousePosOpt = GetMouseWorldPos())
    {
        FVector WorldPos = MousePosOpt.value();
        FVector2D TargetPos2D = FVector2D(WorldPos.X, WorldPos.Y);

        // Restore the 2-point path! The behavior needs a line to follow.
        std::vector<FVector2D> Path = { Thief->GetPosition(), TargetPos2D };
        ThiefPathFollow.SetPath(Path);
    }
}

