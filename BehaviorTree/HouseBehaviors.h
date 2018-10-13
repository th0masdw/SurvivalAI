#pragma once
#include "stdafx.h"
#include "Blackboard.h"
#include "BehaviorTree.h"
using namespace Elite;

enum class HouseState
{
	Searching, Going, Inside
};

#pragma region Helper Functions
bool IsHouseSet(const HouseInfo& house)
{
	return (house.Size.SqrtMagnitude() > FLT_EPSILON);
}
#pragma endregion

bool IsHouseStateSearching(Blackboard* pBlackboard)
{
	//Get data
	HouseState state = HouseState::Searching;
	bool foundData = pBlackboard->GetData("HouseState", state);

	if (!foundData)
		return false;

	return (state == HouseState::Searching);
}

bool IsHouseStateGoing(Blackboard* pBlackboard)
{
	//Get data
	HouseState state = HouseState::Searching;
	bool foundData = pBlackboard->GetData("HouseState", state);

	if (!foundData)
		return false;

	return (state == HouseState::Going);
}

bool IsHouseStateInside(Blackboard* pBlackboard)
{
	//Get data
	HouseState state = HouseState::Searching;
	bool foundData = pBlackboard->GetData("HouseState", state);

	if (!foundData)
		return false;

	return (state == HouseState::Inside);
}

BehaviorState ToHouseSearchingState(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("HouseState", HouseState::Searching);

	//Logging
	printf("Searching new house...\n");

	return Success;
}

BehaviorState ToHouseGoingState(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("HouseState", HouseState::Going);

	//Logging
	printf("Going to house...\n");

	return Success;
}

BehaviorState ToHouseInsideState(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("HouseState", HouseState::Inside);

	//Logging
	printf("I'm inside the house\n");

	return Success;
}

bool HasHouseInFOV(Blackboard* pBlackboard)
{
	//Get data
	HouseInfo house = {};
	bool foundData = pBlackboard->GetData("NearbyHouse", house);

	if (!foundData)
		return false;

	return IsHouseSet(house);
}

bool HasNotVisitedHouse(Blackboard* pBlackboard)
{
	//Get data
	HouseInfo nearbyHouse = {};
	deque<HouseInfo> visitedHouses = {};
	bool foundData = pBlackboard->GetData("NearbyHouse", nearbyHouse) &&
					 pBlackboard->GetData("VisitedHouses", visitedHouses);

	if (!foundData || !IsHouseSet(nearbyHouse))
		return false;

	Vector2 nearbyPos = nearbyHouse.Center;
	Vector2 lastPos = {};
	for (const HouseInfo& house : visitedHouses) {
		lastPos = house.Center;

		if (Distance(nearbyPos, lastPos) <= FLT_EPSILON) 
			return false;
	}
	return true;
}

BehaviorState SetTargetFromHouse(Blackboard* pBlackboard)
{
	//Get data
	HouseInfo newHouse = {};
	bool foundData = pBlackboard->GetData("TargetHouse", newHouse);

	if (!foundData || !IsHouseSet(newHouse))
		return Failure;

	pBlackboard->ChangeData("MoveTarget", newHouse.Center);

	return Success;
}

bool IsInsideHouse(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	bool foundData = pBlackboard->GetData("Agent", agent);

	if (!foundData)
		return false;

	return agent.IsInHouse;
}

BehaviorState SetHouseAsVisited(Blackboard* pBlackboard)
{
	//Get data
	HouseInfo newHouse = {};
	deque<HouseInfo> visitedHouses = {};
	int maxHouseCount = 0;
	bool foundData = pBlackboard->GetData("NearbyHouse", newHouse) &&
					 pBlackboard->GetData("VisitedHouses", visitedHouses) &&
					 pBlackboard->GetData("MaxHouseVisited", maxHouseCount);

	if (!foundData || !IsHouseSet(newHouse))
		return Failure;

	if (visitedHouses.size() >= static_cast<UINT>(maxHouseCount))
		visitedHouses.pop_front();

	visitedHouses.push_back(newHouse);

	pBlackboard->ChangeData("VisitedHouses", visitedHouses);
	return Success;
}

bool IsOutsideHouse(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	bool foundData = pBlackboard->GetData("Agent", agent);

	if (!foundData)
		return false;

	return !agent.IsInHouse;
}

BehaviorState UpdateTargetHouse(Blackboard* pBlackboard)
{
	//Get data
	HouseInfo nearest = {};
	bool foundData = pBlackboard->GetData("NearbyHouse", nearest);

	if (!foundData || !IsHouseSet(nearest))
		return Failure;

	pBlackboard->ChangeData("TargetHouse", nearest);

	//Logging
	printf("New target house\n");

	return Success;
}

BehaviorState ClearTargetHouse(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData<HouseInfo>("TargetHouse", {});

	//Logging
	printf("Cleared target house\n");

	return Success;
}

bool IsInCenterOfHouse(Blackboard* pBlackboard)
{
	//Get data
	HouseInfo current = {};
	AgentInfo agent = {};
	float radius = 0.0f;
	bool foundData = pBlackboard->GetData("TargetHouse", current) &&
					 pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("HouseInspectionRadius", radius);

	if (!foundData)
		return false;

	return (Distance(current.Center, agent.Position) <= radius);
}

bool HasNotInspectedHouse(Blackboard* pBlackboard)
{
	//Get data
	bool hasInspected = false;
	bool foundData = pBlackboard->GetData("HasInspectedHouse", hasInspected);

	if (!foundData)
		return false;

	return !hasInspected;
}

BehaviorState SetHouseInspected(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("HasInspectedHouse", true);

	//Logging
	printf("Inspected house\n");

	return Success;
}

BehaviorState ResetHouseInspected(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("HasInspectedHouse", false);
	return Success;
}