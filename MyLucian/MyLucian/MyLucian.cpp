#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <sstream>
#include <map>
#include <math.h>       /* acos */
#include <string>
#include <vector>
#define PI 3.14159265

PluginSetup("MyLucian")

IMenu* MainMenu;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboQExt;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ESmart;
IMenuOption* AutoR;

IMenu* HarassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassQExt;
IMenuOption* HarassW;
IMenuOption* HarassE;

IMenu* FarmMenu;
IMenuOption* FarmQ;
IMenuOption* FarmW;
IMenuOption* FarmE;

IMenu* KSMenu;
IMenuOption* KSQ;
IMenuOption* KSW;
IMenuOption* KSE;
IMenuOption* KSR;

IMenu* ExtraMenu;
IMenuOption* AutoQExt;
IMenuOption* EGap;
IMenuOption* LockR;
IMenuOption* RSemi;
IMenuOption* EModeChange;
IMenuOption* EMode;
std::map<int, IMenuOption*> GapTarget;

IMenu* RenderMenu;
IMenuOption* DrawReady;
IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawR;
IMenuOption* DrawPriority;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
IInventoryItem* Tiamat;
IInventoryItem* Titanic_Hydra;
IInventoryItem* Ravenous_Hydra;
IInventoryItem* Youmuus;

IUnit* closestTurret;

std::vector<std::string> const& Names = { "Safe", "Risky"};
std::vector<std::string> const& ENames = { "Fast", "Slow" };

Vec3 direction;
Vec3 movePos;
int timer = 0;
bool checker = false;

int ComboMode = 1;
float KeyPre = 0.f;
bool inkeep = false;
int keeptimer;
float distances;
Vec3 choser;

void  DrawMenu()
{
	MainMenu = GPluginSDK->AddMenu("A-Series Lucian");

	ComboMenu = MainMenu->AddMenu("Combo");
	ComboQ = ComboMenu->CheckBox("Use Q Combo", true);
	ComboQExt = ComboMenu->CheckBox("Use Q Extend Combo", true);
	ComboW = ComboMenu->CheckBox("Use W Combo", true);
	ComboE = ComboMenu->CheckBox("Use E Combo", true);
	ESmart = ComboMenu->CheckBox("Smart E", true);
	AutoR = ComboMenu->CheckBox("Auto R if killable", true);

	HarassMenu = MainMenu->AddMenu("Harrass");
	HarassQ = HarassMenu->CheckBox("Use Q Harrass", true);
	HarassQExt = HarassMenu->CheckBox("Use Q Extend Harrass", true);
	HarassW = HarassMenu->CheckBox("Use W Harrass", true);
	HarassE = HarassMenu->CheckBox("Use E Harrass", true);

	FarmMenu = MainMenu->AddMenu("Farm");
	FarmQ = FarmMenu->CheckBox("Use Q Farm", true);
	FarmW = FarmMenu->CheckBox("Use W Farm", true);
	FarmE = FarmMenu->CheckBox("Use E Farm", true);

	KSMenu = MainMenu->AddMenu("KS");
	KSQ = KSMenu->CheckBox("Use Q KS", true);
	KSW = KSMenu->CheckBox("Use W KS", true);
	KSE = KSMenu->CheckBox("Use E to Gap Close in KS", true);
	KSR = KSMenu->CheckBox("Use R KS", true);

	ExtraMenu = MainMenu->AddMenu("Extra");
	AutoQExt = ExtraMenu->CheckBox("Auto Q Extend", true);
	LockR = ExtraMenu->CheckBox("Lock on R target", true);
	RSemi = ExtraMenu->AddKey("Semi Manual R", 'T');
	EModeChange = ExtraMenu->AddKey("Change E Mode Key", 'U');
	EMode = ExtraMenu->AddSelection("E Mode", 1, Names);
	for (auto target : GEntityList->GetAllHeros(false, true))
	{
		if (target != nullptr)
		{
			std::string GapMenu = "Anti Gapcloser if enemy Champion: " + std::string(target->ChampionName());
			GapTarget[target->GetNetworkId()] = ExtraMenu->CheckBox(GapMenu.c_str(), target->IsMelee());
		}
	}

	RenderMenu = MainMenu->AddMenu("Drawing Settings");
	DrawReady = RenderMenu->CheckBox("Draw Only Ready Spells", true);
	DrawQ = RenderMenu->CheckBox("Draw Q", true);
	DrawW = RenderMenu->CheckBox("Draw W", true);
	DrawE = RenderMenu->CheckBox("Draw E", true);
	DrawR = RenderMenu->CheckBox("Draw R", true);
	DrawPriority = RenderMenu->CheckBox("Draw QMode", true);
}

void  LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, kCollidesWithYasuoWall);
	Q->SetOverrideRange(GEntityList->Player()->AttackRange() + 70);
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, true, false, kCollidesWithYasuoWall);
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, false, kCollidesWithNothing);
	R = GPluginSDK->CreateSpell2(kSlotR, kLineCast, false, false, kCollidesWithYasuoWall);

	Titanic_Hydra = GPluginSDK->CreateItemForId(3748, 385);
	Ravenous_Hydra = GPluginSDK->CreateItemForId(3074, 385);
	Tiamat = GPluginSDK->CreateItemForId(3077, 385);
	Youmuus = GPluginSDK->CreateItemForId(3142, 0);
}

int EnemiesInRange(float range, Vec3 Source)
{
	auto Targets = GEntityList->GetAllHeros(false, true);
	auto player = GEntityList->Player();
	auto enemiesInRange = 0;

	for (auto target : Targets)
	{
		if (target != nullptr && !target->IsDead() && player->IsValidTarget(target, range) && target->IsHero())
		{
			auto flDistance = (target->GetPosition() - Source).Length();
			if (flDistance < range)
			{
				enemiesInRange++;
			}
		}
	}
	return enemiesInRange;
}

