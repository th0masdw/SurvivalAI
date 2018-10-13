#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "BehaviorTree\Blackboard.h"
#include "BehaviorTree\BehaviorTree.h"
#include "BehaviorTree\Behaviors.h"
#include "UtilitySystem\WeaponSelector.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Rick Grimes";
	info.Student_FirstName = "Thomas";
	info.Student_LastName = "De Wispelaere";
	info.Student_Class = "2DAE4";

	m_pWeaponSelector = new WeaponSelector();
	InitPipeline();
	InitBehaviorTree();
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called when the plugin gets unloaded
	SAFE_DELETE(m_pSteeringPipeline);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pFaceBehavior);
	SAFE_DELETE(m_pTargeter);
	SAFE_DELETE(m_pConstraint);
	SAFE_DELETE(m_pActuator);
	SAFE_DELETE(m_pBehaviorTree);
	SAFE_DELETE(m_pWeaponSelector);
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = false; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.LevelFile = "LevelOne.gppl";
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.OverrideDifficulty = false;
	params.Difficulty = 2.f;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::ProcessEvents(const SDL_Event& e)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	switch (e.type)
	{
	case SDL_MOUSEBUTTONUP:
	{
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			int x, y;
			SDL_GetMouseState(&x, &y);
			const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(x), static_cast<float>(y));
			m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
		}
		break;
	}
	case SDL_KEYDOWN:
	{
		if (e.key.keysym.sym == SDLK_SPACE)
		{
			m_CanRun = true;
		}
		else if (e.key.keysym.sym == SDLK_LEFT)
		{
			m_AngSpeed -= Elite::ToRadians(10);
		}
		else if (e.key.keysym.sym == SDLK_RIGHT)
		{
			m_AngSpeed += Elite::ToRadians(10);
		}
		else if (e.key.keysym.sym == SDLK_g)
		{
			m_GrabItem = true;
		}
		else if (e.key.keysym.sym == SDLK_u)
		{
			m_UseItem = true;
		}
		else if (e.key.keysym.sym == SDLK_r)
		{
			m_RemoveItem = true;
		}
		break;
	}
	case SDL_KEYUP:
	{
		if (e.key.keysym.sym == SDLK_SPACE)
		{
			m_CanRun = false;
		}
		break;
	}
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	UpdateBlackboard(dt);
	m_pBehaviorTree->Update();

	return GetSteering();
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });

	DrawBlackboardInfo();
}

void Plugin::InitPipeline()
{
	m_pTargeter = new FixedGoalTargeter();
	m_pConstraint = new AvoidSpheresConstraint({});

	m_pSeekBehavior = new Seek();
	m_pActuator = new BasicActuator(m_pSeekBehavior);

	m_pWanderBehavior = new Wander();
	m_pFaceBehavior = new Face();

	m_pSteeringPipeline = new SteeringPipeline(m_pInterface);
	m_pSteeringPipeline->SetTargeters({ m_pTargeter });
	m_pSteeringPipeline->SetConstraints({ m_pConstraint });
	m_pSteeringPipeline->SetActuator(m_pActuator);
	m_pSteeringPipeline->SetFallBack(m_pWanderBehavior);
}

