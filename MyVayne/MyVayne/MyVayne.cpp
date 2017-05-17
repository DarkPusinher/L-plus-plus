#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <sstream>
#include <map>
#include <string>
#include <vector>

PluginSetup("MyVayne")

IMenu* MainMenu;

IMenu* ComboMenu;
IMenuOption* QSmart;
IMenuOption* InvisH;
IMenuOption* ComboQ;
IMenuOption* Keeper;
IMenuOption* AutoE;
IMenuOption* AutoR;
IMenuOption* AutoRX;

IMenu* ExtraMenu;
IMenuOption* EGap;
IMenuOption* EInt;

IMenu* RenderMenu;
IMenuOption* DrawReady;
IMenuOption* DrawQ;
IMenuOption* DrawE;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;


void  DrawMenu()
{
	MainMenu = GPluginSDK->AddMenu("A-Series Vayne");

	ComboMenu = MainMenu->AddMenu("Combo");
	ComboQ = ComboMenu->CheckBox("Use Q", true);
	QSmart = ComboMenu->CheckBox("Smart Q", true);
	Keeper = ComboMenu->CheckBox("Keep invisibility", true);
	InvisH = ComboMenu->AddFloat("Keep invisibilty if HP < %", 0, 100, 35);
	AutoE = ComboMenu->CheckBox("Auto Condemn", true);
	AutoR = ComboMenu->CheckBox("Auto R if more than x", true);
	AutoRX = ComboMenu->AddInteger("Enemies in range", 1, 5, 2);
	
	ExtraMenu = MainMenu->AddMenu("Extra");
	EGap = ExtraMenu->CheckBox("Auto Anti-GapCloser", true);
	EInt = ExtraMenu->CheckBox("Auto interrupter", true);

	RenderMenu = MainMenu->AddMenu("Drawing Settings");
	DrawReady = RenderMenu->CheckBox("Draw Only Ready Spells", true);
	DrawQ = RenderMenu->CheckBox("Draw Q", true);
	DrawE = RenderMenu->CheckBox("Draw E", true);
}

void  LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, false, false, kCollidesWithNothing);
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, false, kCollidesWithNothing);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, true, false, kCollidesWithYasuoWall);
	E->SetOverrideRange(750);
	R = GPluginSDK->CreateSpell2(kSlotR, kCircleCast, false, false, kCollidesWithNothing);
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
		if (Distance(colliPos, minionPos) <= enemy->BoundingRadius() + minion->BoundingRadius() - 10 && Distance(colliPos, from) <= 425 && Distance(from, minionPos) < Distance(ToVec2(player->GetPosition()), minionPos))
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
		if (Distance(colliPos, heroPos) <= enemy->BoundingRadius() + hero->BoundingRadius() - 10 && Distance(colliPos, from) <= 425 && Distance(from, heroPos) < Distance(ToVec2(player->GetPosition()), heroPos))
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
	if (Distance(colliPos, mnexusPos) <= enemy->BoundingRadius() + mnexus->BoundingRadius() - 50 && Distance(colliPos, from) <= 425 && Distance(from, mnexusPos) < Distance(ToVec2(player->GetPosition()), mnexusPos))
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
	if (Distance(colliPos1, enexusPos) <= enemy->BoundingRadius() + enexus->BoundingRadius() - 50 && Distance(colliPos1, from) <= 425 && Distance(from, enexusPos) < Distance(ToVec2(player->GetPosition()), enexusPos))
	{
		collision = true;
	}
	return collision;
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
		if (Distance(colliPos, minionPos) <= enemy->BoundingRadius() + minion->BoundingRadius() - 100 && Distance(colliPos, from) <= 425 && Distance(from, minionPos) < Distance(ToVec2(playerPosi), minionPos))
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
		if (Distance(colliPos, heroPos) <= enemy->BoundingRadius() + hero->BoundingRadius() - 100 && Distance(colliPos, from) <= 425 && Distance(from, heroPos) < Distance(ToVec2(playerPosi), heroPos))
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
	if (Distance(colliPos, mnexusPos) <= enemy->BoundingRadius() + mnexus->BoundingRadius() - 100 && Distance(colliPos, from) <= 425 && Distance(from, mnexusPos) < Distance(ToVec2(playerPosi), mnexusPos))
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
	if (Distance(colliPos1, enexusPos) <= enemy->BoundingRadius() + enexus->BoundingRadius() - 100 && Distance(colliPos1, from) <= 425 && Distance(from, enexusPos) < Distance(ToVec2(playerPosi), enexusPos))
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

