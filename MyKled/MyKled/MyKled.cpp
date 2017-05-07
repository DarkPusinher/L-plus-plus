#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <map>
#include <vector>

PluginSetup("A-Kled")

IMenu* MainMenu;
IMenu* Kled;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboQAA;
IMenuOption* ComboEAA;
IMenuOption* ComboE;

IMenu* HarrassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassW;
IMenuOption* HarassQAA;
IMenuOption* HarassEAA;
IMenuOption* HarassE;

IMenu* FarmMenu;
IMenuOption* FarmQ;
IMenuOption* FarmW;
IMenuOption* FarmE;
IMenuOption* FarmQAA;

IMenu* LastHitMenu;
IMenuOption* lastHitQ;

IMenu* KSMenu;
IMenuOption* KSQ;
IMenuOption* KSE;

IMenu* DrawMenu;
IMenuOption* DrawQ;
IMenuOption* DrawE;
IMenuOption* DrawR;

ISpell2* Q;
ISpell2* Q2;
ISpell2* W;
ISpell2* E;
ISpell2* R;
IInventoryItem* Tiamat;
IInventoryItem* Titanic_Hydra;
IInventoryItem* Ravenous_Hydra;

bool onMount = true;


void inline LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, (kCollidesWithYasuoWall));
	Q->SetOverrideRadius(50.f);
	Q->SetOverrideSpeed(1300.f);
	Q->SetOverrideDelay(0.25f);
	Q2 = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, (kCollidesWithNothing));
	Q2->SetOverrideRadius(50.f);
	Q2->SetOverrideSpeed(2600.f);
	Q2->SetOverrideDelay(0.25f);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, (kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, false, kCollidesWithNothing);
	R = GPluginSDK->CreateSpell2(kSlotR, kLineCast, false, false, (kCollidesWithNothing));
	Titanic_Hydra = GPluginSDK->CreateItemForId(3748, 385);
	Ravenous_Hydra = GPluginSDK->CreateItemForId(3074, 385);
	Tiamat = GPluginSDK->CreateItemForId(3077, 385);
}

void inline Menu()
{
	MainMenu = GPluginSDK->AddMenu("A-Kled");
	Kled = MainMenu->AddMenu("Kled");
	ComboMenu = Kled->AddMenu("Combo");
	{
		ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
		ComboW = ComboMenu->CheckBox("Use W in Combo", true);
		ComboE = ComboMenu->CheckBox("Use E in Combo", true);
		ComboQAA = ComboMenu->CheckBox("Use Q only for AA reset", true);
		ComboEAA = ComboMenu->CheckBox("Use E only for AA reset", true);
	}
	HarrassMenu = Kled->AddMenu("Harrass");
	{
		HarassQ = HarrassMenu->CheckBox("Use Q in Harrass", true);
		HarassW = HarrassMenu->CheckBox("Use W in Harrass", true);
		HarassE = HarrassMenu->CheckBox("Use E in Harrass", true);
		HarassQAA = HarrassMenu->CheckBox("Use Q only for AA reset", true);
		HarassEAA = HarrassMenu->CheckBox("Use E only for AA reset", true);
	}
	FarmMenu = Kled->AddMenu("Farm");
	{
		FarmQ = FarmMenu->CheckBox("Use Q in Harrass", true);
		FarmE = FarmMenu->CheckBox("Use E in Harrass", true);
		FarmQAA = FarmMenu->CheckBox("Use Q only for AA reset", true);
	}
	//LastHitMenu = Kled->AddMenu("LastHit");
	//{
	//	lastHitQ = LastHitMenu->CheckBox("Use Q for lasthit", true);
	//}
	KSMenu = Kled->AddMenu("KS Menu");
	{
		KSQ = KSMenu->CheckBox("Use Q for KS", true);
		KSE = KSMenu->CheckBox("Use E for KS", true);
	}
	DrawMenu = Kled->AddMenu("Draw Menu");
	{
		DrawQ = DrawMenu->CheckBox("Draw Q Range", true);
		DrawE = DrawMenu->CheckBox("Draw E Range", true);
		DrawR = DrawMenu->CheckBox("Draw R Range", true);
	}
}

void Combo()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, Q->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
	{
		if (player->IsValidTarget(enemy, Q->Range()) && ComboQ->Enabled() && Q->IsReady() && onMount == true && !ComboQAA->Enabled())
		{
			Vec3 posq;
			AdvPredictionOutput opt;
			GPrediction->GetFutureUnitPosition(enemy, Q->GetDelay(), true, posq);
			Q->RunPrediction(enemy, false, kCollidesWithNothing, &opt);
			if (Distance(player->GetPosition(), posq) <= Q->Range() && opt.HitChance > kHitChanceHigh)
			{
				Q->CastOnPosition(opt.CastPosition);
			}
		}
		if (player->IsValidTarget(enemy, Q->Range()) && ComboQ->Enabled() && Q->IsReady() && onMount == false && !ComboQAA->Enabled())
		{
			Vec3 posq;
			AdvPredictionOutput opt;
			Q->RunPrediction(enemy, false, kCollidesWithNothing, &opt);
			GPrediction->GetFutureUnitPosition(enemy, Q2->GetDelay(), true, posq);
			if (Distance(player->GetPosition(), posq) <= Q2->Range() && opt.HitChance > kHitChanceHigh)
			{
				Q->CastOnPosition(opt.CastPosition);
			}
		}
		if (player->IsValidTarget(enemy, E->Range()) && ComboE->Enabled() && E->IsReady() && onMount == true && !ComboEAA->Enabled())
		{
			E->CastOnUnit(enemy);
		}
	}
}

