#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <map>
#include <vector>
#include <string>
#include <cstring>

PluginSetup("A-Sion")

IMenu* MainMenu;
IMenu* Sion;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboEDrift;

IMenu* HarrassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassW;
IMenuOption* HarassE;
IMenuOption* HarassEDrift;

IMenu* FarmMenu;
IMenuOption* FarmQ;
IMenuOption* FarmW;
IMenuOption* FarmE;

IMenu* KSMenu;
IMenuOption* KSQ;
IMenuOption* KSW;
IMenuOption* KSE;
IMenuOption* KSED;

IMenu* Extra;
IMenuOption* QX;
IMenuOption* QTR;

IMenu* DrawMenu;
IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawED;

ISpell2* Q;
ISpell2* Q2;
ISpell2* W;
ISpell2* E;
ISpell2* R;
IInventoryItem* Tiamat;
IInventoryItem* Titanic_Hydra;
IInventoryItem* Ravenous_Hydra;

Vec2 playerPos;
Vec2 enemyPos;

int slope = 0;
int pslope = 0;

bool checker = false;
bool checkme = false;
int timer = 0;
bool checkE = false;
int timerE = 0;
std::string xyz;

void inline LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, false, false, (kCollidesWithNothing));
	Q->SetSkillshot(0.25f, 200.f, 1000.f, 750.f);
	Q->SetCharged(300.f, 750.f, 0.f);
	
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, true, (kCollidesWithNothing));
	W->SetSkillshot(0.25f, 400.f, 1000.f, 525.f);

	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, true, false, (kCollidesWithYasuoWall));
	E->SetSkillshot(0.25f, 80.f, 1800.f, 775.f);
	//Drift distance is 600

	R = GPluginSDK->CreateSpell2(kSlotR, kLineCast, false, false, (kCollidesWithNothing));
	R->SetSkillshot(0.5f, 120.f, 1000.f, 800.f);

	Titanic_Hydra = GPluginSDK->CreateItemForId(3748, 385);
	Ravenous_Hydra = GPluginSDK->CreateItemForId(3074, 385);
	Tiamat = GPluginSDK->CreateItemForId(3077, 385);
}

void inline Menu()
{
	MainMenu = GPluginSDK->AddMenu("A-Sion");
	Sion = MainMenu->AddMenu("Sion");
	ComboMenu = Sion->AddMenu("Combo");
	{
		ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
		ComboW = ComboMenu->CheckBox("Use W in Combo", true);
		ComboE = ComboMenu->CheckBox("Use E in Combo", true);
		ComboEDrift = ComboMenu->CheckBox("Use E drift in Combo", true);
	}
	//HarrassMenu = Sion->AddMenu("Harrass");
	//{
	//	HarassQ = HarrassMenu->CheckBox("Use Q in Harrass", true);
	//	HarassW = HarrassMenu->CheckBox("Use W in Harrass", true);
	//	HarassE = HarrassMenu->CheckBox("Use E in Harrass", true);
	//	HarassE = HarrassMenu->CheckBox("Use EDrift in Harrass", true);
	//}
	FarmMenu = Sion->AddMenu("Farm");
	{
		FarmQ = FarmMenu->CheckBox("Use Q in Farm", false);
		FarmW = FarmMenu->CheckBox("Use W in Farm if x >= 2", false);
		FarmE = FarmMenu->CheckBox("Use E in Farm", false);
	}
	KSMenu = Sion->AddMenu("KS Menu");
	{
		//KSQ = KSMenu->CheckBox("Use Q for KS", true);
		KSW = KSMenu->CheckBox("Use W for KS", true);
		KSE = KSMenu->CheckBox("Use E for KS", true);
		KSED = KSMenu->CheckBox("Use EDrift for KS", true);
	}
	Extra = Sion->AddMenu("Extra");
	{
		QTR = Extra->CheckBox("Q if X enemies", true);
		QX = Extra->AddInteger("Q if X enemies", 1, 5, 2);
	}
	DrawMenu = Sion->AddMenu("Draw Menu");
	{
		DrawQ = DrawMenu->CheckBox("Draw Q Range", true);
		DrawE = DrawMenu->CheckBox("Draw E Range", true);
		DrawW = DrawMenu->CheckBox("Draw W Range", true);
		DrawED = DrawMenu->CheckBox("Draw EDrift Range", true);
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
			if (player->IsValidTarget(hero, E->Range() + 600) && E->IsReady())
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
				Vec3 outta;
				GPrediction->GetFutureUnitPosition(hero, 0.6f, true, outta);
				if (Distance(colliPos, heroPos) <= enemy->BoundingRadius() + hero->BoundingRadius() - 10 && Distance(colliPos, from) <= 600 && Distance(from, heroPos) < Distance(ToVec2(player->GetPosition()), heroPos))
				{
					if (Distance(colliPos, ToVec2(outta)) <= enemy->BoundingRadius() + hero->BoundingRadius() - 5)
					{
						collision = true;
						break;
					}
				}
			}
		}
	}
	return collision;
}

