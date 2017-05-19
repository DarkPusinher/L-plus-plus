#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <map>
#include <vector>

PluginSetup("A-Poppy")

IMenu* MainMenu;
IMenu* Poppy;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboQAA;
IMenuOption* ComboE;

IMenu* HarassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassQAA;
IMenuOption* HarassE;

IMenu* FarmMenu;
IMenuOption* FarmQ;
IMenuOption* FarmQAA;

IMenu* KSMenu;
IMenuOption* KSQ;
IMenuOption* KSE;

IMenu* Extra;
IMenuOption* WDash;
IMenuOption* EInt;

IMenu* DrawMenu;
IMenuOption* DrawQ;
IMenuOption* DrawE;
IMenuOption* DrawR;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
ISpell2* R2;
IInventoryItem* Tiamat;
IInventoryItem* Titanic_Hydra;
IInventoryItem* Ravenous_Hydra;

IUnit* Passive = nullptr;
Vec3 behind;
Vec3 epos;
Vec3 spos;
Vec3 epos1;
Vec3 spos1;
Vec3 epos2;
Vec3 spos2;

bool checker = false;
bool bugger = false;
int time1;
int time2;
int timePas;

void inline LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, false, true, (kCollidesWithNothing));
	Q->SetSkillshot(0.5f, 100.f, 3000.f, 430.f);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, (kCollidesWithNothing));
	W->SetSkillshot(0.25f, 400.f, 3000.f, 400.f);
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, false, kCollidesWithWalls);
	E->SetSkillshot(0.25f, 0.f, 3000.f, 475.f);
	R = GPluginSDK->CreateSpell2(kSlotR, kLineCast, false, true, (kCollidesWithNothing));
	R->SetSkillshot(0.3f, 150.f, 1600.f, 1200.f);
	R2 = GPluginSDK->CreateSpell2(kSlotR, kLineCast, false, true, (kCollidesWithNothing));
	R2->SetSkillshot(0.3f, 150.f, 1600.f, 1200.f);
	Titanic_Hydra = GPluginSDK->CreateItemForId(3748, 385);
	Ravenous_Hydra = GPluginSDK->CreateItemForId(3074, 385);
	Tiamat = GPluginSDK->CreateItemForId(3077, 385);
}

void inline Menu()
{
	MainMenu = GPluginSDK->AddMenu("A-Poppy");
	Poppy = MainMenu->AddMenu("Poppy");
	ComboMenu = Poppy->AddMenu("Combo");
	{
		ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
		ComboQAA = ComboMenu->CheckBox("Use Q only for AA reset", true);
		ComboE = ComboMenu->CheckBox("Use E in Combo", true);
	}

	HarassMenu = Poppy->AddMenu("Harass");
	{
		HarassQ = HarassMenu->CheckBox("Use Q in Harass", true);
		HarassQAA = HarassMenu->CheckBox("Use Q only for AA reset", true);
		HarassE = HarassMenu->CheckBox("Use E in Harass", true);
	}

	FarmMenu = Poppy->AddMenu("Farm");
	{
		FarmQ = FarmMenu->CheckBox("Use Q in Farm", true);
		FarmQAA = FarmMenu->CheckBox("Use Q only for AA reset", true);
	}

	KSMenu = Poppy->AddMenu("KS");
	{
		KSQ = KSMenu->CheckBox("Use Q for KS", true);
		KSE = KSMenu->CheckBox("Use E for KS", true);
	}

	Extra = Poppy->AddMenu("Extra");
	{
		WDash = Extra->CheckBox("W on Dash", true);
		EInt = Extra->CheckBox("E to interrupt skills", true);
	}

	DrawMenu = Poppy->AddMenu("Draw Menu");
	{
		DrawQ = DrawMenu->CheckBox("Draw Q Range", true);
		DrawE = DrawMenu->CheckBox("Draw E Range", true);
		DrawR = DrawMenu->CheckBox("Draw R Range", true);
	}
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

void Combo()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range());
	auto enemies = GEntityList->GetAllHeros(false, true);
	auto player = GEntityList->Player();

	for (auto target : enemies)
	{
		if (target != nullptr && target->IsHero())
		{
			if (player->IsValidTarget(target, E->Range()) && ComboE->Enabled() && E->IsReady() && (LineEquation(target, player, 425) == true || buildingCheck(target, player, 425)))
			{
				E->CastOnUnit(target);
			}
		}
	}
	if (enemy != nullptr && enemy->IsHero())
	{
		if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && ComboQAA->Enabled())
		{
			Vec3 yey;
			GPrediction->GetFutureUnitPosition(enemy, 0.2f, true, yey);
			if (Distance(player->GetPosition(), yey) <= Q->Range() && Distance(player->GetPosition(), enemy->GetPosition()) > 250)
			{
				Q->CastOnUnit(enemy);
			}
		}
		if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && !ComboQAA->Enabled())
		{
			Vec3 yey;
			GPrediction->GetFutureUnitPosition(enemy, 0.2f, true, yey);
			if (Distance(player->GetPosition(), yey) <= Q->Range())
			{
				Q->CastOnUnit(enemy);
			}
		}
	}
}

