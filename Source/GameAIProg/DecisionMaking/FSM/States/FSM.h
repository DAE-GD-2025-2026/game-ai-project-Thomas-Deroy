#pragma once
#include <vector>
#include <memory>
#include <functional>

// Forward declarations for Unreal classes
class UBlackboardComponent;
class AGameAIController;

namespace GameAI::FSM
{
    class State; // Forward declaration so Transition knows it exists

    // 1. TRANSITION
    class Transition
    {
    public:
       Transition(State* targetState, std::function<bool()> condition) 
          : TargetState(targetState), Condition(std::move(condition)) {}

       bool Evaluate() const { return Condition(); }
       State* GetTargetState() const { return TargetState; }

    private:
       State* TargetState;
       std::function<bool()> Condition;
    };

    // 2. STATE
    class State
    {
    public:
       virtual ~State() = default;
        
       virtual void OnEnter(UBlackboardComponent* Blackboard, AGameAIController* Controller) {}
       virtual void Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller) = 0;
       virtual void OnExit(UBlackboardComponent* Blackboard, AGameAIController* Controller) {}
       
       void AddTransition(std::unique_ptr<Transition> transition) { Transitions.push_back(std::move(transition)); }
       const std::vector<std::unique_ptr<Transition>>& GetTransitions() const { return Transitions; }

    private:
       std::vector<std::unique_ptr<Transition>> Transitions;
    };

    // 3. FSM CORE
    class FSM
    {
    public:
       void AddState(std::unique_ptr<State> state)
       {
          if (!CurrentState) CurrentState = state.get(); // Set first state as default
          States.push_back(std::move(state));
       }

       void Start(UBlackboardComponent* Blackboard, AGameAIController* Controller)
       {
          if (CurrentState) CurrentState->OnEnter(Blackboard, Controller);
       }

       void Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller)
       {
          if (!CurrentState) return;

          // 1. Check all transitions
          for (const auto& transition : CurrentState->GetTransitions())
          {
             if (transition->Evaluate())
             {
                ChangeState(transition->GetTargetState(), Blackboard, Controller);
                return; // State changed, stop updating the old state
             }
          }

          // 2. If no transition fired, update the current state
          CurrentState->Update(DeltaTime, Blackboard, Controller);
       }

    private:
       void ChangeState(State* newState, UBlackboardComponent* Blackboard, AGameAIController* Controller)
       {
          if (CurrentState) CurrentState->OnExit(Blackboard, Controller);
          CurrentState = newState;
          if (CurrentState) CurrentState->OnEnter(Blackboard, Controller);
       }

       std::vector<std::unique_ptr<State>> States;
       State* CurrentState = nullptr;
    };
}