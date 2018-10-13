#pragma once

struct Weapon
{
public:
	Weapon(int ammo, float dps, float range, int slot)
		: Ammo(ammo),
		DPS(dps),
		Range(range),
		SlotID(slot),
		Utility(0.0f)
	{
	}

	int Ammo;
	float DPS;
	float Range;
	int SlotID;
	float Utility;
};