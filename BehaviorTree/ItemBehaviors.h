#pragma once
#include "stdafx.h"
#include "Blackboard.h"
#include "BehaviorTree.h"
#include "UtilitySystem\WeaponSelector.h"
using namespace Elite;

#pragma region Helper Functions
bool IsItemSet(const EntityInfo& item)
{
	return (item.Location.SqrtMagnitude() > FLT_EPSILON);
}

int GetEmptyInventorySlot(const vector<bool>& slots)
{
	for (UINT i = 0; i < slots.size(); ++i) {
		if (!slots[i]) return i;
	}

	return -1;
}

int GetItemCount(IExamInterface* pInterface, const vector<bool>& slots, const eItemType itemType)
{
	int count = 0;
	ItemInfo info = {};

	for (UINT i = 0; i < slots.size(); ++i) {
		if (slots[i] && pInterface->Inventory_GetItem(i, info))
			if (info.Type == itemType) ++count;
	}

	return count;
}

bool CanTakeItem(const ItemInfo& item, IExamInterface* pInterface, const vector<bool>& slots, int maxFood, int maxMedkit, int maxWeapons)
{
	switch (item.Type) {
		case eItemType::FOOD:
			return (GetItemCount(pInterface, slots, item.Type) < maxFood) ? true : false;

		case eItemType::MEDKIT:
			return (GetItemCount(pInterface, slots, item.Type) < maxMedkit) ? true : false;

		case eItemType::PISTOL:
			return (GetItemCount(pInterface, slots, item.Type) < maxWeapons) ? true : false;

		case eItemType::GARBAGE:
			return false;

		default:
			return false;
	}
}

bool ShouldEatFood(IExamInterface* pInterface, const AgentInfo& agent, const ItemInfo& item)
{
	int energy = pInterface->Item_GetMetadata(item, "energy");
	return (agent.Energy <= (10 - energy));
}

bool ShouldUseMedkit(IExamInterface* pInterface, const AgentInfo& agent, const ItemInfo& item)
{
	int health = pInterface->Item_GetMetadata(item, "health");
	return (agent.Health <= (10 - health));
}
#pragma endregion

bool HasItemsInFOV(Blackboard* pBlackboard)
{
	//Get data
	vector<EntityInfo> items = {};
	bool foundData = pBlackboard->GetData("Items", items);

	if (!foundData)
		return false;

	return items.size() > 0;
}

bool IsTargetItemNotSet(Blackboard* pBlackboard)
{
	//Get data
	EntityInfo item = {};
	bool foundData = pBlackboard->GetData("TargetItem", item);

	if (!foundData)
		return false;

	return !IsItemSet(item);
}

bool IsTargetItemSet(Blackboard* pBlackboard)
{
	//Get data
	EntityInfo item = {};
	bool foundData = pBlackboard->GetData("TargetItem", item);

	if (!foundData)
		return false;

	return IsItemSet(item);
}

BehaviorState UpdateTargetItem(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	vector<EntityInfo> items = {};
	bool foundData = pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("Items", items);

	if (!foundData || items.empty())
		return Failure;

	auto closest = min_element(items.begin(), items.end(), [agent](const EntityInfo& left, const EntityInfo& right) {
		return Distance(left.Location, agent.Position) < Distance(right.Location, agent.Position);
	});

	pBlackboard->ChangeData("TargetItem", *closest);

	//Logging
	printf("New closest item\n");

	return Success;
}

bool FarFromTargetItem(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	EntityInfo item = {};
	bool foundData = pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("TargetItem", item);

	if (!foundData)
		return false;

	return (Distance(agent.Position, item.Location) > agent.GrabRange);
}

BehaviorState GoToItem(Blackboard* pBlackboard)
{
	EntityInfo item = {};
	bool foundData = pBlackboard->GetData("TargetItem", item);

	if (!foundData)
		return Failure;

	pBlackboard->ChangeData("MoveTarget", item.Location);

	return Success;
}

bool NearTargetItem(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	EntityInfo item = {};
	bool foundData = pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("TargetItem", item);

	if (!foundData)
		return false;

	return (Distance(agent.Position, item.Location) <= agent.GrabRange);
}

