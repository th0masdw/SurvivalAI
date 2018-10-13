#pragma once
#include "stdafx.h"
#include "Blackboard.h"
#include "BehaviorTree.h"
#include "UtilitySystem\WeaponSelector.h"
using namespace Elite;

BehaviorState StartAiming(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("IsAiming", true);
	return Success;
}

BehaviorState StopAiming(Blackboard* pBlackboard)
{
	pBlackboard->ChangeData("IsAiming", false);
	return Success;
}

BehaviorState SetFaceTarget(Blackboard* pBlackboard)
{
	//Get data
	EnemyInfo closestEnemy = {};
	Face* pFaceBehavior = nullptr;
	bool foundData = pBlackboard->GetData("ClosestEnemy", closestEnemy) &&
					 pBlackboard->GetData("FaceBehavior", pFaceBehavior);

	if (!foundData || !pFaceBehavior)
		return Failure;

	TargetData target = { closestEnemy.Location, 0.0f };
	pFaceBehavior->SetTarget(target);

	return Success;
}

bool HasTargetInSight(Blackboard* pBlackboard)
{
	//Get data
	AgentInfo agent = {};
	EnemyInfo enemy = {};
	float minDP = 0.0f;
	bool foundData = pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("ClosestEnemy", enemy) &&
					 pBlackboard->GetData("MinAimDP", minDP);

	if (!foundData)
		return false;

	Vector2 agentForward = OrientationToVector(agent.Orientation);
	Vector2 toEnemy = enemy.Location - agent.Position;
	Normalize(agentForward);
	Normalize(toEnemy);

	return (Dot(agentForward, toEnemy) >= minDP);
}

bool HasWeapon(Blackboard* pBlackboard)
{
	//Get data
	WeaponSelector* pWeaponSelector = nullptr;
	bool foundData = pBlackboard->GetData("WeaponSelector", pWeaponSelector);

	if (!foundData || !pWeaponSelector)
		return false;

	return (pWeaponSelector->GetWeaponCount() > 0);
}

BehaviorState Shoot(Blackboard* pBlackboard)
{
	//Get data
	IExamInterface* pInterface = nullptr;
	WeaponSelector* pWeaponSelector = nullptr;
	bool foundData = pBlackboard->GetData("Interface", pInterface) &&
					 pBlackboard->GetData("WeaponSelector", pWeaponSelector);

	if (!foundData || !pInterface || !pWeaponSelector)
		return Failure;

	UINT slotIndex = pWeaponSelector->GetBestWeaponSlot();
	pInterface->Inventory_UseItem(slotIndex);

	return Success;
}

BehaviorState RemoveEmptyWeapons(Blackboard* pBlackboard)
{
	//Get data
	IExamInterface* pInterface = nullptr;
	WeaponSelector* pWeaponSelector = nullptr;
	vector<bool> slots = {};
	bool foundData = pBlackboard->GetData("Interface", pInterface) &&
					 pBlackboard->GetData("WeaponSelector", pWeaponSelector) &&
					 pBlackboard->GetData("InventorySlots", slots);

	if (!foundData || !pInterface || !pWeaponSelector)
		return Failure;

	ItemInfo info = {};
	int ammo = 0;
	for (UINT i = 0; i < slots.size(); ++i) {
		if (slots[i] && pInterface->Inventory_GetItem(i, info)) {
			if (info.Type == eItemType::PISTOL) {
				ammo = pInterface->Item_GetMetadata(info, "ammo");

				if (ammo <= 0) {
					pInterface->Inventory_RemoveItem(i);
					pWeaponSelector->RemoveWeapon(i);
					slots[i] = false;
				}
			}
		}
	}

	pBlackboard->ChangeData("InventorySlots", slots);
	return Success;
}