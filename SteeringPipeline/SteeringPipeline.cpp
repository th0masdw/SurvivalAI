#include "stdafx.h"
#include "SteeringPipeline.h"
#include "IExamInterface.h"

float Path::GetMaxPriority()
{
	if (!m_pAgent)
		return 0.f;

	return (m_pAgent->Position - m_Goal.Position).Magnitude();
}

SteeringPlugin_Output SteeringPipeline::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	Goal currGoal = {}; //Goal used by the pipeline

	//1. TARGETER
	//Try to merge the targeter's goals
	for (Targeter* pTargeter : m_Targeters) {
		Goal targeterGoal = pTargeter->GetGoal();

		if (currGoal.CanMergeGoal(targeterGoal)) {
			currGoal.UpdateGoal(targeterGoal);
		}
	}

	//2. DECOMPOSER
	//Decompose the current goal
	for (Decomposer* pDecomposer : m_Decomposers) {
		currGoal = pDecomposer->DecomposeGoal(currGoal, deltaT);
	}

	//3. PATH & CONSTRAINTS
	//Check for constraint violations
	if (!m_pPath) m_pPath = m_pActuator->CreatePath();
	float shortestViolation, currentViolation, maxViolation;
	Constraint* pViolatingConstraint = nullptr;

	for (UINT i = 0; i < m_MaxConstraintSteps; ++i) {
		//Get path to goal
		m_pActuator->UpdatePath(m_pPath, pAgent, currGoal);

		//If no constraints => calculate steering using the actuator
		if (m_Constraints.size() == 0) {
			return m_pActuator->CalculateSteering(m_pPath, deltaT, pAgent);
		}

		//Find violating constraint
		shortestViolation = maxViolation = m_pPath->GetMaxPriority();
		for (Constraint* pConstraint : m_Constraints) {
			if (i == 0)
				pConstraint->SetSuggestionUsed(false);

			//Check if this constraint violation is more urgent than the last
			currentViolation = pConstraint->WillViolate(m_pPath, pAgent, shortestViolation);
			if (currentViolation < shortestViolation && currentViolation > 0.0f) {
				shortestViolation = currentViolation;
				pViolatingConstraint = pConstraint;
			}
		}

		//Check most violating constraint
		if (shortestViolation < maxViolation) {
			currGoal = pViolatingConstraint->Suggest(m_pPath);
			pViolatingConstraint->SetSuggestionUsed(true);
		} else {
			if (m_pContext) {
				m_pContext->Draw_SolidCircle(currGoal.Position, 0.5f, { 0.0f, 0.0f }, { 1, 1, 0 });
				m_pContext->Draw_Segment(pAgent->Position, currGoal.Position, { 1, 1, 0 });
			}

			//Found a goal
			return m_pActuator->CalculateSteering(m_pPath, deltaT, pAgent);
		}
	}

	//ConstraintStep reached (no solution found, use fallback behaviour for now)
	if (m_pFallbackBehaviour)
		return m_pFallbackBehaviour->CalculateSteering(deltaT, pAgent);

	return SteeringPlugin_Output(); //Empty steeringoutput (Ultra-Fallback...)
}