void  ELogic() // Unique
{
	auto enemies = GEntityList->GetAllHeros(false, true);
	auto player = GEntityList->Player();

	for (auto target : enemies)
	{
		if (target != nullptr && target->IsHero())
		{
			if (player->IsValidTarget(target, E->Range()) && AutoE->Enabled() && E->IsReady() && (LineEquation(target, player, 425) == true || buildingCheck(target, player, 425)))
			{
				E->CastOnUnit(target);
			}
		}
	}
}

Vec3 SmartQLogic()                 
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, player->AttackRange());
	auto nexus = GEntityList->GetTeamNexus();
	Vec3 post1;
	Vec3 post2;
	Vec3 pos1;
	Vec3 pos2;
	bool success = false;
	bool check = false;
	
	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero() && QSmart->Enabled() && E->IsReady() && ComboQ->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		if (player->IsValidTarget(enemy, Q->Range() + player->AttackRange()))
		{
			for (int i = 5; i < 360; i += 5)
			{
				Vec3 postQ1;
				Vec3 posQ1;
				postQ1 = RotateAround(enemy->GetPosition(), player->GetPosition(), i);
				posQ1 = Extend(player->GetPosition(), postQ1, Q->Range());
				if (Q->IsReady() && QSmart->Enabled() && ComboQ->Enabled() && (LineEquations(enemy, posQ1, player, 425) == true || buildingChecks(enemy, posQ1, player, 425) == true))
				{
					return posQ1;
					success = true;
					break;
				}
			}
		}
	}


	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero() && ComboQ->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && QSmart->Enabled() && success == false)
	{
		if (EnemiesInRange(1100.f, player->GetPosition()) > 2)
		{
			if (EnemiesInRange(player->AttackRange() + 100, player->GetPosition()) >= 2)
			{
				post1 = RotateAround(enemy->GetPosition(), player->GetPosition(), 90);
				post2 = RotateAround(enemy->GetPosition(), player->GetPosition(), -90);
				pos1 = Extend(player->GetPosition(), post1, Q->Range());
				pos2 = Extend(player->GetPosition(), post2, Q->Range());
				if (Distance(pos1, nexus->GetPosition()) <= Distance(pos2, nexus->GetPosition()))
				{
					return pos1;
				}
				else
					return pos2;
			}
			else
				return GGame->CursorPosition();
		}
		else
			return GGame->CursorPosition();
	}
	else if (ComboQ->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && !QSmart->Enabled())
	{
		return GGame->CursorPosition();
	}
}

void  RLogic()
{
	if (AutoR->Enabled() && R->IsReady())
	{
		if (AutoRX->GetInteger() < EnemiesInRange(700, GEntityList->Player()->GetPosition()))
		{
			R->CastOnPlayer();
		}
		if (GEntityList->Player()->HasBuff("vaynetumblefade") && Keeper->Enabled() && GEntityList->Player()->HealthPercent() < InvisH->GetFloat())
		{
			GOrbwalking->SetAttacksAllowed(false);
		}
		else if (!GEntityList->Player()->HasBuff("vaynetumblefade"))
		{
			GOrbwalking->SetAttacksAllowed(true);
		}
	}
}

PLUGIN_EVENT(void) OnAfterAttack(IUnit* Source, IUnit* target)
{
	if (Source == GEntityList->Player())
	{
		if (ComboQ->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo)
		{
			Q->CastOnPosition(SmartQLogic());
		}
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	ELogic();

	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		RLogic();
	}
}

PLUGIN_EVENT(void) OnGapCloser(GapCloserSpell const& champ)
{
	auto player = GEntityList->Player();
	if (champ.Source != GEntityList->Player() && champ.Source != nullptr && player->IsValidTarget(champ.Source, E->Range()) && EGap->Enabled() && E->IsReady()
		&& (Distance(player->GetPosition(), champ.EndPosition) < 350))
	{
		E->CastOnUnit(champ.Source);
	}
}

PLUGIN_EVENT(void) OnRender()
{
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }

		if (E->IsReady() && DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }
	}
	else
	{
		if (DrawQ->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }

		if (DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }
	}
}

PLUGIN_EVENT(void) OnInterruptible(InterruptibleSpell const& champ)
{
	if (EInt->Enabled() && Distance(GEntityList->Player(), champ.Source) < E->Range())
	{
		if (E->IsReady())
		{
			E->CastOnUnit(champ.Source);
		}
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	DrawMenu();
	LoadSpells();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->AddEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->AddEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->RemoveEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}