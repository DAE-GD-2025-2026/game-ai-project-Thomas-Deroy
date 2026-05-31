// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Level_Base.h"
#include "Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"

#include "Level_FSM.generated.h"


UCLASS()
class GAMEAIPROG_API ALevel_FSM : public ALevel_Base
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevel_FSM();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class UInputAction* SetTargetAction{};
    
	virtual void BindLevelInputActions() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void SetTarget();
	
	UPROPERTY()
	ASteeringAgent* Guard{nullptr}; 

	UPROPERTY()
	ASteeringAgent* Thief{nullptr};
	
	PathFollow ThiefPathFollow{}; 
	PathFollow GuardPathFollow{};

private:
	UPROPERTY()
	ASteeringAgent* Agent{nullptr}; // ref
};
