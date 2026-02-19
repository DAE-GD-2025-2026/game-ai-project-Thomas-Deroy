#include "Level_CombinedSteering.h"

#include "imgui.h"
#include <vector>


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	SeekAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{-300.f, 0.f, 90.f}, FRotator::ZeroRotator);
	WandererAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{300.f, 0.f, 90.f}, FRotator::ZeroRotator);
	if (!IsValid(SeekAgent) || !IsValid(WandererAgent))
		return;

	SeekBehavior = MakeUnique<Seek>();
	EvadeBehavior = MakeUnique<Evade>();
	WanderBehavior = MakeUnique<Wander>();

	std::vector<BlendedSteering::WeightedBehavior> weightedBehaviors{
		{SeekBehavior.Get(), 0.7f},
		{EvadeBehavior.Get(), 1.0f}
	};

	BlendedBehavior = MakeUnique<BlendedSteering>(weightedBehaviors);

	SeekAgent->SetSteeringBehavior(BlendedBehavior.Get());
	WandererAgent->SetSteeringBehavior(WanderBehavior.Get());

	SeekAgent->SetDebugRenderingEnabled(CanDebugRender);
	WandererAgent->SetDebugRenderingEnabled(CanDebugRender);
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();

}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
			if (IsValid(SeekAgent))
				SeekAgent->SetDebugRenderingEnabled(CanDebugRender);
			if (IsValid(WandererAgent))
				WandererAgent->SetDebugRenderingEnabled(CanDebugRender);
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		if (BlendedBehavior)
		{
			auto& weightedBehaviors = BlendedBehavior->GetWeightedBehaviorsRef();
			if (weightedBehaviors.size() >= 2)
			{
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
					weightedBehaviors[0].Weight, 0.f, 1.f,
					[this](float InVal) { BlendedBehavior->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Evade",
					weightedBehaviors[1].Weight, 0.f, 1.f,
					[this](float InVal) { BlendedBehavior->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
			}
		}
	
		//End
		ImGui::End();
	}
#pragma endregion
	
	// Combined Steering Update
	if (!IsValid(SeekAgent) || !IsValid(WandererAgent) || !SeekBehavior || !EvadeBehavior)
		return;

	if (UseMouseTarget)
	{
		SeekBehavior->SetTarget(MouseTarget);
	}

	FTargetData WandererTarget;
	WandererTarget.Position = WandererAgent->GetPosition();
	WandererTarget.Orientation = WandererAgent->GetRotation();
	WandererTarget.LinearVelocity = WandererAgent->GetLinearVelocity();
	WandererTarget.AngularVelocity = WandererAgent->GetAngularVelocity();
	EvadeBehavior->SetTarget(WandererTarget);
}
