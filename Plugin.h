#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "SteeringPipeline\SteeringPipeline_Implementation.h"

class IBaseInterface;
class IExamInterface;
class BehaviorTree;
class Blackboard;
class WeaponSelector;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void ProcessEvents(const SDL_Event& e) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	//Steering pipeline
	Seek* m_pSeekBehavior = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Face* m_pFaceBehavior = nullptr;
	SteeringPipeline* m_pSteeringPipeline = nullptr;
	FixedGoalTargeter* m_pTargeter = nullptr;
	AvoidSpheresConstraint* m_pConstraint = nullptr;
	BasicActuator* m_pActuator = nullptr;

	//Behavior tree
	BehaviorTree* m_pBehaviorTree = nullptr;

	//Utility-based AI
	WeaponSelector* m_pWeaponSelector = nullptr;

	//Functions
	void InitPipeline();
	void InitBehaviorTree();
	void SetBlackboardData(Blackboard* pBlackboard);
	SteeringPlugin_Output GetSteering();

	void UpdateBlackboard(float dt);
	void UpdateEntitiesInFOV(Blackboard* pBlackboard);
	void UpdateNearestHouse(Blackboard* pBlackboard, const AgentInfo& agent);
	void UpdateWeaponSelector(Blackboard* pBlackboard);

	void DrawBlackboardInfo() const;

	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}