#include "stdafx.h"
#include "SteeringPipeline_Implementation.h"
#include "IExamInterface.h"

#undef max

//1. TARGETER (FixedGoalTargeter)
//*******************************
Goal& FixedGoalTargeter::GetGoalRef()
{
	return m_Goal;
}

Goal FixedGoalTargeter::GetGoal()
{
	return m_Goal;
}

//2. DECOMPOSER (skip for now)

//3. CONSTRAINT (Avoid Sphere Obstacles)
//**************************************
void AvoidSpheresConstraint::UpdateObstacles(const vector<SphereObstacle>& obstacles)
{
	m_SphereObstacles = move(obstacles);
}

float AvoidSpheresConstraint::WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority)
{
	float currPriority = 0.f;
	float smallestPriority = maxPriority;
	for (auto sphere : m_SphereObstacles)
	{
		currPriority = WillViolate(pPath, pAgent, maxPriority, sphere);
		if (currPriority < smallestPriority) smallestPriority = currPriority;
	}

	return smallestPriority;
}

Goal AvoidSpheresConstraint::Suggest(const Path* pPath)
{
	return m_SuggestedGoal; //Should be calculated during WillViolate
}

//Internal single sphere check
float AvoidSpheresConstraint::WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority, const SphereObstacle& sphere)
{
	//1. Calculate the closest point on the vector between the Agent and Goal to the Obstacle
	Vector2 a2t = (pPath->GetGoal().Position - pAgent->Position).GetNormalized();
	Vector2 a2s = sphere.Position - pAgent->Position;
	float distToPoint = Dot(a2s, a2t);
	Vector2 closestPoint = pAgent->Position + (a2t * distToPoint);

	if (distToPoint < 0 || distToPoint >= maxPriority)
		return FLT_MAX;
	
	//2. Use that previously calculated 'closestPoint' to calculate the offsetDirection
	Vector2 offsetDirection = (closestPoint - sphere.Position).GetNormalized();
	float distFromObstacleCenter = Distance(closestPoint, sphere.Position);

	if (distFromObstacleCenter > sphere.Radius + m_AvoidMargin)
		return FLT_MAX;

	//3. Calculate 'newTarget' (=subgoal)
	Vector2 newTarget = sphere.Position + ((sphere.Radius + m_AvoidMargin) * offsetDirection);
	float distToNewTarget = Distance(pAgent->Position, newTarget);

	if (distToNewTarget >= maxPriority)
		return FLT_MAX;

	//4. Set 'm_SuggestedGoal.Position' to newTarget
	m_SuggestedGoal.Position = newTarget;
	m_SuggestedGoal.PositionSet = true;

	return distToNewTarget;
}

//4. ACTUATOR (Basic Seek Path Point)
//***********************************
Path* BasicActuator::CreatePath()
{
	//Create a new Path Object (Delete previous, if valid)
	SAFE_DELETE(m_pPath);

	m_pPath = new Path();
	return m_pPath;
}

void BasicActuator::UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal)
{
	//Assign the Agent & Goal to the Path Object
	pPath->SetAgent(pAgent);
	pPath->SetGoal(goal);
}

SteeringPlugin_Output BasicActuator::CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent)
{
	//1. Retrieve the goal from the Path Object
	//2. Is the goal has a position component, use the Seek behaviour to steer towards it
	Goal goal = pPath->GetGoal();

	if (goal.PositionSet && m_pSeekBehaviour) {
		TargetData newTarget = TargetData(goal.Position);
		m_pSeekBehaviour->SetTarget(newTarget);

		return m_pSeekBehaviour->CalculateSteering(deltaT, pAgent);
	}

	return SteeringPlugin_Output(); //empty
}