int EnemiesInRange(IUnit* Source, float range)
{
	auto Targets = GEntityList->GetAllHeros(false, true);
	auto enemiesInRange = 0;

	for (auto target : Targets)
	{
		if (target != nullptr && !target->IsDead())
		{
			auto flDistance = (target->GetPosition() - Source->GetPosition()).Length();
			if (flDistance < range)
			{
				enemiesInRange++;
			}
		}
	}
	return enemiesInRange;
}

bool LineEquation(IUnit* enemy, IUnit* player, float distance) // distance from enemy
{
	bool wall;
	Vec3 epos = enemy->GetPosition();
	Vec3 ppos = player->GetPosition();
	Vec2 epo = ToVec2(epos);
	Vec2 ppo = ToVec2(ppos);

	float x1 = ppo.x;
	float y1 = ppo.y;
	float x2 = epo.x;
	float y2 = epo.y;

	float m = (y2 - y1) / (x2 - x1);
	float c = y1 - m*x1;
	Vec3 pos = Extend(enemy->GetPosition(), player->GetPosition(), -distance);
	Vec2 checkPos;

	if (pos.x > x2)
	{
		for (int i = x2; i <= pos.x; i++)
		{
			y2 = m*i + c;
			checkPos.Set(i, y2);
			Vec3 check = ToVec3(checkPos);
			auto flags = GNavMesh->GetCollisionFlagsForPoint(check);
			if (flags == kBuildingMesh || flags == kWallMesh)
			{
				wall = true;
				break;
			}
			else
			{
				wall = false;
			}
		}
	}
	if (pos.x < x2)
	{
		for (int i = x2; i >= pos.x; i--)
		{
			y2 = m*i + c;
			checkPos.Set(i, y2);
			Vec3 check = ToVec3(checkPos);
			auto flags = GNavMesh->GetCollisionFlagsForPoint(check);
			if (flags == kBuildingMesh || flags == kWallMesh)
			{
				wall = true;
				break;
			}
			else
			{
				wall = false;
			}
		}
	}
	return wall;
}

bool buildingCheck(IUnit* enemy, IUnit* player, float distance)
{
	auto collision = false;
	Vec3 porte = Extend(enemy->GetPosition(), player->GetPosition(), -distance);
	Vec2 from = ToVec2(enemy->GetPosition());
	Vec2 to = ToVec2(porte);
	float m = ((to.y - from.y) / (to.x - from.x));
	float X;
	float Y;
	float m2 = (-(to.x - from.x) / (to.y - from.y));
	auto minions = GEntityList->GetAllTurrets(true, true);
	auto heros = GEntityList->GetAllInhibitors(true, true);
	auto mnexus = GEntityList->GetTeamNexus();
	auto enexus = GEntityList->GetEnemyNexus();
	for (auto minion : minions)
	{
		Vec3 minionP = minion->GetPosition();
		Vec2 minionPos = ToVec2(minionP);
		float px = minionPos.x;
		float py = minionPos.y;
		X = ((m2*px) - (from.x*m) + (from.y - py)) / (m2 - m);
		Y = m * (X - from.x) + from.y;
		Vec2 colliPos;
		colliPos.Set(X, Y);
		if (Distance(colliPos, minionPos) <= enemy->BoundingRadius() + minion->BoundingRadius() - 10 && Distance(colliPos, from) <= distance && Distance(from, minionPos) < Distance(ToVec2(player->GetPosition()), minionPos))
		{
			collision = true;
			break;
		}
	}
	for (auto hero : heros)
	{
		Vec3 heroP;
		heroP = hero->GetPosition();
		Vec2 heroPos = ToVec2(heroP);
		float herox = heroPos.x;
		float heroy = heroPos.y;
		X = ((m2*herox) - (from.x*m) + (from.y - heroy)) / (m2 - m);
		Y = m * (X - from.x) + from.y;
		Vec2 colliPos;
		colliPos.Set(X, Y);
		if (Distance(colliPos, heroPos) <= enemy->BoundingRadius() + hero->BoundingRadius() - 10 && Distance(colliPos, from) <= distance && Distance(from, heroPos) < Distance(ToVec2(player->GetPosition()), heroPos))
		{
			collision = true;
			break;
		}
	}
	Vec3 mnexusP;
	mnexusP = mnexus->GetPosition();
	Vec2 mnexusPos = ToVec2(mnexusP);
	float mnexusx = mnexusPos.x;
	float mnexusy = mnexusPos.y;
	X = ((m2*mnexusx) - (from.x*m) + (from.y - mnexusy)) / (m2 - m);
	Y = m * (X - from.x) + from.y;
	Vec2 colliPos;
	colliPos.Set(X, Y);
	if (Distance(colliPos, mnexusPos) <= enemy->BoundingRadius() + mnexus->BoundingRadius() - 50 && Distance(colliPos, from) <= distance && Distance(from, mnexusPos) < Distance(ToVec2(player->GetPosition()), mnexusPos))
	{
		collision = true;
	}
	Vec3 enexusP;
	enexusP = enexus->GetPosition();
	Vec2 enexusPos = ToVec2(enexusP);
	float enexusx = enexusPos.x;
	float enexusy = enexusPos.y;
	X = ((m2*enexusx) - (from.x*m) + (from.y - enexusy)) / (m2 - m);
	Y = m * (X - from.x) + from.y;
	Vec2 colliPos1;
	colliPos1.Set(X, Y);
	if (Distance(colliPos1, enexusPos) <= enemy->BoundingRadius() + enexus->BoundingRadius() - 50 && Distance(colliPos1, from) <= distance && Distance(from, enexusPos) < Distance(ToVec2(player->GetPosition()), enexusPos))
	{
		collision = true;
	}
	return collision;
}

