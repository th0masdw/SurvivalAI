#pragma once
#include "stdafx.h"
using namespace Elite;

struct TargetData
{
	Vector2 Position;
	float Orientation;

	TargetData(Vector2 position = ZeroVector2, float orientation = 0.f) 
		: Position(position),
		Orientation(orientation)
	{
	}

	//Functions
	void Clear()
	{
		Position = ZeroVector2;
		Orientation = 0.f;
	}

	Vector2 GetDirection() const
	{
		return Vector2(cos(Orientation - b2_pi / 2.f), sin(Orientation - b2_pi / 2.f));
	}

	//Operator overloads
	TargetData(const TargetData& other)
	{
		Position = other.Position;
		Orientation = other.Orientation;
	}

	TargetData& operator=(const TargetData& other)
	{
		Position = other.Position;
		Orientation = other.Orientation;

		return *this;
	}

	bool operator==(const TargetData& other) const
	{
		return Position == other.Position && Orientation == other.Orientation;
	}

	bool operator!=(const TargetData& other) const
	{
		return Position != other.Position || Orientation != other.Orientation;
	}
};