void Harass()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, E->Range()) && HarassE->Enabled() && E->IsReady() && (LineEquation(enemy, player, 425) == true || buildingCheck(enemy, player, 425)))
		{
			E->CastOnUnit(enemy);
		}
		if (HarassQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && HarassQAA->Enabled())
		{
			Vec3 yey;
			GPrediction->GetFutureUnitPosition(enemy, 0.2f, true, yey);
			if (Distance(player->GetPosition(), yey) <= Q->Range() && Distance(player->GetPosition(), enemy->GetPosition()) > 250)
			{
				Q->CastOnUnit(enemy);
			}
		}
		if (HarassQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && !HarassQAA->Enabled())
		{
			Vec3 yey;
			GPrediction->GetFutureUnitPosition(enemy, 0.2f, true, yey);
			if (Distance(player->GetPosition(), yey) <= Q->Range())
			{
				Q->CastOnUnit(enemy);
			}
		}
	}
}

void KS()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero())
	{
		Vec3 cps = Extend(enemy->GetPosition(), player->GetPosition(), -425);
		if (E->IsReady() && Q->IsReady() && KSE->Enabled() && player->IsValidTarget(enemy, E->Range()) && enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotE) + GDamage->GetAutoAttackDamage(player, enemy, false) + GDamage->GetSpellDamage(player, enemy, kSlotQ) && GUtility->IsPositionUnderTurret(cps, false, true) == false)
		{
			E->CastOnUnit(enemy);
		}
		if (Q->IsReady() && KSQ->Enabled() && player->IsValidTarget(enemy, Q->Range()) && enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotQ))
		{
			Q->CastOnTarget(enemy, 5);
		}
		if (E->IsReady() && KSE->Enabled() && player->IsValidTarget(enemy, E->Range()) && enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotE) + GDamage->GetAutoAttackDamage(player, enemy, false) && GUtility->IsPositionUnderTurret(cps, false, true) == false)
		{
			E->CastOnUnit(enemy);
		}
	}
}