Vec3 LineChecker(IUnit* enemy, IUnit* player, float distance, Vec3 che1, Vec3 che2)
{
	auto collision = false;
	Vec3 porte = Extend(player->GetPosition(), enemy->GetPosition(), -distance);
	Vec2 from = ToVec2(enemy->GetPosition());
	Vec2 to = ToVec2(porte);
	float m = ((to.y - from.y) / (to.x - from.x));
	float X1;
	float Y1;
	float X2;
	float Y2;
	float m2 = (-(to.x - from.x) / (to.y - from.y));

	Vec2 minionPos = ToVec2(che1);
	float px = minionPos.x;
	float py = minionPos.y;
	X1 = ((m2*px) - (from.x*m) + (from.y - py)) / (m2 - m);
	Y1 = m * (X1 - from.x) + from.y;
	Vec2 colliPos1;
	colliPos1.Set(X1, Y1);


	Vec2 minionPos2 = ToVec2(che2);
	float px2 = minionPos2.x;
	float py2 = minionPos2.y;
	X2 = ((m2*px2) - (from.x*m) + (from.y - py2)) / (m2 - m);
	Y2 = m * (X2 - from.x) + from.y;
	Vec2 colliPos2;
	colliPos2.Set(X2, Y2);
	if (Distance(colliPos1, minionPos) <= Distance(colliPos2, minionPos2))
	{
		return che1;
	}
	if (Distance(colliPos2, minionPos2) <= Distance(colliPos1, minionPos))
	{
		return che2;
	}
}

bool buildingChecks(IUnit* enemy, Vec3 playerPosi, IUnit* player, float distance)
{
	auto collision = false;
	Vec3 porte = Extend(enemy->GetPosition(), playerPosi, -distance);
	Vec2 from = ToVec2(enemy->GetPosition());
	Vec2 to = ToVec2(porte);
	float m = ((to.y - from.y) / (to.x - from.x));
	float X;
	float Y;
	float m2 = (-(to.x - from.x) / (to.y - from.y));
	auto minions = GEntityList->GetAllTurrets(true, true);
	auto heros = GEntityList->GetAllInhibitors(true, true);
	auto mnexus = GEntityList->GetTeamNexus();
	auto enexus = GEntityList->GetEnemyNexus();
	for (auto minion : minions)
	{
		Vec3 minionP = minion->GetPosition();
		Vec2 minionPos = ToVec2(minionP);
		float px = minionPos.x;
		float py = minionPos.y;
		X = ((m2*px) - (from.x*m) + (from.y - py)) / (m2 - m);
		Y = m * (X - from.x) + from.y;
		Vec2 colliPos;
		colliPos.Set(X, Y);
		if (Distance(colliPos, minionPos) <= enemy->BoundingRadius() + minion->BoundingRadius() - 100 && Distance(colliPos, from) <= distance && Distance(from, minionPos) < Distance(ToVec2(playerPosi), minionPos))
		{
			collision = true;
			break;
		}
	}
	for (auto hero : heros)
	{
		Vec3 heroP;
		heroP = hero->GetPosition();
		Vec2 heroPos = ToVec2(heroP);
		float herox = heroPos.x;
		float heroy = heroPos.y;
		X = ((m2*herox) - (from.x*m) + (from.y - heroy)) / (m2 - m);
		Y = m * (X - from.x) + from.y;
		Vec2 colliPos;
		colliPos.Set(X, Y);
		if (Distance(colliPos, heroPos) <= enemy->BoundingRadius() + hero->BoundingRadius() - 100 && Distance(colliPos, from) <= distance && Distance(from, heroPos) < Distance(ToVec2(playerPosi), heroPos))
		{
			collision = true;
			break;
		}
	}
	Vec3 mnexusP;
	mnexusP = mnexus->GetPosition();
	Vec2 mnexusPos = ToVec2(mnexusP);
	float mnexusx = mnexusPos.x;
	float mnexusy = mnexusPos.y;
	X = ((m2*mnexusx) - (from.x*m) + (from.y - mnexusy)) / (m2 - m);
	Y = m * (X - from.x) + from.y;
	Vec2 colliPos;
	colliPos.Set(X, Y);
	if (Distance(colliPos, mnexusPos) <= enemy->BoundingRadius() + mnexus->BoundingRadius() - 100 && Distance(colliPos, from) <= distance && Distance(from, mnexusPos) < Distance(ToVec2(playerPosi), mnexusPos))
	{
		collision = true;
	}
	Vec3 enexusP;
	enexusP = enexus->GetPosition();
	Vec2 enexusPos = ToVec2(enexusP);
	float enexusx = enexusPos.x;
	float enexusy = enexusPos.y;
	X = ((m2*enexusx) - (from.x*m) + (from.y - enexusy)) / (m2 - m);
	Y = m * (X - from.x) + from.y;
	Vec2 colliPos1;
	colliPos1.Set(X, Y);
	if (Distance(colliPos1, enexusPos) <= enemy->BoundingRadius() + enexus->BoundingRadius() - 100 && Distance(colliPos1, from) <= distance && Distance(from, enexusPos) < Distance(ToVec2(playerPosi), enexusPos))
	{
		collision = true;
	}
	return collision;
}

