#include "stdafx.h"
#include "SteeringBehaviours.h"
#include "IExamInterface.h"

//SEEK
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_TargetRef.Position - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	//Calculate WanderOffset
	Vector2 offset = pAgent->LinearVelocity;
	offset.Normalize();
	offset *= m_Offset;

	//WanderCircle Offset (Polar to Cartesian Coordinates)
	Vector2 circleOffset = { cos(m_WanderAngle) * m_Radius, sin(m_WanderAngle) * m_Radius };

	//Change the WanderAngle slightly for next frame
	m_WanderAngle += randomFloat() * m_AngleChange - (m_AngleChange * .5f); //RAND[-angleChange/2,angleChange/2]

	//Set target as Seek::Target
	Vector2 newTarget = pAgent->Position + offset + circleOffset;
	Seek::m_TargetRef = newTarget;

	return Seek::CalculateSteering(deltaT, pAgent);
}

//ALIGN
//*****
SteeringPlugin_Output Align::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	auto rotation = m_TargetRef.Orientation - pAgent->Orientation;

	//Wrap Angle
	auto a = fmodf(rotation + b2_pi, 2 * b2_pi);
	rotation = a >= 0 ? (a - b2_pi) : (a + b2_pi);

	auto rotationSize = abs(rotation);

	if (rotationSize <= 0 || rotationSize < m_TargetAngle)
		return steering; //Should be empty :)

	auto targetRotationSpeed = pAgent->MaxAngularSpeed;
	if (rotationSize < m_SlowAngle)
	{
		targetRotationSpeed = pAgent->MaxAngularSpeed * rotationSize / m_SlowAngle;
	}

	targetRotationSpeed *= rotation / rotationSize;
	steering.AngularVelocity = targetRotationSpeed - pAgent->AngularVelocity;
	steering.AngularVelocity /= m_TimeToTarget;

	auto angularSpeed = abs(steering.AngularVelocity);
	if (angularSpeed > pAgent->MaxAngularSpeed)
	{
		steering.AngularVelocity /= angularSpeed;
		steering.AngularVelocity *= pAgent->MaxAngularSpeed;
	}

	steering.LinearVelocity = ZeroVector2;
	return steering;
}

//FACE
//****
SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	auto direction = m_TargetRef.Position - pAgent->Position;
	auto distance = direction.Magnitude();

	if (distance == 0.f)
		return steering; //Should be empty

	//Assemble Align Target (= new orientation > face target)
	auto newTarget = m_TargetRef;
	newTarget.Orientation = atan2f(direction.x, -direction.y);

	//ALIGN BEHAVIOUR
	Align::m_TargetRef = newTarget;
	return Align::CalculateSteering(deltaT, pAgent);
}