bool HasRoomInInventory(Blackboard* pBlackboard)
{
	//Get data
	vector<bool> slots = {};
	bool foundData = pBlackboard->GetData("InventorySlots", slots);

	if (!foundData)
		return false;

	int freeSlots = count(slots.begin(), slots.end(), false);
	return (freeSlots > 0);
}

BehaviorState GrabItem(Blackboard* pBlackboard)
{
	//Get data
	IExamInterface* pInterface = nullptr;
	EntityInfo item = {};
	vector<bool> slots = {};
	int maxFoodCount = 0;
	int maxMedkitCount = 0;
	int maxWeaponCount = 0;
	WeaponSelector* pSelector = nullptr;
	bool foundData = pBlackboard->GetData("Interface", pInterface) &&
					 pBlackboard->GetData("TargetItem", item) &&
					 pBlackboard->GetData("InventorySlots", slots) &&
					 pBlackboard->GetData("WeaponSelector", pSelector) &&
					 pBlackboard->GetData("MaxNumberFood", maxFoodCount) &&
					 pBlackboard->GetData("MaxNumberMedkits", maxMedkitCount) &&
					 pBlackboard->GetData("MaxNumberPistols", maxWeaponCount);

	if (!foundData || !pInterface || !pSelector)
		return Failure;

	ItemInfo info = {};
	int index = GetEmptyInventorySlot(slots);

	if (index < 0)
		return Failure;
	
	if (pInterface->Item_Grab(item, info)) {

		if (CanTakeItem(info, pInterface, slots, maxFoodCount, maxMedkitCount, maxWeaponCount)) {
			pInterface->Inventory_AddItem(index, info);
			slots[index] = true;

			if (info.Type == eItemType::PISTOL) {
				int ammo = pInterface->Item_GetMetadata(info, "ammo");
				float dps = pInterface->Item_GetMetadata(info, "dps");
				float range = pInterface->Item_GetMetadata(info, "range");
				pSelector->AddWeapon(ammo, dps, range, index);
			}

			pBlackboard->ChangeData("InventorySlots", move(slots));
		} else {
			pInterface->Inventory_AddItem(index, info);
			pInterface->Inventory_RemoveItem(index);
		}
	}

	return Success;
}

BehaviorState ResetTargetItem(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData<EntityInfo>("TargetItem", {});

	//Logging
	printf("Reset target item\n");

	return Success;
}

BehaviorState EatFood(Blackboard* pBlackboard)
{
	//Get data
	vector<bool> slots = {};
	IExamInterface* pInterface = nullptr;
	AgentInfo agent = {};
	bool foundData = pBlackboard->GetData("InventorySlots", slots) &&
					 pBlackboard->GetData("Interface", pInterface) &&
					 pBlackboard->GetData("Agent", agent);

	if (!foundData || !pInterface)
		return Failure;

	ItemInfo info = {};

	for (UINT i = 0; i < slots.size(); ++i) {
		if (slots[i] && pInterface->Inventory_GetItem(i, info)) {
			if (info.Type == eItemType::FOOD && ShouldEatFood(pInterface, agent, info)) {
				pInterface->Inventory_UseItem(i);
				pInterface->Inventory_RemoveItem(i);
				slots[i] = false;
				break;
			}
		}
	}
	pBlackboard->ChangeData("InventorySlots", slots);

	return Success;
}

BehaviorState UseMedkit(Blackboard* pBlackboard)
{
	//Get data
	vector<bool> slots = {};
	IExamInterface* pInterface = nullptr;
	AgentInfo agent = {};
	bool foundData = pBlackboard->GetData("InventorySlots", slots) &&
					 pBlackboard->GetData("Interface", pInterface) &&
					 pBlackboard->GetData("Agent", agent);

	if (!foundData || !pInterface)
		return Failure;

	ItemInfo info = {};

	for (UINT i = 0; i < slots.size(); ++i) {
		if (slots[i] && pInterface->Inventory_GetItem(i, info)) {
			if (info.Type == eItemType::MEDKIT && ShouldUseMedkit(pInterface, agent, info)) {
				pInterface->Inventory_UseItem(i);
				pInterface->Inventory_RemoveItem(i);
				slots[i] = false;
				break;
			}
		}
	}
	pBlackboard->ChangeData("InventorySlots", slots);

	return Success;
}