bool LineEquations(IUnit* enemy, Vec3 playerPosi, IUnit* player, float distance) // push distance from enemy
{
	bool wall;
	Vec3 epos = enemy->GetPosition();
	Vec3 ppos = playerPosi;
	Vec2 epo = ToVec2(epos);
	Vec2 ppo = ToVec2(ppos);

	float x1 = ppo.x;
	float y1 = ppo.y;
	float x2 = epo.x;
	float y2 = epo.y;

	float m = (y2 - y1) / (x2 - x1);
	float c = y1 - m*x1;
	Vec3 pos = Extend(enemy->GetPosition(), playerPosi, -distance);
	Vec2 checkPos;

	if (pos.x > x2)
	{
		for (int i = x2; i <= pos.x; i++)
		{
			y2 = m*i + c;
			checkPos.Set(i, y2);
			Vec3 check = ToVec3(checkPos);
			auto flags = GNavMesh->GetCollisionFlagsForPoint(check);
			if (flags == kBuildingMesh || flags == kWallMesh)
			{
				wall = true;
				break;
			}
			else
			{
				wall = false;
			}
		}
	}
	if (pos.x < x2)
	{
		for (int i = x2; i >= pos.x; i--)
		{
			y2 = m*i + c;
			checkPos.Set(i, y2);
			Vec3 check = ToVec3(checkPos);
			auto flags = GNavMesh->GetCollisionFlagsForPoint(check);
			if (flags == kBuildingMesh || flags == kWallMesh)
			{
				wall = true;
				break;
			}
			else
			{
				wall = false;
			}
		}
	}
	return wall;
}

bool LineEquations1(IUnit* player, Vec3 enemyPosi, float distance)
{
	bool wall;
	Vec3 epos = enemyPosi;
	Vec3 ppos = player->GetPosition();
	Vec2 epo = ToVec2(epos);
	Vec2 ppo = ToVec2(ppos);

	float x1 = ppo.x;
	float y1 = ppo.y;
	float x2 = epo.x;
	float y2 = epo.y;

	float m = (y2 - y1) / (x2 - x1);
	float c = y1 - m*x1;
	Vec3 pos = Extend(enemyPosi, player->GetPosition(), -distance);
	Vec2 checkPos;

	if (pos.x > x2)
	{
		for (int i = x2; i <= pos.x; i++)
		{
			y2 = m*i + c;
			checkPos.Set(i, y2);
			Vec3 check = ToVec3(checkPos);
			auto flags = GNavMesh->GetCollisionFlagsForPoint(check);
			if (flags == kBuildingMesh || flags == kWallMesh)
			{
				wall = true;
				break;
			}
			else
			{
				wall = false;
			}
		}
	}
	if (pos.x < x2)
	{
		for (int i = x2; i >= pos.x; i--)
		{
			y2 = m*i + c;
			checkPos.Set(i, y2);
			Vec3 check = ToVec3(checkPos);
			auto flags = GNavMesh->GetCollisionFlagsForPoint(check);
			if (flags == kBuildingMesh || flags == kWallMesh)
			{
				wall = true;
				break;
			}
			else
			{
				wall = false;
			}
		}
	}
	return wall;
}

bool buildingCheck1(IUnit* enemy, Vec3 enemyPosi, IUnit* player, float distance)
{
	auto collision = false;
	Vec3 porte = Extend(enemyPosi, player->GetPosition(), -distance);
	Vec2 from = ToVec2(enemyPosi);
	Vec2 to = ToVec2(porte);
	float m = ((to.y - from.y) / (to.x - from.x));
	float X;
	float Y;
	float m2 = (-(to.x - from.x) / (to.y - from.y));
	auto minions = GEntityList->GetAllTurrets(true, true);
	auto heros = GEntityList->GetAllInhibitors(true, true);
	auto mnexus = GEntityList->GetTeamNexus();
	auto enexus = GEntityList->GetEnemyNexus();
	for (auto minion : minions)
	{
		Vec3 minionP = minion->GetPosition();
		Vec2 minionPos = ToVec2(minionP);
		float px = minionPos.x;
		float py = minionPos.y;
		X = ((m2*px) - (from.x*m) + (from.y - py)) / (m2 - m);
		Y = m * (X - from.x) + from.y;
		Vec2 colliPos;
		colliPos.Set(X, Y);
		if (Distance(colliPos, minionPos) <= enemy->BoundingRadius() + minion->BoundingRadius() - 10 && Distance(colliPos, from) <= distance && Distance(from, minionPos) < Distance(ToVec2(player->GetPosition()), minionPos))
		{
			collision = true;
			break;
		}
	}
	for (auto hero : heros)
	{
		Vec3 heroP;
		heroP = hero->GetPosition();
		Vec2 heroPos = ToVec2(heroP);
		float herox = heroPos.x;
		float heroy = heroPos.y;
		X = ((m2*herox) - (from.x*m) + (from.y - heroy)) / (m2 - m);
		Y = m * (X - from.x) + from.y;
		Vec2 colliPos;
		colliPos.Set(X, Y);
		if (Distance(colliPos, heroPos) <= enemy->BoundingRadius() + hero->BoundingRadius() - 10 && Distance(colliPos, from) <= distance && Distance(from, heroPos) < Distance(ToVec2(player->GetPosition()), heroPos))
		{
			collision = true;
			break;
		}
	}
	Vec3 mnexusP;
	mnexusP = mnexus->GetPosition();
	Vec2 mnexusPos = ToVec2(mnexusP);
	float mnexusx = mnexusPos.x;
	float mnexusy = mnexusPos.y;
	X = ((m2*mnexusx) - (from.x*m) + (from.y - mnexusy)) / (m2 - m);
	Y = m * (X - from.x) + from.y;
	Vec2 colliPos;
	colliPos.Set(X, Y);
	if (Distance(colliPos, mnexusPos) <= enemy->BoundingRadius() + mnexus->BoundingRadius() - 50 && Distance(colliPos, from) <= distance && Distance(from, mnexusPos) < Distance(ToVec2(player->GetPosition()), mnexusPos))
	{
		collision = true;
	}
	Vec3 enexusP;
	enexusP = enexus->GetPosition();
	Vec2 enexusPos = ToVec2(enexusP);
	float enexusx = enexusPos.x;
	float enexusy = enexusPos.y;
	X = ((m2*enexusx) - (from.x*m) + (from.y - enexusy)) / (m2 - m);
	Y = m * (X - from.x) + from.y;
	Vec2 colliPos1;
	colliPos1.Set(X, Y);
	if (Distance(colliPos1, enexusPos) <= enemy->BoundingRadius() + enexus->BoundingRadius() - 50 && Distance(colliPos1, from) <= distance && Distance(from, enexusPos) < Distance(ToVec2(player->GetPosition()), enexusPos))
	{
		collision = true;
	}
	return collision;
}

