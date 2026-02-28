#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
, FlockSize{ FlockSize }
, pAgentToEvade{pAgentToEvade}
{
	// Allocate space
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	Neighbors.SetNum(FlockSize); 
#endif
	Agents.SetNum(FlockSize);
	
	// Create behaviors
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();
	
	// Combine flocking behaviors
	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>{
		{pSeparationBehavior.get(), 1.0f},
		{pCohesionBehavior.get(), 0.2f},
		{pVelMatchBehavior.get(), 0.2f},
		{pWanderBehavior.get(), 0.1f},
		{pSeekBehavior.get(), 0.0f}
	});
	
	// Evade first, otherwise use flocking
	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{
		pEvadeBehavior.get(),
		pBlendedSteering.get()
	});
	
#ifdef GAMEAI_USE_SPACE_PARTITIONING
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, WorldSize, WorldSize, NrOfCellsX, NrOfCellsX, FlockSize);
	
	OldPositions.SetNum(FlockSize);
#endif
	
	
	for (int i = 0; i < FlockSize; ++i)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
		// Random spawn position
		FVector RandomPos = FVector(FMath::RandRange(-500.f, 500.f), FMath::RandRange(-500.f, 500.f), 0.f);
    
		Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, RandomPos, FRotator::ZeroRotator, SpawnParams);
    
		if (Agents[i])
		{
			Agents[i]->SetSteeringBehavior(pPrioritySteering.get());
			Agents[i]->SetDebugRenderingEnabled(false); // I wanna see the screen
			
#ifdef GAMEAI_USE_SPACE_PARTITIONING
			pPartitionedSpace->AddAgent(*Agents[i]);
			OldPositions[i] = Agents[i]->GetPosition();
#endif
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Flock: Failed to spawn agent!"));
		}
	}
}

Flock::~Flock()
{
	for (ASteeringAgent* pAgent : Agents)
	{
		if (pAgent && pAgent->IsValidLowLevel())
		{
			pAgent->Destroy();
		}
	}
	
	Agents.Empty();
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	Neighbors.Empty();
#endif
}

void Flock::Tick(float DeltaTime)
{
	// Invisible circle in the middle bullshit, but fixed it kinda
	if (pAgentToEvade && pAgentToEvade->IsValidLowLevel())
	{
		pEvadeBehavior->SetTarget(FSteeringParams{ pAgentToEvade->GetPosition() });
	}
	else if (pEvadeBehavior)
	{
		pEvadeBehavior->SetTarget(FSteeringParams{ FVector2D(99999.f, 99999.f) }); 
	}
	
	for (int i = 0; i < Agents.Num(); ++i)
	{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		pPartitionedSpace->UpdateAgentCell(*Agents[i], OldPositions[i]);
		OldPositions[i] = Agents[i]->GetPosition();
        
		pPartitionedSpace->RegisterNeighbors(*Agents[i], NeighborhoodRadius); 
#else
		RegisterNeighbors(Agents[i]);
#endif
	}
}

void Flock::RenderDebug()
{
	RenderNeighborhood();
	
#ifdef GAMEAI_USE_SPACE_PARTITIONING
	if (DebugRenderPartitions && pPartitionedSpace)
	{
		pPartitionedSpace->RenderCells();
	}
#endif
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();
		
		if (ImGui::Checkbox("Debug Render Steering", &DebugRenderSteering))
		{
			for (ASteeringAgent* pAgent : Agents)
			{
				if (pAgent) pAgent->SetDebugRenderingEnabled(DebugRenderSteering);
			}
		}
		ImGui::Checkbox("Debug Render Neighborhood", &DebugRenderNeighborhood);
		ImGui::Checkbox("Debug Render Partitions", &DebugRenderPartitions);

		ImGui::Spacing();
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();
		
		if (pBlendedSteering)
		{
			auto& weightedBehaviors = pBlendedSteering->GetWeightedBehaviorsRef();
			
			if (weightedBehaviors.size() >= 5)
			{
				// Separation
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation", 
					weightedBehaviors[0].Weight, 0.f, 1.f, 
					[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");

				// Cohesion
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion", 
					weightedBehaviors[1].Weight, 0.f, 1.f, 
					[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");

				// Alignment (Velocity Match)
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Alignment", 
					weightedBehaviors[2].Weight, 0.f, 1.f, 
					[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = InVal; }, "%.2f");

				// Wander
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander", 
					weightedBehaviors[3].Weight, 0.f, 1.f, 
					[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = InVal; }, "%.2f");

				// Seek
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek", 
					weightedBehaviors[4].Weight, 0.f, 1.f, 
					[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = InVal; }, "%.2f");
			}
		}
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
	if (!DebugRenderNeighborhood || Agents.Num() == 0) return;
    
	// just the first agent because I still wanna see
	ASteeringAgent* currentAgent = Agents[0];
	Agents[0]->SetDebugRenderingEnabled(true);
	
	FVector center3D = FVector(currentAgent->GetPosition(), 0.f);
	
	DrawDebugCircle(pWorld, center3D, NeighborhoodRadius, 32, FColor::Green, false, -1.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0), false);
	
	for (ASteeringAgent* other : Agents)
	{
		if (currentAgent == other) continue;

		float distSq = FVector2D::DistSquared(currentAgent->GetPosition(), other->GetPosition());
		if (distSq < (NeighborhoodRadius * NeighborhoodRadius))
		{
			DrawDebugLine(pWorld, center3D, FVector(other->GetPosition(), 0.f), FColor::Green, false, -1.f, 0, 1.5f);
		}
	}
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0; 
	FVector2D agentPos = pAgent->GetPosition();

	for (ASteeringAgent* pOtherAgent : Agents)
	{
		if (pAgent == pOtherAgent) continue;
		
		float distSq = FVector2D::DistSquared(agentPos, pOtherAgent->GetPosition());
		if (distSq < (NeighborhoodRadius * NeighborhoodRadius))
		{
			Neighbors[NrOfNeighbors] = pOtherAgent;
			NrOfNeighbors++;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	const auto& neighbors = GetNeighbors(); 
	int nr = GetNrOfNeighbors();            

	if (nr == 0) return FVector2D::ZeroVector; 
    
	FVector2D avgPosition = FVector2D::ZeroVector;
	for (int i = 0; i < nr; ++i) 
	{
		avgPosition += neighbors[i]->GetPosition(); 
	}
    
	return avgPosition / static_cast<float>(nr); 
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	const auto& neighbors = GetNeighbors();
	int nr = GetNrOfNeighbors();

	if (nr == 0) return FVector2D::ZeroVector; 

	FVector2D avgVelocity = FVector2D::ZeroVector;
	for (int i = 0; i < nr; ++i) 
	{
		avgVelocity += neighbors[i]->GetLinearVelocity();
	}

	return avgVelocity / static_cast<float>(nr);
}


void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	if (pSeekBehavior)
	{
		pSeekBehavior->SetTarget(Target);
	}
}

