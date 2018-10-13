#pragma once
#include "stdafx.h"
#include "Blackboard.h"
#include "BehaviorTree.h"
using namespace Elite;

BehaviorState SetTargeterGoal(Blackboard* pBlackboard)
{
	//Get data
	FixedGoalTargeter* pTargeter = nullptr;
	Vector2 newTarget = ZeroVector2;
	IExamInterface* pInterface = nullptr;
	bool foundData = pBlackboard->GetData("Targeter", pTargeter) &&
					 pBlackboard->GetData("MoveTarget", newTarget) &&
					 pBlackboard->GetData("Interface", pInterface);

	if (!foundData || !pTargeter || !pInterface)
		return Failure;

	Vector2 nextPos = pInterface->NavMesh_GetClosestPathPoint(newTarget);
	pTargeter->GetGoalRef().Position = nextPos;
	pTargeter->GetGoalRef().PositionSet = true;

	return Success;
}

BehaviorState CalculateSteering(Blackboard* pBlackboard)
{
	SteeringPipeline* pPipeline = nullptr;
	float deltaTime = 0.0f;
	AgentInfo agent = {};
	bool isAiming = false;
	Face* pFaceBehavior = nullptr;

	bool foundData = pBlackboard->GetData("Pipeline", pPipeline) &&
					 pBlackboard->GetData("DeltaTime", deltaTime) &&
					 pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("IsAiming", isAiming) &&
					 pBlackboard->GetData("FaceBehavior", pFaceBehavior);

	if (!foundData || !pPipeline || !pFaceBehavior)
		return Failure;

	auto agentSteering = pPipeline->CalculateSteering(deltaTime, &agent);
	if (isAiming) {
		auto aimSteering = pFaceBehavior->CalculateSteering(deltaTime, &agent);
		agentSteering.AngularVelocity = aimSteering.AngularVelocity;
		agentSteering.AutoOrientate = false;
	}

	pBlackboard->ChangeData("Steering", agentSteering);
	return Success;
}