inline static int CountMinionsNearMe(IUnit* Source, float range)
{
	auto minion = GEntityList->GetAllMinions(false, true, true);
	auto count = 0;

	for (auto unit : minion)
	{
		if (unit->IsValidTarget() && !unit->IsDead() && (unit->IsCreep() || unit->IsJungleCreep()))
		{
			auto flDistance = (unit->GetPosition() - Source->GetPosition()).Length();
			if (flDistance < range)
			{
				count++;
			}
		}
	}

	return count;
}

inline int ChangePriority()
{
	if (GetAsyncKeyState(EModeChange->GetInteger()) && !GGame->IsChatOpen() && GGame->Time() > KeyPre)
	{
		if (ComboMode == 1)
		{
			ComboMode = 0;
			KeyPre = GGame->Time() + 0.250;
		}
		else
		{
			ComboMode = 1;
			KeyPre = GGame->Time() + 0.250;
		}
		return ComboMode;
	}
}

Vec3 SmartELogic()
{
	IUnit* temporar = GEntityList->GetEnemyNexus();
	if (ComboMode == 0)
	{
		auto player = GEntityList->Player();
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range() + player->AttackRange());
		auto enemySafe = GTargetSelector->FindTarget(ClosestPriority, PhysicalDamage, E->Range() + player->AttackRange());
		auto nexus = GEntityList->GetTeamNexus();
		auto turrets = GEntityList->GetAllTurrets(true, false);

		for (int m = 0; m < turrets.size(); m++)
		{
			if (Distance(turrets[m], GEntityList->Player()) < Distance(temporar, GEntityList->Player()) && !turrets[m]->IsDead())
			{
				temporar = turrets[m];
				closestTurret = turrets[m];
			}
		}

		if (enemySafe != nullptr && enemySafe->IsValidTarget() && enemySafe->IsHero() && ComboE->Enabled() && ESmart->Enabled() && E->IsReady())
		{
			if (EnemiesInRange(1500, player->GetPosition()) > 2)
			{
				if (EnemiesInRange(W->Range(), player->GetPosition()) >= 2)
				{
					std::vector<Vec3> temp;
					for (int i = 5; i < 360; i += 5)
					{
						Vec3 posteQ1;
						Vec3 poseQ1;
						posteQ1 = RotateAround(enemySafe->GetPosition(), player->GetPosition(), i);
						poseQ1 = Extend(player->GetPosition(), posteQ1, E->Range()/3);
						if (Distance(poseQ1, enemySafe->GetPosition()) >= player->AttackRange() - 150 && Distance(poseQ1, enemySafe->GetPosition()) <= player->AttackRange() - 100 && !GNavMesh->IsPointWall(poseQ1))
						{
							temp.push_back(poseQ1);
						}
					}
					if (temp.size() == 0 && Distance(player->GetPosition(), enemySafe->GetPosition()) < 300)
					{
						return Extend(player->GetPosition(), enemySafe->GetPosition(), -450);
					}
					else if (temp.size() == 0 && Distance(player->GetPosition(), enemySafe->GetPosition()) > 300)
					{
						return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
					}
					
					Vec3 tem = GEntityList->GetEnemyNexus()->GetPosition();
					for (int y = 0; y < temp.size(); y++)
					{
						if (closestTurret != nullptr && Distance(tem, closestTurret->GetPosition()) > Distance(temp[y], closestTurret->GetPosition()))
						{
							tem = temp[y];
						}
					}
					return tem;
				}
				else
					return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
			}
			else
				return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
		}
		else if (ComboE->Enabled() && E->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && !ESmart->Enabled())
		{
			return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
		}
	}
	
	if (ComboMode == 1)
	{
		auto player = GEntityList->Player();
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range() + player->AttackRange());
		auto nexus = GEntityList->GetTeamNexus();
		auto turrets = GEntityList->GetAllTurrets(true, false);

		for (int m = 0; m < turrets.size(); m++)
		{
			if (Distance(turrets[m], GEntityList->Player()) < Distance(temporar, GEntityList->Player()) && !turrets[m]->IsDead())
			{
				temporar = turrets[m];
				closestTurret = turrets[m];
			}
		}

		if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero() && ComboE->Enabled() && ESmart->Enabled() && E->IsReady())
		{
			if (EnemiesInRange(1500, player->GetPosition()) > 2)
			{
				if (EnemiesInRange(W->Range(), player->GetPosition()) >= 2)
				{
					std::vector<Vec3> temp;
					for (int i = 5; i < 360; i += 5)
					{
						Vec3 posteQ1;
						Vec3 poseQ1;
						posteQ1 = RotateAround(enemy->GetPosition(), player->GetPosition(), i);
						poseQ1 = Extend(player->GetPosition(), posteQ1, E->Range()/3);
						if (Distance(poseQ1, enemy->GetPosition()) >= player->AttackRange() - 150 && Distance(poseQ1, enemy->GetPosition()) <= player->AttackRange() - 50 && !GNavMesh->IsPointWall(poseQ1))
						{
							temp.push_back(poseQ1);
						}
					}
					if (temp.size() == 0 && Distance(player->GetPosition(), enemy->GetPosition()) < 300)
					{
						return Extend(player->GetPosition(), enemy->GetPosition(), -450);
					}
					else if (temp.size() == 0 && Distance(player->GetPosition(), enemy->GetPosition()) > 300)
					{
					return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
					}
					
					Vec3 tem = GEntityList->GetEnemyNexus()->GetPosition();
					for (int y = 0; y < temp.size(); y++)
					{
						if (closestTurret != nullptr && Distance(tem, closestTurret->GetPosition()) >= Distance(temp[y], closestTurret->GetPosition()))
						{
							tem = temp[y];
						}
					}
					return tem;
				}
				else
					return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
			}
			else
				return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
		}
		else if (ComboE->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && !ESmart->Enabled())
		{
			return Extend(player->GetPosition(), GGame->CursorPosition(), E->Range() / 3);
		}
	}
}

