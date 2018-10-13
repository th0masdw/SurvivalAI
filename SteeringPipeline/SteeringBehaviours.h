#pragma once

#include "Exam_HelperStructs.h"
#include "TargetData.h"
class IExamInterface;
using namespace Elite;

class ISteeringBehaviour
{
public:
	ISteeringBehaviour() {}
	ISteeringBehaviour(IExamInterface* pContext):m_pContext(pContext){}
	virtual ~ISteeringBehaviour(){}

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehaviour, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	IExamInterface* m_pContext = nullptr;
};

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehaviour
{
public:
	Seek() {};
	Seek(IExamInterface* pContext) :ISteeringBehaviour(pContext) {}
	virtual ~Seek() {};

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	virtual void SetTarget(const TargetData& target) { m_TargetRef = target; }

protected:
	TargetData m_TargetRef;
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() {};
	Wander(IExamInterface* pContext) :Seek(pContext) {}
	virtual ~Wander() {};

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_AngleChange = rad; }

protected:
	float m_Offset = 6.f; //Offset (Agent Direction)
	float m_Radius = 4.f; //WanderRadius
	float m_AngleChange = ToRadians(45); //Max WanderAngle change per frame
	float m_WanderAngle = 0.f; //Internal

private:
	void SetTarget(const TargetData& target) override {}
};

/////////////////////////////////////////
//ALIGN
//******
class Align : public ISteeringBehaviour
{
public:
	Align() {}
	Align(IExamInterface* pContext) :ISteeringBehaviour(pContext) {}
	virtual ~Align() {}

	//Align Fuctions
	virtual void SetTarget(const TargetData& target) { m_TargetRef = target; }

	void SetTargetAngle(float rad) { m_TargetAngle = rad; }
	void SetSlowAngle(float rad) { m_SlowAngle = rad; }
	void SetTimeToTarget(float time) { m_TimeToTarget = time; }

	//Align Behaviour
	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

protected:
	TargetData m_TargetRef;

private:
	float m_TargetAngle = ToRadians(1);
	float m_SlowAngle = ToRadians(0);
	float m_TimeToTarget = 0.1f;
};

/////////////////////////////////////////
//FACE
//******
class Face : public Align
{
public:
	Face() {}
	Face(IExamInterface* pContext) :Align(pContext) {}
	virtual ~Face() {}

	//Face Functions (Align::SetTarget override)
	virtual void SetTarget(const TargetData& target) { m_TargetRef = target; }

	//Face Behaviour
	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agent) override;

protected:
	TargetData m_TargetRef;
};