void liner ()
{
	auto player = GEntityList->Player();
	auto minions = GEntityList->GetAllMinions(false, true, true);
	for (auto minion : minions)
	{
		if (LineDistance(minion, player, 600))
		{
			if (minion != nullptr && E->IsReady() && player->IsValidTarget(minion, E->Range()))
			{
				E->CastOnUnit(minion);
			}
		}
	}
}

void QMany()
{
	auto player = GEntityList->Player();

	if (EnemiesInRange(player, 7*Q->Range()/10) >= QX->GetInteger())
	{
		Vec3 castp;
		int xenemy;
		GPrediction->FindBestCastPosition(Q->Range(), 200, true, false, true, castp, xenemy);
		if (xenemy >= QX->GetInteger() && !Q->IsCharging() && Q->IsReady() && checker == false)
		{
			Q->StartCharging(castp);
			enemyPos = ToVec2(castp);
			playerPos = ToVec2(player->GetPosition());
			slope = ((enemyPos.y - playerPos.y) / (enemyPos.x - playerPos.x));
			pslope = (-(enemyPos.x - playerPos.x) / (enemyPos.y - playerPos.y));

			timer = GGame->TickCount();
			checker = true;
		}
	}

}

void ComboEs()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range() + 600);
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
	{
		if ((ComboE->Enabled()) && E->IsReady() && player->IsValidTarget(enemy, E->Range() - 50) && checkE == false)
		{
			E->CastOnTarget(enemy, kHitChanceHigh);
			checkE = true;
			timerE = GGame->TickCount();
		}
	}
}

void Combo()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range() + 600);
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
	{
		if ((!E->IsReady() || checkE == false) && ComboQ->Enabled() && !Q->IsCharging() && Q->IsReady() && player->IsValidTarget(enemy, 7 * Q->Range() / 10) && checker == false)
		{
			Q->StartCharging(enemy->GetPosition());
			enemyPos = ToVec2(enemy->GetPosition());
			playerPos = ToVec2(player->GetPosition());
			slope = ((enemyPos.y - playerPos.y) / (enemyPos.x - playerPos.x));
			pslope = (-(enemyPos.x - playerPos.x) / (enemyPos.y - playerPos.y));

			timer = GGame->TickCount();
			checker = true;
		}
		
		if (ComboW->Enabled() && W->IsReady() && player->IsValidTarget(enemy, W->Range() - 200))
		{
			W->CastOnPlayer();
		}
	}
}