bool LineDistance(IUnit* enemy, IUnit* player, float distance)
{
	auto collision = false;
	Vec3 porte = Extend(enemy->GetPosition(), player->GetPosition(), -distance);
	Vec2 from = ToVec2(enemy->GetPosition());
	Vec2 to = ToVec2(porte);
	float m = ((to.y - from.y) / (to.x - from.x));
	float X;
	float Y;
	float m2 = (-(to.x - from.x) / (to.y - from.y));
	auto heros = GEntityList->GetAllHeros(false, true);
	auto minions = GEntityList->GetAllMinions(false, true, true);

	for (auto hero : heros)
	{
		if (hero != nullptr && hero->IsValidTarget())
		{
			if (player->IsValidTarget(hero, Q->Range() + 200) && Q->IsReady() && Distance(hero->GetPosition(), player->GetPosition()) > Q->Range())
			{
				Vec3 heroP;
				heroP = hero->GetPosition();
				Vec2 heroPos = ToVec2(heroP);
				float herox = heroPos.x;
				float heroy = heroPos.y;
				X = ((m2*herox) - (from.x*m) + (from.y - heroy)) / (m2 - m);
				Y = m * (X - from.x) + from.y;
				Vec2 colliPos;
				colliPos.Set(X, Y);

				if (Distance(colliPos, heroPos) <= hero->BoundingRadius() && Distance(colliPos, from) <= 200 && Distance(from, heroPos) < Distance(ToVec2(player->GetPosition()), heroPos))
				{
					collision = true;
					break;
				}
			}
		}
	}
	return collision;
}

void  QLogic()
{
	auto player = GEntityList->Player();
	auto minions = GEntityList->GetAllMinions(false, true, true);
	for (auto minion : minions)
	{
		if (LineDistance(minion, player, 200))
		{
			if (minion != nullptr && E->IsReady() && player->IsValidTarget(minion, Q->Range()))
			{
				Q->CastOnUnit(minion);
			}
		}
	}
}

void RLock()
{
	Vec3 empt;
	empt.Set(0,0,0);
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range() + 200);
	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero() && GEntityList->Player()->IsValidTarget(enemy, R->Range() + 200))
	{
		if (LockR->Enabled() && GEntityList->Player()->HasBuff("LucianR") && direction != empt)
		{
			GOrbwalking->SetMovementAllowed(true);

			Vec3 enemyposi = enemy->GetPosition();

			GOrbwalking->SetAttacksAllowed(false);
			movePos = enemyposi + direction*(-distances - 10);
			Vec3 rotpo1 = RotateAround(enemyposi, GEntityList->Player()->GetPosition(), 90);
			Vec3 rotpo2 = RotateAround(enemyposi, GEntityList->Player()->GetPosition(), -90);
			float p12f = Distance(GEntityList->Player()->GetPosition(), rotpo1);
			float p13f = Distance(GEntityList->Player()->GetPosition(), movePos);
			float p23f = Distance(movePos, rotpo1);
			float p12s = Distance(GEntityList->Player()->GetPosition(), rotpo2);
			float p13s = Distance(GEntityList->Player()->GetPosition(), movePos);
			float p23s = Distance(movePos, rotpo2);
			double incos1 = ((p12f * p12f) + (p13f * p13f) - (p23f * p23f)) / (2 * p12f * p13f);
			double incos2 = ((p12s * p12s) + (p13s * p13s) - (p23s * p23s)) / (2 * p12s * p13s);
			double result1 = acos(incos1) * 180.0 / PI;
			double result2 = acos(incos2) * 180.0 / PI;
			if (result1 < 91)
			{
				Vec3 mover1 = RotateAround(movePos, GEntityList->Player()->GetPosition(), 2*result1);
				Vec3 mover2 = RotateAround(movePos, GEntityList->Player()->GetPosition(), -2*result1);
				choser = LineChecker(enemy, GEntityList->Player(), 400, mover1, mover2);
			}
			if (result2 < 91)
			{
				Vec3 mover1 = RotateAround(movePos, GEntityList->Player()->GetPosition(), 2*result2);
				Vec3 mover2 = RotateAround(movePos, GEntityList->Player()->GetPosition(), -2*result2);
				choser = LineChecker(enemy, GEntityList->Player(), 400, mover1, mover2);
			}
			float di1 = Distance(GGame->CursorPosition(), enemy->GetPosition());
			float di2 = Distance(GEntityList->Player()->GetPosition(), enemy->GetPosition());
			float cdi1 = Distance(choser, enemy->GetPosition());
			float cdi2 = Distance(movePos, enemy->GetPosition());
			if (enemy == nullptr)
			{
				GOrbwalking->Orbwalk(GEntityList->Player(), GGame->CursorPosition());
				GOrbwalking->SetMovementAllowed(false);
			}
			if (di1 < di2 && enemy->IsMoving())
			{
				if (cdi1 < cdi2)
				{
					GOrbwalking->Orbwalk(GEntityList->Player(), choser);
					GOrbwalking->SetMovementAllowed(false);
				}
				if (cdi1 > cdi2)
				{
					GOrbwalking->Orbwalk(GEntityList->Player(), movePos);
					GOrbwalking->SetMovementAllowed(false);
				}
			}
			if (di2 < di1 && enemy->IsMoving())
			{
				if (cdi1 > cdi2)
				{
					GOrbwalking->Orbwalk(GEntityList->Player(), choser);
					GOrbwalking->SetMovementAllowed(false);
				}
				if (cdi1 < cdi2)
				{
					GOrbwalking->Orbwalk(GEntityList->Player(), movePos);
					GOrbwalking->SetMovementAllowed(false);
				}
			}
			if(!enemy->IsMoving())
			{
				Vec3 alter = Extend(enemy->GetPosition(), movePos, di1);
				GOrbwalking->Orbwalk(GEntityList->Player(), alter);
				GOrbwalking->SetMovementAllowed(false);
			}
		}
	}
}