void Plugin::InitBehaviorTree()
{
	Blackboard* pBlackboard = new Blackboard();
	SetBlackboardData(pBlackboard);

	//Create tree
	m_pBehaviorTree = new BehaviorTree(pBlackboard, 
		new BehaviorList({
			//Enemy checks
			new BehaviorSelector({
				new BehaviorSequence({
					new BehaviorConditional(HasEnemiesInFOV),
					new BehaviorAction(UpdateEnemies),
					new BehaviorAction(UpdateClosestEnemy),
					new BehaviorAction(ResetEnemyTimer),
					new BehaviorAction(DodgeEnemies)
				}),
				new BehaviorSequence({
					new BehaviorConditional(HasSeenEnemies),
					new BehaviorAction(UpdateEnemyTimer),
					new BehaviorSequence({
						new BehaviorConditional(HasEnemyTimerExceeded),
						new BehaviorAction(ForgetEnemies),
						new BehaviorAction(ResetEnemyTimer)
					})
				}),
				new BehaviorSequence({
					new BehaviorConditional(HasSeenEnemies),
					new BehaviorConditional(HasLostEnemies),
					new BehaviorAction(ForgetEnemies)
				}),
				new BehaviorSequence({
					new BehaviorConditional(HasNoEnemiesInFOV),
					new BehaviorAction(ClearClosestEnemy)
				})
			}),
			//House behavior
			new BehaviorSelector({
				//Searching house
				new BehaviorSequence({
					new BehaviorConditional(IsHouseStateSearching),
					new BehaviorConditional(HasRoomInInventory),
					new BehaviorConditional(HasHouseInFOV),
					new BehaviorConditional(HasNotVisitedHouse),
					new BehaviorAction(UpdateTargetHouse),
					new BehaviorAction(ToHouseGoingState)
				}),
				//Going to house
				new BehaviorSequence({
					new BehaviorConditional(IsHouseStateGoing),
					new BehaviorAction(SetTargetFromHouse),
					new BehaviorSequence({
						new BehaviorConditional(IsInsideHouse),
						new BehaviorAction(ToHouseInsideState),
						new BehaviorAction(SetHouseAsVisited)
					})
				}),
				//Inside house
				new BehaviorSequence({
					new BehaviorConditional(IsHouseStateInside),
					new BehaviorList({
						new BehaviorSelector({
							new BehaviorSequence({
								new BehaviorConditional(HasNotInspectedHouse),
								new BehaviorAction(SetTargetFromHouse),
								new BehaviorSequence({
									 new BehaviorConditional(IsInCenterOfHouse),
									 new BehaviorAction(SetHouseInspected)
								})
							})
						}),
						//Looting
						new BehaviorSelector({
							new BehaviorSequence({
								new BehaviorConditional(HasItemsInFOV),
								new BehaviorConditional(IsTargetItemNotSet),
								new BehaviorConditional(HasRoomInInventory),
								new BehaviorAction(UpdateTargetItem)
							}),
							new BehaviorSequence({
								new BehaviorConditional(IsTargetItemSet),
								new BehaviorConditional(FarFromTargetItem),
								new BehaviorAction(GoToItem)
							}),
							new BehaviorSequence({
								new BehaviorConditional(IsTargetItemSet),
								new BehaviorConditional(NearTargetItem),
								new BehaviorAction(GrabItem),
								new BehaviorAction(ResetTargetItem)
							})
						}),
						//Leave house
						new BehaviorSequence({
							new BehaviorConditional(IsOutsideHouse),
							new BehaviorAction(ClearTargetHouse),
							new BehaviorAction(ToHouseSearchingState),
							new BehaviorAction(ResetHouseInspected)
						})
					})
				})
			}),
			//Item management
			new BehaviorList({
				new BehaviorAction(EatFood),
				new BehaviorAction(UseMedkit)
			}),
			//Combat
			new BehaviorList({
				new BehaviorSequence({
					new BehaviorConditional(HasEnemiesInFOV),
					new BehaviorAction(SetFaceTarget),
					new BehaviorAction(StartAiming),
					new BehaviorSequence({
						new BehaviorConditional(HasTargetInSight),
						new BehaviorConditional(HasWeapon),
						new BehaviorAction(Shoot),
						new BehaviorAction(RemoveEmptyWeapons)
					})
				}),
				new BehaviorSequence({
					new BehaviorConditional(HasNoEnemiesInFOV),
					new BehaviorAction(StopAiming)
				})
			}),
			//Navigation
			new BehaviorList({
				new BehaviorAction(SetTargeterGoal),
				new BehaviorAction(CalculateSteering)
			})
		})
	);
}

void Plugin::SetBlackboardData(Blackboard* pBlackboard)
{
	//Agent
	pBlackboard->AddData<AgentInfo>("Agent", {});

	//Navigation
	pBlackboard->AddData<SteeringPlugin_Output>("Steering", {});
	pBlackboard->AddData("MoveTarget", ZeroVector2);
	pBlackboard->AddData("Pipeline", m_pSteeringPipeline);
	pBlackboard->AddData("Targeter", m_pTargeter);
	pBlackboard->AddData("FaceBehavior", m_pFaceBehavior);

	//Enemies
	pBlackboard->AddData<vector<EnemyInfo>>("Enemies", {});
	pBlackboard->AddData("EnemyLocation", ZeroVector2);
	pBlackboard->AddData("HasSeenEnemies", false);
	pBlackboard->AddData("MaxEnemyDistance", 15.0f);
	pBlackboard->AddData("DodgeRadius", 5.0f);
	pBlackboard->AddData("Constraint", m_pConstraint);
	pBlackboard->AddData<EnemyInfo>("ClosestEnemy", {});
	pBlackboard->AddData("EnemyTimer", 0.0f);
	pBlackboard->AddData("MaxEnemyTime", 10.0f);

	//Houses
	pBlackboard->AddData("HouseState", HouseState::Searching);
	pBlackboard->AddData<HouseInfo>("NearbyHouse", {});
	pBlackboard->AddData<HouseInfo>("TargetHouse", {});
	pBlackboard->AddData<deque<HouseInfo>>("VisitedHouses", {});
	pBlackboard->AddData("MaxHouseVisited", 3);
	pBlackboard->AddData("HasInspectedHouse", false);
	pBlackboard->AddData("HouseInspectionRadius", 3.0f);

	//Items
	UINT capacity = m_pInterface->Inventory_GetCapacity();
	vector<bool> slots(capacity, false);
	pBlackboard->AddData<vector<EntityInfo>>("Items", {});
	pBlackboard->AddData<EntityInfo>("TargetItem", {});
	pBlackboard->AddData("InventorySlots", slots);
	pBlackboard->AddData("MaxNumberFood", 2);
	pBlackboard->AddData("MaxNumberMedkits", 2);

	//Combat
	pBlackboard->AddData("MaxNumberPistols", 3);
	pBlackboard->AddData("IsAiming", false);
	pBlackboard->AddData("MinAimDP", 0.995f);

	//Misc
	pBlackboard->AddData("Interface", m_pInterface);
	pBlackboard->AddData("DeltaTime", 0.0f);
	pBlackboard->AddData("WeaponSelector", m_pWeaponSelector);
}

