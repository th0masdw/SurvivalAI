#include "stdafx.h"
#include "WeaponSelector.h"
using namespace Elite;

#define UTILITY_FUNCTION_COUNT 3

WeaponSelector::WeaponSelector()
	: m_Weapons()
{
}

WeaponSelector::~WeaponSelector()
{
	m_Weapons.clear();
}

void WeaponSelector::Update(const AgentInfo& agent, const EnemyInfo& enemy)
{
	if (enemy.Size <= 0.0f)
		return;

	//Reset + calculate utility
	for (Weapon& weapon : m_Weapons) {
		weapon.Utility = 0.0f;

		weapon.Utility += GetShotCountUtility(weapon, enemy);
		weapon.Utility += GetAmmoUtility(weapon, enemy);
		weapon.Utility += GetRangeUtility(weapon, agent, enemy);

		weapon.Utility /= UTILITY_FUNCTION_COUNT;
	}
}

int WeaponSelector::GetBestWeaponSlot() const
{
	auto best = max_element(m_Weapons.begin(), m_Weapons.end(), [](const Weapon& a, const Weapon& b) {
		return a.Utility < b.Utility;
	});

	return best->SlotID;
}

UINT WeaponSelector::GetWeaponCount() const
{
	return m_Weapons.size();
}

void WeaponSelector::AddWeapon(int ammo, float dps, float range, int slot)
{
	m_Weapons.push_back(Weapon(ammo, dps, range, slot));
}

void WeaponSelector::RemoveWeapon(int slot)
{
	m_Weapons.erase(remove_if(m_Weapons.begin(), m_Weapons.end(), [slot](const Weapon& weapon) {
		return (weapon.SlotID == slot);
	}), m_Weapons.end());
}

float WeaponSelector::GetShotCountUtility(const Weapon& weapon, const EnemyInfo& enemy) const
{
	float utility = weapon.DPS / enemy.Health;
	if (utility > 1.0f)
		utility = 1.0f;

	return utility;
}

float WeaponSelector::GetAmmoUtility(const Weapon& weapon, const EnemyInfo& enemy) const
{
	float shotCount = ceil(enemy.Health / weapon.DPS);
	float t = (weapon.Ammo / shotCount) / weapon.Ammo;

	return (shotCount > weapon.Ammo) ? 0.0f : Lerp(0.5f, 1.0f, t);
}

float WeaponSelector::GetRangeUtility(const Weapon& weapon, const AgentInfo& agent, const EnemyInfo& enemy) const
{
	float distance = Distance(agent.Position, enemy.Location);
	float t = (weapon.Range / distance) / weapon.Range;

	return (distance > weapon.Range) ? 0.0f : Lerp(0.5f, 1.0f, t);
}