void KS()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range() + E->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
	{
		if (KSE->Enabled() && E->IsReady() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range() + E->Range()) && (enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotQ) + GDamage->GetAutoAttackDamage(player, enemy, false)) && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (EnemiesInRange(player, R->Range()) < 3)
			{
				E->CastOnUnit(enemy);
			}
		}
		if (KSQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotQ) && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			Q->CastOnUnit(enemy);
		}

		if (KSW->Enabled() && W->IsReady() && player->IsValidTarget(enemy, W->Range()) && enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotW) && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			W->CastOnTarget(enemy, 5);
		}
		
		if (KSR->Enabled() && R->IsReady() && player->IsValidTarget(enemy, R->Range()) && enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotR) && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (Distance(enemy, player) > Q->Range())
			{
				R->CastOnTarget(enemy, 5);
			}
			if (!Q->IsReady() && !W->IsReady() && Distance(enemy, player) <= Q->Range())
			{
				R->CastOnTarget(enemy, 5);
			}
		}
	}
}

PLUGIN_EVENT(void) OnAfterAttack(IUnit* Source, IUnit* target)
{
	if (Source == GEntityList->Player())
	{
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, GEntityList->Player()->AttackRange() + 170);
		auto Player = GEntityList->Player();
		switch (GOrbwalking->GetOrbwalkingMode())
		{
			case kModeCombo:
				if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && GEntityList->Player()->IsValidTarget(enemy, GEntityList->Player()->AttackRange() + 170))
				{
					if (ComboE->Enabled() && E->IsReady())
					{
						E->CastOnPosition(SmartELogic());
					}

					if ((!E->IsReady() || !ComboE->Enabled()) && ComboQ->Enabled() && Q->IsReady())
					{
						Q->CastOnUnit(enemy);
					}
					if (!Q->IsReady() && (!E->IsReady() || !ComboE->Enabled()) && ComboW->Enabled() && W->IsReady())
					{
						W->CastOnUnit(enemy);
					}
					if (!W->IsReady() && !Q->IsReady() && !E->IsReady())
					{
						if (Ravenous_Hydra->IsOwned() && Ravenous_Hydra->IsReady() && !(Player->IsDead()))
						{
							if (EnemiesInRange(Player, 385) > 0)
							{
								Ravenous_Hydra->CastOnPlayer();
							}
						}
						if (Tiamat->IsOwned() && Tiamat->IsReady() && !(Player->IsDead()))
						{
							if (EnemiesInRange(Player, 385) > 0)
							{
								Tiamat->CastOnPlayer();
							}
						}
						if (Titanic_Hydra->IsOwned() && Titanic_Hydra->IsReady() && !(Player->IsDead()))
						{
							if (EnemiesInRange(Player, 385) > 0)
							{
								Titanic_Hydra->CastOnPlayer();
							}
						}
					}
				}
				break;
			case kModeMixed:
				if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && GEntityList->Player()->IsValidTarget(enemy, GEntityList->Player()->AttackRange() + 170))
				{
					if (HarassE->Enabled() && E->IsReady())
					{
						E->CastOnPosition(SmartELogic());
					}

					if ((!E->IsReady() || !HarassE->Enabled()) && HarassQ->Enabled() && Q->IsReady())
					{
						Q->CastOnUnit(enemy);
					}
					if (!Q->IsReady() && (!E->IsReady() || !HarassE->Enabled()) && HarassW->Enabled() && W->IsReady())
					{
						W->CastOnUnit(enemy);
					}
					if (!W->IsReady() && !Q->IsReady() && !E->IsReady())
					{
						if (Ravenous_Hydra->IsOwned() && Ravenous_Hydra->IsReady() && !(Player->IsDead()))
						{
							if (EnemiesInRange(Player, 385) > 0)
							{
								Ravenous_Hydra->CastOnPlayer();
							}
						}
						if (Tiamat->IsOwned() && Tiamat->IsReady() && !(Player->IsDead()))
						{
							if (EnemiesInRange(Player, 385) > 0)
							{
								Tiamat->CastOnPlayer();
							}
						}
						if (Titanic_Hydra->IsOwned() && Titanic_Hydra->IsReady() && !(Player->IsDead()))
						{
							if (EnemiesInRange(Player, 385) > 0)
							{
								Titanic_Hydra->CastOnPlayer();
							}
						}
					}
				}
				break;
			case kModeLaneClear:
				auto enemies = GEntityList->GetAllMinions(false, true, true);
				for (auto minion : enemies)
				{
					if (minion != nullptr && minion->IsValidTarget() && GEntityList->Player()->IsValidTarget(minion, GEntityList->Player()->AttackRange() + 170))
					{
						if (FarmE->Enabled() && E->IsReady())
						{
							E->CastOnPosition(Extend(Player->GetPosition(), GGame->CursorPosition(), E->Range() / 3));
						}

						if ((!E->IsReady() || !FarmE->Enabled()) && FarmQ->Enabled() && Q->IsReady())
						{
							Q->CastOnUnit(minion);
						}
						if (!Q->IsReady() && (!E->IsReady() || !FarmE->Enabled()) && FarmW->Enabled() && W->IsReady())
						{
							W->CastOnUnit(minion);
						}
						if (!W->IsReady() && !Q->IsReady() && !E->IsReady())
						{
							if (Ravenous_Hydra->IsOwned() && Ravenous_Hydra->IsReady() && !(Player->IsDead()))
							{
								if (EnemiesInRange(Player, 385) > 0)
								{
									Ravenous_Hydra->CastOnPlayer();
								}
							}
							if (Tiamat->IsOwned() && Tiamat->IsReady() && !(Player->IsDead()))
							{
								if (EnemiesInRange(Player, 385) > 0)
								{
									Tiamat->CastOnPlayer();
								}
							}
							if (Titanic_Hydra->IsOwned() && Titanic_Hydra->IsReady() && !(Player->IsDead()))
							{
								if (EnemiesInRange(Player, 385) > 0)
								{
									Titanic_Hydra->CastOnPlayer();
								}
							}
						}
					}
				}
				break;
		}
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	if (!GEntityList->Player()->HasBuff("LucianR"))
	{
		GOrbwalking->SetAttacksAllowed(true);
		GOrbwalking->SetMovementAllowed(true);
	}
	if (checker == true && GGame->TickCount() - timer > 3000)
	{
		checker = false;
	}


	if (GUtility->IsKeyDown(RSemi->GetInteger()) && !GEntityList->Player()->HasBuff("LucianR") && !GGame->IsChatOpen() && GGame->Time() > KeyPre)
	{ 
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range());
		auto player = GEntityList->Player();
		if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
		{
			if (player->IsValidTarget(enemy, R->Range()) && R->IsReady())
			{
				R->CastOnTarget(enemy, 5);
				KeyPre = GGame->Time() + 0.250;
			}
		}
	}

	ChangePriority();

	KS();

	if (AutoQExt->Enabled())
	{
		QLogic();
	}

	if (GOrbwalking->GetOrbwalkingMode() == kModeNone)
	{
		GOrbwalking->SetAttacksAllowed(true);
		GOrbwalking->SetMovementAllowed(true);
	}

	if (LockR->Enabled())
	{
		RLock();
	}

	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		if (ComboQExt->Enabled())
		{
			QLogic();
		}
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		if (HarassQExt->Enabled())
		{
			QLogic();
		}
	}
}