void Harass()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, Q->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
	{
		if (player->IsValidTarget(enemy, Q->Range()) && HarassQ->Enabled() && Q->IsReady() && onMount == true && !HarassQAA->Enabled())
		{
			Vec3 posq;
			GPrediction->GetFutureUnitPosition(enemy, Q->GetDelay(), true, posq);
			if (Distance(player->GetPosition(), posq) <= Q->Range())
			{
				Q->CastOnTarget(enemy, 5);
			}
		}
		if (player->IsValidTarget(enemy, Q->Range()) && HarassQ->Enabled() && Q->IsReady() && onMount == false && !HarassQAA->Enabled())
		{
			Vec3 posq;
			GPrediction->GetFutureUnitPosition(enemy, Q2->GetDelay(), true, posq);
			if (Distance(player->GetPosition(), posq) <= Q2->Range())
			{
				Q2->CastOnTarget(enemy, 5);
			}
		}
		if (player->IsValidTarget(enemy, E->Range()) && HarassE->Enabled() && E->IsReady() && onMount == true && !HarassEAA->Enabled())
		{
			E->CastOnUnit(enemy);
		}
	}
}

void KS()
{
	auto enemies = GEntityList->GetAllHeros(false, true);
	auto player = GEntityList->Player();

	for (auto enemy : enemies)
	{
		if (KSQ->Enabled() && enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && !enemy->HasBuff("judicatorintervention") && !enemy->HasBuff("UndyingRage") && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotQ))
			{
				Q->CastOnUnit(player);
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

void Farm()
{
	auto player = GEntityList->Player();
	auto minions = GEntityList->GetAllMinions(false, true, true);
	for (auto minion : minions)
	{
		if (minion != nullptr && !minion->IsDead() && player->IsValidTarget(minion, Q->Range()) && Q->IsReady() && FarmQ->Enabled() && !FarmQAA->Enabled() && (!GOrbwalking->CanAttack() || Distance(player->GetPosition(), minion->GetPosition()) >350))
		{
			Q->CastOnUnit(minion);
		}
	}
}

//void LastHit()
//{
//	auto player = GEntityList->Player();
//	auto minions = GEntityList->GetAllMinions(false, true, true);
//	for (auto minion : minions)
//	{
//		if (minion != nullptr && !minion->IsDead() && player->IsValidTarget(minion, Q->Range()) && Q->IsReady() && lastHitQ->Enabled())
//		{
//			if (minion->GetHealth() < GDamage->GetSpellDamage(player, minion, kSlotQ))
//			{
//				Q->CastOnUnit(minion);
//			}
//		}
//	}
//}

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

PLUGIN_EVENT(void) OnAfterAttack(IUnit* source, IUnit* target)
{
	auto Player = GEntityList->Player();
	if (source != Player || target == nullptr)
		return;

	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		for (auto hero : GEntityList->GetAllHeros(false, true)) {
			if (ComboQAA->Enabled() && Q->IsReady() && (hero->GetPosition() - GEntityList->Player()->GetPosition()).Length() < Q->Range())
			{
				Q->CastOnTarget(hero, 5);
			}
			if (ComboEAA->Enabled() && E->IsReady() && (hero->GetPosition() - GEntityList->Player()->GetPosition()).Length() < E->Range())
			{
				E->CastOnUnit(hero);

			}
			if (!Q->IsReady() && !E->IsReady())
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
		for (auto hero : GEntityList->GetAllHeros(false, true)) {
			if (HarassQAA->Enabled() && Q->IsReady() && (hero->GetPosition() - GEntityList->Player()->GetPosition()).Length() < Q->Range())
			{
				Q->CastOnTarget(hero, 5);
			}
			if (HarassEAA->Enabled() && E->IsReady() && (hero->GetPosition() - GEntityList->Player()->GetPosition()).Length() < E->Range())
			{
				E->CastOnUnit(hero);

			}
			if (!Q->IsReady() && !E->IsReady())
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
			if (FarmQAA->Enabled() && Q->IsReady() && (minion->GetPosition() - GEntityList->Player()->GetPosition()).Length() < Q->Range())
			{
				Q->CastOnTarget(minion, 5);
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

PLUGIN_EVENT(void) OnGameUpdate()
{
	KS();
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen())
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && !GGame->IsChatOpen())
	{
		Harass();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear && !GGame->IsChatOpen())
	{
		Farm();
	}
	//if (GOrbwalking->GetOrbwalkingMode() == kModeLastHit && !GGame->IsChatOpen())
	//{
	//	LastHit();
	//}
}

PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "Kled_Base_P_offmount.troy"))
	{
		onMount = false;
	}
	if (Equals(Source->GetObjectName(), "Kled_Base_P_remount.troy"))
	{
		onMount = true;
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

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}