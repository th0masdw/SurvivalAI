#pragma once

#include "SteeringBehaviours.h"

#pragma region ***Goal***
struct Goal
{
	//Channels
	Vector2 Position = { 0.f, 0.f };
	bool PositionSet = false;

	//Can be expanded with LinVelocity, AngVelocity & Rotation Channels
	//...

	Goal() {};

	//Reset Goal's Channels
	void Clear()
	{
		Position = ZeroVector2;
		PositionSet = false;
	}

	//Update Goal
	void UpdateGoal(const Goal& goal)
	{
		if (goal.PositionSet)
		{
			Position = goal.Position;
			PositionSet = true;
		}
	}

	//Merge Goal
	bool CanMergeGoal(const Goal& goal) const
	{
		return !(PositionSet && goal.PositionSet);
	}

};
#pragma endregion

#pragma region ***Path***
//SteeringPipeline Internal Path Segment (Path to (sub)goal)
class Path
{
public:
	virtual ~Path() {}
	void SetGoal(Goal goal) { m_Goal = goal; }
	Goal GetGoal()const { return m_Goal; }
	void SetAgent(AgentInfo* pAgent) { m_pAgent = pAgent; }
	virtual float GetMaxPriority();

protected:
	float m_MaxPriority = 50.f;
	Goal m_Goal = {};
	AgentInfo* m_pAgent = nullptr;
};
#pragma endregion

#pragma region ***[PIPELINE-PART] Targeter***
class Targeter
{
public:
	virtual ~Targeter() {}

	//Targeter Functions
	virtual Goal GetGoal() = 0;
};
#pragma endregion

#pragma region ***[PIPELINE-PART] Decomposer***
class Decomposer
{
public:
	virtual ~Decomposer() {}

	//Decomposer Functions
	virtual Goal DecomposeGoal(const Goal& goal, float deltaTime) = 0;
};
#pragma endregion

#pragma region ***[PIPELINE-PART] Constraint***
class Constraint
{
public:
	virtual ~Constraint() {}

	//Constraint Functions
	virtual float WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority) = 0;
	virtual Goal Suggest(const Path* pPath) = 0;
	void SetSuggestionUsed(bool val) { m_SuggestionUsed = val; }

private:
	bool m_SuggestionUsed = false;
};
#pragma endregion

#pragma region ***[PIPELINE-PART] Actuator***
class Actuator
{
public:
	virtual ~Actuator() {}

	//Actuator Functions
	virtual Path* CreatePath() = 0;
	virtual void UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal) = 0;
	virtual SteeringPlugin_Output CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent) = 0;
};
#pragma endregion

#pragma region ***STEERING PIPELING***
class SteeringPipeline : public ISteeringBehaviour
{
public:
	SteeringPipeline(IExamInterface* pContext) : ISteeringBehaviour(pContext){}
	virtual ~SteeringPipeline() {};

	//SteeringPipeline Functions
	void SetActuator(Actuator* pActuator) { m_pActuator = pActuator; SAFE_DELETE(m_pPath); }
	void SetTargeters(vector<Targeter*> targeters) { m_Targeters = targeters; }
	void SetDecomposers(vector<Decomposer*> decomposers) { m_Decomposers = decomposers; }
	void SetConstraints(vector<Constraint*> constraints) { m_Constraints = constraints; }
	void SetFallBack(ISteeringBehaviour* pFallback) { m_pFallbackBehaviour = pFallback; }

	//SteeringPipeline Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

private:

	Path* m_pPath = nullptr;
	Actuator* m_pActuator = nullptr;
	vector<Targeter*> m_Targeters = {};
	vector<Decomposer*> m_Decomposers = {};
	vector<Constraint*> m_Constraints = {};

	UINT m_MaxConstraintSteps = 10;
	ISteeringBehaviour* m_pFallbackBehaviour = nullptr;
};
#pragma endregion