PLUGIN_EVENT(void) OnSpellCast(CastedSpell const& Args)
{
	if (Args.Caster_ != GEntityList->Player())
	{
		return;
	}

	if (Contains(Args.Name_, "LucianR") && checker == false)
	{
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range() + 200);
		if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && GEntityList->Player()->IsValidTarget(enemy, R->Range() + 200))
		{
			direction = (enemy->GetPosition() - GEntityList->Player()->GetPosition()).VectorNormalize();
			distances = Distance(enemy->GetPosition(), GEntityList->Player()->GetPosition());
			timer = GGame->TickCount();
			checker = true;


			if (Youmuus->IsOwned() && Youmuus->IsReady())
			{
				Youmuus->CastOnPlayer();
			}
		}
	}
}

PLUGIN_EVENT(void) OnGapCloser(GapCloserSpell const& champ)
{
	if (!champ.Source->IsEnemy(GEntityList->Player()) || !champ.Source->IsHero() || !E->IsReady())
	{
		return;
	}
	
	if (GapTarget[champ.Source->GetNetworkId()]->Enabled() && champ.Source != GEntityList->Player())
	{
		if (GEntityList->Player()->IsValidTarget(champ.Source, 250))
		{
			E->CastOnPosition(Extend(GEntityList->Player()->GetPosition(), champ.Source->GetPosition(), -475.f));
		}
	}
}

PLUGIN_EVENT(void) OnRender()
{
	if (DrawPriority->Enabled())
	{
		static IFont* pFont = nullptr;

		if (pFont == nullptr)
		{
			pFont = GRender->CreateFont("Tahoma", 16.f, kFontWeightNormal);
			pFont->SetOutline(true);
			pFont->SetLocationFlags(kFontLocationNormal);
		}
		Vec2 pos;
		if (GGame->Projection(GEntityList->Player()->GetPosition(), &pos))
		{
			std::string text = std::string(Names[ComboMode]);
			Vec4 clr = Vec4(188, 255, 50, 255);
			pFont->SetColor(clr);
			pFont->Render(pos.x, pos.y, text.c_str());
		}
	}
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }

		if (W->IsReady() && DrawW->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range()); }

		if (E->IsReady() && DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }

		if (R->IsReady() && DrawR->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }
	}
	else
	{
		if (DrawQ->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }

		if (DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range()); }

		if (DrawW->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }

		if (DrawR->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	DrawMenu();
	LoadSpells();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->AddEventHandler(kEventOnSpellCast, OnSpellCast);
	GEventManager->AddEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->RemoveEventHandler(kEventOnSpellCast, OnSpellCast);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}