PLUGIN_EVENT(void) OnAfterAttack(IUnit* source, IUnit* target)
{
	auto heros = GEntityList->GetAllHeros(false, true);
	auto Player = GEntityList->Player();
	if (source != Player || target == nullptr)
		return;

	if (bugger == true)
	{
		Vec3 mover = Extend(GEntityList->Player()->GetPosition(), GOrbwalking->GetLastTarget()->GetPosition(), 50);
		GOrbwalking->SetAttacksAllowed(false);
		timePas = GGame->TickCount();
		bugger = false;
	}

	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		for (auto hero : heros)
		{
			if (ComboQAA->Enabled() && Q->IsReady() && Distance(hero->GetPosition(), GEntityList->Player()->GetPosition()) <= Q->Range())
			{
				if (Distance(GEntityList->Player()->GetPosition(), hero->GetPosition()) <= Q->Range())
				{
					Q->CastOnTarget(hero, 5);
				}

			}
			if (!Q->IsReady())
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
		for (auto hero : heros)
		{
			if (HarassQAA->Enabled() && Q->IsReady() && (hero->GetPosition() - GEntityList->Player()->GetPosition()).Length() <= Q->Range())
			{
				if (Distance(GEntityList->Player()->GetPosition(), hero->GetPosition()) <= Q->Range())
				{
					Q->CastOnTarget(hero, 5);
				}
			}
			if (!Q->IsReady())
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
		for (auto minion : GEntityList->GetAllMinions(false, true, true)) {
			if (FarmQ->Enabled() && W->IsReady() && (minion->GetPosition() - GEntityList->Player()->GetPosition()).Length() <= Q->Range())
			{
				Vec3 posot;
				int ehit;
				Q->FindBestCastPosition(true, false, posot, ehit);
				if (ehit >= 1)
				{
					Q->CastOnPosition(posot);
				}
			}
			if (!Q->IsReady())
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
	}
}

PLUGIN_EVENT(void) OrbwalkBeforeAttack(IUnit* Source, IUnit* Target)
{
	if (Passive != nullptr)
	{
		bugger = true;
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	KS();


	if (bugger == false && (GGame->TickCount() - timePas) > (1000 / GEntityList->Player()->AttackSpeed()))
	{
		GOrbwalking->SetAttacksAllowed(true);
	}


	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen())
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && !GGame->IsChatOpen())
	{
		Harass();
	}
}

PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "Poppy_Base_P_cas.troy"))
	{
		Passive = Source;
	}
}

PLUGIN_EVENT(void) OnDestroyObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "Poppy_Base_P_cas.troy"))
	{
		Passive = nullptr;
	}
}

PLUGIN_EVENT(void) OnRender()
{
	if (DrawQ->Enabled())
	{
		GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range());
	}
	if (DrawE->Enabled())
	{
		GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range());
	}
	if (DrawR->Enabled())
	{
		GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range());
	}
}

PLUGIN_EVENT(void) OnDash(UnitDash* dash)
{
	if (WDash->Enabled())
	{
		if (dash->Source != nullptr && dash->Source != GEntityList->Player() && dash->Source->IsHero() && dash->Source->IsValidTarget())
		{
			Vec3 stp = dash->StartPosition;
			Vec3 enp = dash->EndPosition;
			if (GEntityList->Player()->IsValidTarget(dash->Source, W->Range() + 400) && dash->Source->IsVisible() && !GNavMesh->IsPointGrass(stp) && Distance(enp, GEntityList->Player()->GetPosition()) < W->Range())
			{
				W->CastOnPlayer();
			}
		}
	}
}

PLUGIN_EVENT(void) OnInterruptible(InterruptibleSpell const& Args)
{
	if (EInt->Enabled() && Distance(GEntityList->Player(), Args.Source) < E->Range())
	{
		if (E->IsReady() && Args.Source != nullptr && Args.Source->IsValidTarget() && GEntityList->Player()->IsValidTarget(Args.Source, E->Range()) && Args.Source->IsHero())
		{
			E->CastOnUnit(Args.Source);
		}
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->AddEventHandler(kEventOrbwalkBeforeAttack, OrbwalkBeforeAttack);
	GEventManager->AddEventHandler(kEventOnDash, OnDash);
	GEventManager->AddEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->AddEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->RemoveEventHandler(kEventOrbwalkBeforeAttack, OrbwalkBeforeAttack);
	GEventManager->RemoveEventHandler(kEventOnDash, OnDash);
	GEventManager->RemoveEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->RemoveEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}