void DistanceFromLine()
{
	if (Q->IsCharging())
	{
		auto enemy = GOrbwalking->GetLastTarget();
		auto player = GEntityList->Player();
		Vec2 emp;
		emp.Set(0, 0);

		if (enemyPos != emp && enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
		{
			if (player->IsValidTarget(enemy, Q->Range()))
			{
				Vec2 posit = ToVec2(enemy->GetPosition());
				float x = ((pslope*enemyPos.x) - (posit.x*slope) + (posit.y - enemyPos.y)) / (pslope - slope);
				float y = slope*(x - enemyPos.x) + enemyPos.y;
				Vec2 colliPos;
				colliPos.Set(x, y);
				Vec3 extrp = Extend(player->GetPosition(), ToVec3(enemyPos), Q->Range());

				if ((Distance(colliPos, posit) < Q->Radius() + enemy->BoundingRadius()))
				{
					if (Distance(player->GetPosition(), enemy->GetPosition()) < Q->Range() + enemy->BoundingRadius())
					{
						if (Distance(extrp, player->GetPosition()) > Distance(extrp, enemy->GetPosition()))
						{
							Vec3 outta;
							GPrediction->GetFutureUnitPosition(enemy, 0.1f, true, outta);
							if (Distance(ToVec2(outta), colliPos) > Q->Radius() + enemy->BoundingRadius() || Distance(player->GetPosition(), outta) > Q->Range()
								|| Distance(extrp, player->GetPosition()) < Distance(extrp, outta))
							{
								checkme = true;
							}
						}
					}
				}
				else
				{
					checkme = false;
				}
			}
			else
				checkme = false;
		}
		else
			checkme = false;
	}
}

//void Harass()
//{
//	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range() + 600);
//	auto player = GEntityList->Player();
//
//	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
//	{
//		if (HarassQ->Enabled() && !Q->IsCharging() && Q->IsReady() && player->IsValidTarget(enemy, 7 * Q->Range() / 10) && checker == false)
//		{
//			Q->StartCharging(enemy->GetPosition());
//			enemyPos = ToVec2(enemy->GetPosition());
//			playerPos = ToVec2(player->GetPosition());
//			slope = ((enemyPos.y - playerPos.y) / (enemyPos.x - playerPos.x));
//			pslope = (-(enemyPos.x - playerPos.x) / (enemyPos.y - playerPos.y));
//
//			timer = GGame->TickCount();
//			checker = true;
//		}
//		
//		if (HarassW->Enabled() && W->IsReady() && player->IsValidTarget(enemy, W->Range() - 200))
//		{
//			W->CastOnPlayer();
//		}
//	}
//}

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

void Farm()
{
	auto player = GEntityList->Player();
	auto minions = GEntityList->GetAllMinions(false, true, true);

	for (auto minion : minions)
	{
		if (minion != nullptr && minion->IsValidTarget())
		{
			if (!E->IsReady() && FarmQ->Enabled() && Distance(minion, player) < Q->Range()
				&& !Q->IsCharging() && Q->IsReady() && checker == false)
			{
				Vec3 castp;
				int xenemy;
				GPrediction->FindBestCastPosition(Q->Range(), 200, true, true, false, castp, xenemy);

				Q->StartCharging(castp);
				enemyPos = ToVec2(castp);
				playerPos = ToVec2(player->GetPosition());
				slope = ((enemyPos.y - playerPos.y) / (enemyPos.x - playerPos.x));
				pslope = (-(enemyPos.x - playerPos.x) / (enemyPos.y - playerPos.y));

				timer = GGame->TickCount();
				checker = true;
			}
			if (FarmE->Enabled() && E->IsReady() && player->IsValidTarget(minion, E->Range()))
			{
				E->CastOnUnit(minion);
			}
			if (FarmW->Enabled() && W->IsReady() && CountMinionsNearMe(player, W->Range()) >= 2)
			{
				W->CastOnPlayer();
			}
		}
	}
}

void KS()
{
	auto enemies = GEntityList->GetAllHeros(false, true);
	auto player = GEntityList->Player();
	
	for (auto enemy : enemies)
	{
		if (KSW->Enabled() && enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, W->Range()) && W->IsReady() && !enemy->HasBuff("judicatorintervention") && !enemy->HasBuff("UndyingRage") && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotW))
			{
				W->CastOnPlayer();
			}
		}
		if (KSE->Enabled() && enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, E->Range()) && E->IsReady() && !enemy->HasBuff("judicatorintervention") && !enemy->HasBuff("UndyingRage") && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotE))
			{
				E->CastOnTarget(enemy, 5);
			}
		}
	}
}

PLUGIN_EVENT(void) OnAfterAttack(IUnit* source, IUnit* target)
{
	auto Player = GEntityList->Player();
	if (source != Player || target == nullptr)
		return;

	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		for (auto hero : GEntityList->GetAllHeros(false, true)) {
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
		break;
	case kModeMixed:
		for (auto hero : GEntityList->GetAllHeros(false, true)) {
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
		break;
	case kModeLaneClear:
		for (auto minion : GEntityList->GetAllMinions(false, true, true)) {
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
		break;
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	if (checkE = true && GGame->TickCount() - timerE > 500)
	{
		checkE = false;
	}

	DistanceFromLine();

	if ((checker == true && GGame->TickCount() - timer > 1950) || checkme == true)
	{
		Q->CastOnPlayer();
		checker = false;
		checkme = false;
	}

	KS();
	if (KSED->Enabled())
	{
		auto enemies = GEntityList->GetAllHeros(false, true);
		auto player = GEntityList->Player();
		for (auto enemy : enemies)
		{
			if (enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, E->Range() + 600) && E->IsReady() && !enemy->HasBuff("judicatorintervention") && !enemy->HasBuff("UndyingRage") && !enemy->HasBuffOfType(BUFF_Invulnerability))
			{
				if (enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotE))
				{
					liner();
				}
			}
		}
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen())
	{
		ComboEs();

		if (ComboE->Enabled() && ComboEDrift->Enabled())
		{
			liner();
		}

		Combo();
		
		if (QTR->Enabled())
		{
			QMany();
		}
	}
	//if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && !GGame->IsChatOpen())
	//{
	//	ComboEs();
	//
	//	if (HarassE->Enabled() && HarassEDrift->Enabled())
	//	{
	//		liner();
	//	}
	//
	//	Harass();
	//	
	//	if (QTR->Enabled())
	//	{
	//		QMany();
	//	}
	//}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear && !GGame->IsChatOpen())
	{
		Farm();
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
	if (DrawW->Enabled())
	{
		GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range());
	}
	if (DrawED->Enabled())
	{
		GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range() + 600);
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}