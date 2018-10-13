#pragma once

#include "SteeringPipeline.h"

//***********************************************************************************
//1. TARGETER (FixedGoalTargeter)
//Basic Implementation
class FixedGoalTargeter : public Targeter
{
public:
	Goal& GetGoalRef();
	Goal GetGoal() override;

private:
	Goal m_Goal = {};
};

//***********************************************************************************
//2. DECOMPOSER (Path Planner)
//Could be combined with NavMesh Graph
//Skip for now...


//***********************************************************************************
//3. CONSTRAINT
//Simple constraint that checks if the character isn't walking into a SphereObstacle

//Sphere Obstacle
struct SphereObstacle
{
	Vector2 Position;
	float Radius;
};

class AvoidSpheresConstraint : public Constraint
{
public:
	AvoidSpheresConstraint(vector<SphereObstacle> obstacles) :m_SphereObstacles(obstacles) {}

	void UpdateObstacles(const vector<SphereObstacle>& obstacles);
	float WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority) override;
	Goal Suggest(const Path* pPath) override;

private:
	float m_AvoidMargin = 2.f;
	vector<SphereObstacle> m_SphereObstacles = {};
	float WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority, const SphereObstacle& sphere); //Internal single sphere check

	Goal m_SuggestedGoal = {};
};


//***********************************************************************************
//4. ACTUATOR (Basic Actuator, Seek path goal)
class BasicActuator : public Actuator
{
public:
	BasicActuator(Seek* pSeekBehaviour) :m_pSeekBehaviour(pSeekBehaviour) {}
	~BasicActuator() { SAFE_DELETE(m_pPath); }

	Path* CreatePath() override;
	void UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal) override;
	SteeringPlugin_Output CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent) override;

private:
	Path* m_pPath = nullptr;
	Seek* m_pSeekBehaviour = nullptr;
};