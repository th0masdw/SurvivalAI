#pragma once
#include "stdafx.h"
#include "Blackboard.h"
#include "BehaviorTree.h"
#include "IExamInterface.h"
#include "SteeringPipeline\SteeringPipeline_Implementation.h"
using namespace Elite;

bool HasEnemiesInFOV(Blackboard* pBlackboard)
{
	//Get data
	vector<EnemyInfo> enemies{};
	bool foundData = pBlackboard->GetData("Enemies", enemies);

	if (!foundData)
		return false;

	return (enemies.size() > 0);
}

BehaviorState UpdateEnemies(Blackboard* pBlackboard)
{
	//Get data
	vector<EnemyInfo> enemies{};
	bool foundData = pBlackboard->GetData("Enemies", enemies);

	if (!foundData)
		return Failure;

	Vector2 enemyLocation = {};
	for (const EnemyInfo& enemy : enemies) {
		enemyLocation += enemy.Location;
	}
	enemyLocation /= static_cast<float>(enemies.size());

	pBlackboard->ChangeData("EnemyLocation", enemyLocation);
	pBlackboard->ChangeData("HasSeenEnemies", true);

	return Success;
}

BehaviorState UpdateClosestEnemy(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	vector<EnemyInfo> enemies = {};
	bool foundData = pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("Enemies", enemies);

	if (!foundData)
		return Failure;

	auto closest = min_element(enemies.begin(), enemies.end(), [agent](const EnemyInfo& a, const EnemyInfo& b) {
		return Distance(a.Location, agent.Position) < Distance(b.Location, agent.Position);
	});

	pBlackboard->ChangeData("ClosestEnemy", *closest);
	return Success;
}

BehaviorState DodgeEnemies(Blackboard* pBlackboard)
{
	//Get data
	Vector2 enemyPos = ZeroVector2;
	float dodgeRadius = 0.0f;
	AvoidSpheresConstraint* pConstraint = nullptr;
	bool foundData = pBlackboard->GetData("EnemyLocation", enemyPos) &&
					 pBlackboard->GetData("DodgeRadius", dodgeRadius) &&
					 pBlackboard->GetData("Constraint", pConstraint);

	if (!foundData || !pConstraint)
		return Failure;

	SphereObstacle obstacle = {};
	obstacle.Position = enemyPos;
	obstacle.Radius = dodgeRadius;
		
	pConstraint->UpdateObstacles({ obstacle });

	return Success;
}

bool HasSeenEnemies(Blackboard* pBlackboard)
{
	//Get data
	bool haveSeen = false;
	bool foundData = pBlackboard->GetData("HasSeenEnemies", haveSeen);

	if (!foundData)
		return false;

	return haveSeen;
}

bool HasLostEnemies(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	Vector2 enemyLocation = ZeroVector2;
	float maxDistance = 0.0f;

	bool foundData = pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("EnemyLocation", enemyLocation) &&
					 pBlackboard->GetData("MaxEnemyDistance", maxDistance);

	if (!foundData)
		return false;

	return (Distance(agent.Position, enemyLocation) > maxDistance);
}

BehaviorState ForgetEnemies(Blackboard* pBlackboard)
{
	AvoidSpheresConstraint* pConstraint = nullptr;
	bool foundData = pBlackboard->GetData("Constraint", pConstraint);

	if (!foundData || !pConstraint)
		return Failure;

	pBlackboard->ChangeData("HasSeenEnemies", false);
	pBlackboard->ChangeData("EnemyLocation", ZeroVector2);
	pConstraint->UpdateObstacles({});

	//Logging
	printf("Forgot enemies...\n");

	return Success;
}

bool HasNoEnemiesInFOV(Blackboard* pBlackboard)
{
	//Get data
	vector<EnemyInfo> enemies{};
	bool foundData = pBlackboard->GetData("Enemies", enemies);

	if (!foundData)
		return false;

	return enemies.empty();
}

BehaviorState ClearClosestEnemy(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData<EnemyInfo>("ClosestEnemy", {});
	return Success;
}

BehaviorState UpdateEnemyTimer(Blackboard* pBlackboard)
{
	//Get data
	float enemyTimer = 0.0f;
	float deltaTime = 0.0f;
	bool foundData = pBlackboard->GetData("EnemyTimer", enemyTimer) &&
					 pBlackboard->GetData("DeltaTime", deltaTime);

	if (!foundData)
		return Failure;

	pBlackboard->ChangeData("EnemyTimer", (enemyTimer + deltaTime));
	return Success;
}

bool HasEnemyTimerExceeded(Blackboard* pBlackboard)
{
	//Get data
	float enemyTimer = 0.0f;
	float maxTime = 0.0f;
	bool foundData = pBlackboard->GetData("EnemyTimer", enemyTimer) &&
					 pBlackboard->GetData("MaxEnemyTime", maxTime);

	if (!foundData)
		return false;

	return (enemyTimer > maxTime);
}

BehaviorState ResetEnemyTimer(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("EnemyTimer", 0.0f);
	return Success;
}