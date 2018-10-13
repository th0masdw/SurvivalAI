#pragma once

#include "Weapon.h"
#include "Exam_HelperStructs.h"

class WeaponSelector
{
public:
	WeaponSelector();
	~WeaponSelector();

	void Update(const AgentInfo& agent, const EnemyInfo& enemy);
	int GetBestWeaponSlot() const;
	UINT GetWeaponCount() const;

	void AddWeapon(int ammo, float dps, float range, int slot);
	void RemoveWeapon(int slot);

private:
	list<Weapon> m_Weapons;

	float GetShotCountUtility(const Weapon& weapon, const EnemyInfo& enemy) const;
	float GetAmmoUtility(const Weapon& weapon, const EnemyInfo& enemy) const;
	float GetRangeUtility(const Weapon& weapon, const AgentInfo& agent, const EnemyInfo& enemy) const;
};