SteeringPlugin_Output Plugin::GetSteering()
{
	SteeringPlugin_Output steering = {};

	Blackboard* pBlackboard = m_pBehaviorTree->GetBlackboard();
	if (pBlackboard)
		pBlackboard->GetData("Steering", steering);

	return steering;
}

void Plugin::UpdateBlackboard(float dt)
{
	Blackboard* pBlackboard = m_pBehaviorTree->GetBlackboard();
	if (pBlackboard) {
		//Agent
		AgentInfo agent = m_pInterface->Agent_GetInfo();
		pBlackboard->ChangeData("Agent", agent);

		//Target
		Vector2 nextLocation = m_pInterface->World_GetCheckpointLocation();
		//nextLocation = m_Target;
		pBlackboard->ChangeData("MoveTarget", nextLocation);

		UpdateEntitiesInFOV(pBlackboard);
		UpdateNearestHouse(pBlackboard, agent);
		UpdateWeaponSelector(pBlackboard);

		pBlackboard->ChangeData("DeltaTime", dt);
	}
}

void Plugin::UpdateEntitiesInFOV(Blackboard* pBlackboard)
{
	vector<EnemyInfo> enemies{};
	vector<EntityInfo> items{};

	for (const EntityInfo& entity : GetEntitiesInFOV()) {
		if (entity.Type == eEntityType::ENEMY) {
			EnemyInfo enemy = {};
			if (m_pInterface->Enemy_GetInfo(entity, enemy))
				enemies.push_back(enemy);
		}

		if (entity.Type == eEntityType::ITEM)
			items.push_back(entity);
	}

	pBlackboard->ChangeData("Enemies", enemies);
	pBlackboard->ChangeData("Items", items);
}

void Plugin::UpdateNearestHouse(Blackboard* pBlackboard, const AgentInfo& agent)
{
	vector<HouseInfo> houses = GetHousesInFOV();

	if (houses.size() > 0) {
		auto it = min_element(houses.begin(), houses.end(), [agent](const HouseInfo& left, const HouseInfo& right) {
			return Distance(left.Center, agent.Position) < Distance(right.Center, agent.Position);
		});

		pBlackboard->ChangeData("NearbyHouse", *it);
	} else
		pBlackboard->ChangeData("NearbyHouse", HouseInfo());
}

void Plugin::UpdateWeaponSelector(Blackboard* pBlackboard)
{
	AgentInfo agent = {};
	EnemyInfo enemy = {};
	bool foundData = pBlackboard->GetData("Agent", agent) &&
					 pBlackboard->GetData("ClosestEnemy", enemy);

	if (!foundData)
		return;

	m_pWeaponSelector->Update(agent, enemy);
}

void Plugin::DrawBlackboardInfo() const
{
	Blackboard* pBlackboard = m_pBehaviorTree->GetBlackboard();
	if (pBlackboard) {
		Vector2 enemyPos = ZeroVector2;
		bool hasSeen = false;
		float dodgeRadius = 0.0f;
		bool hasInspected = false;
		float inspectionRadius = 0.0f;
		HouseInfo currentHouse = {};
		AgentInfo agent = {};
		EnemyInfo closestEnemy = {};
		float minDP = 0.0f;

		bool foundData = pBlackboard->GetData("EnemyLocation", enemyPos) &&
						 pBlackboard->GetData("HasSeenEnemies", hasSeen) &&
					     pBlackboard->GetData("DodgeRadius", dodgeRadius) &&
						 pBlackboard->GetData("HasInspectedHouse", hasInspected) &&
						 pBlackboard->GetData("HouseInspectionRadius", inspectionRadius) &&
						 pBlackboard->GetData("TargetHouse", currentHouse) &&
						 pBlackboard->GetData("Agent", agent) &&
						 pBlackboard->GetData("ClosestEnemy", closestEnemy) &&
						 pBlackboard->GetData("MinAimDP", minDP);

		if (!foundData)
			return;

		if (hasSeen)
			m_pInterface->Draw_Circle(enemyPos, dodgeRadius, { 1, 0, 0 });

		if (!hasInspected && agent.IsInHouse)
			m_pInterface->Draw_Circle(currentHouse.Center, inspectionRadius, { 1, 0, 1 });

		if (closestEnemy.Size > 0.0f) {
			m_pInterface->Draw_Circle(closestEnemy.Location, 2.0f, { 0, 0, 0 });

			Vector2 agentForward = OrientationToVector(agent.Orientation);
			Vector2 toEnemy = closestEnemy.Location - agent.Position;

			if (Dot(agentForward.GetNormalized(), toEnemy.GetNormalized()) >= minDP)
				m_pInterface->Draw_Direction(agent.Position, agentForward, 20.0f, { 1, 1, 1 }, 0.0f);
		}
	}
}

#pragma region Helper Functions
vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}
#pragma endregion