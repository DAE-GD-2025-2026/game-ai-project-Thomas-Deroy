#pragma once

#include "FSM.h"

class UBlackboardComponent;
class AGameAIController;

namespace GameAI::FSM
{
	// ----------------------------------------------------
	// 1. PATROL STATE
	// ----------------------------------------------------
	class PatrolState : public State
	{
	public:
		virtual void OnEnter(UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
		virtual void Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
		virtual void OnExit(UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
	};

	// ----------------------------------------------------
	// 2. CHASE STATE
	// ----------------------------------------------------
	class ChaseState : public State
	{
	public:
		virtual void OnEnter(UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
		virtual void Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
		virtual void OnExit(UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
	};

	// ----------------------------------------------------
	// 3. SEARCH STATE
	// ----------------------------------------------------
	class SearchState : public State
	{
	public:
		virtual void OnEnter(UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
		virtual void Update(float DeltaTime, UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
		virtual void OnExit(UBlackboardComponent* Blackboard, AGameAIController* Controller) override;
	};
}