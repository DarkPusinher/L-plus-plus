#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <map>
#include <vector>

PluginSetup("A-Darius")

IMenu* MainMenu;
IMenu* Darius;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboWAA;
IMenuOption* ComboE;

IMenu* HarrassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassW;
IMenuOption* HarassWAA;
IMenuOption* HarassE;

IMenu* FarmMenu;
IMenuOption* FarmQ;
IMenuOption* FarmW;
IMenuOption* FarmWAA;

IMenu* KSMenu;
IMenuOption* KSQ;
IMenuOption* KSW;
IMenuOption* KSE;
IMenuOption* KSR;

IMenu* Extra;
IMenuOption* LQ;
IMenuOption* EX;
IMenuOption* EO;
IMenuOption* QO;

IMenu* DrawMenu;
IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawR;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
IInventoryItem* Tiamat;
IInventoryItem* Titanic_Hydra;
IInventoryItem* Ravenous_Hydra;


IUnit* DariusQ;
IUnit* MaxStack;
vector<IUnit*> hemos;
Vec3 posi;

void inline LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kCircleCast, false, true, (kCollidesWithNothing));
	Q->SetOverrideRange(425.f);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, (kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kConeCast, false, true, kCollidesWithYasuoWall);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, (kCollidesWithNothing));
	Titanic_Hydra = GPluginSDK->CreateItemForId(3748, 385);
	Ravenous_Hydra = GPluginSDK->CreateItemForId(3074, 385);
	Tiamat = GPluginSDK->CreateItemForId(3077, 385);
}

void inline Menu()
{
	MainMenu = GPluginSDK->AddMenu("A-Darius");
	Darius = MainMenu->AddMenu("Darius");
	ComboMenu = Darius->AddMenu("Combo");
	{
		ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
		ComboW = ComboMenu->CheckBox("Use W in Combo", true);
		ComboWAA = ComboMenu->CheckBox("Use W only for AA reset", true);
		ComboE = ComboMenu->CheckBox("Use E in Combo", true);
	}
	HarrassMenu = Darius->AddMenu("Harrass");
	{
		HarassQ = HarrassMenu->CheckBox("Use Q in Harrass", true);
		HarassW = HarrassMenu->CheckBox("Use W in Harrass", true);
		HarassWAA = HarrassMenu->CheckBox("Use W only for AA reset", true);
		HarassE = HarrassMenu->CheckBox("Use E in Harrass", true);
	}
	FarmMenu = Darius->AddMenu("Farm");
	{
		FarmQ = FarmMenu->CheckBox("Use Q in Harrass", false);
		FarmW = FarmMenu->CheckBox("Use W in Harrass", false);
		FarmWAA = FarmMenu->CheckBox("Use W only for AA reset", false);
	}
	KSMenu = Darius->AddMenu("KS Menu");
	{
		KSQ = KSMenu->CheckBox("Use Q for KS", true);
		KSW = KSMenu->CheckBox("Use W for KS", true);
		KSE = KSMenu->CheckBox("Use E for KS", true);
		KSR = KSMenu->CheckBox("Use R for KS", true);
	}
	Extra = Darius->AddMenu("Extra");
	{
		LQ = Extra->CheckBox("Lock Q distance", true);
		EX = Extra->AddInteger("E if X enemies", 1, 5, 2);
		EO = Extra->CheckBox("E only for out of AA range", true);
		QO = Extra->CheckBox("Q only for out of AA range", true);
	}
	DrawMenu = Darius->AddMenu("Draw Menu");
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

void EMany()
{
	auto player = GEntityList->Player();

	if (EnemiesInRange(player, E->Range()) >= EX->GetInteger())
	{
		Vec3 castp;
		int xenemy;
		GPrediction->FindBestCastPosition(E->Range(), 300, true, false, true, castp, xenemy);
		if (xenemy >= EX->GetInteger())
		{
			E->CastOnPosition(castp);
		}
	}

}

double RDamage()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range());
	double damage = GDamage->GetSpellDamage(GEntityList->Player(), enemy, kSlotR);
	double rdmg;
	if (enemy->HasBuff("DariusHemo"))
	{
		for (auto Source : GEntityList->GetAllUnits())
		{
			if (Distance(Source->GetPosition(), enemy->GetPosition()) <= 150 && Equals(Source->GetObjectName(), "darius_Base_hemo_counter_01.troy"))
			{
				rdmg = damage * 1.2;
			}
			if (Distance(Source->GetPosition(), enemy->GetPosition()) <= 150 && Equals(Source->GetObjectName(), "darius_Base_hemo_counter_02.troy"))
			{
				rdmg = damage * 1.2 * 1.2;
			}
			if (Distance(Source->GetPosition(), enemy->GetPosition()) <= 150 && Equals(Source->GetObjectName(), "darius_Base_hemo_counter_03.troy"))
			{
				rdmg = damage * 1.2 * 1.2 * 1.2;
			}
			if (Distance(Source->GetPosition(), enemy->GetPosition()) <= 150 && Equals(Source->GetObjectName(), "darius_Base_hemo_counter_04.troy"))
			{
				rdmg = damage * 1.2 * 1.2 * 1.2 * 1.2;
			}
			if (Distance(Source->GetPosition(), enemy->GetPosition()) <= 150 && Equals(Source->GetObjectName(), "darius_Base_passive_overhead_max_stack.troy"))
			{
				rdmg = damage * 1.2 * 1.2 * 1.2 * 1.2 * 1, 2 * 1, 2;
			}
		}
	}
	return rdmg;
}

void QLock()
{
	if (LQ->Enabled())
	{
		if (DariusQ == nullptr)
		{
			GOrbwalking->SetAttacksAllowed(true);
			GOrbwalking->SetMovementAllowed(true);
		}
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, Q->Range() + 400);
		auto player = GEntityList->Player();
		if (enemy != nullptr && player->IsValidTarget(enemy, Q->Range() + 400) && enemy->IsHero())
		{
			if (DariusQ != nullptr)
			{
				GOrbwalking->SetMovementAllowed(true);
				posi = Extend(enemy->GetPosition(), player->GetPosition(), 350.f);
				GOrbwalking->SetAttacksAllowed(false);
				GOrbwalking->Orbwalk(enemy, posi);
				GOrbwalking->SetMovementAllowed(false);
			}
		}
	}
}

void Combo()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, 300))
	{
		if (ComboW->Enabled() && !ComboWAA->Enabled())
		{
			W->CastOnPlayer();
		}
	}
	if (enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, E->Range()))
	{
		if (ComboE->Enabled() && !EO->Enabled())
		{
			Vec3 oout;
			GPrediction->GetFutureUnitPosition(enemy, 80.f, true, oout);
			if (Distance(player->GetPosition(), oout) <= E->Range() -50)
			{
				E->CastOnUnit(enemy);
			}
		}
		if (ComboE->Enabled() && EO->Enabled())
		{
			Vec3 oout;
			GPrediction->GetFutureUnitPosition(enemy, 80.f, true, oout);
			if (Distance(player->GetPosition(), oout) <= E->Range() - 50 && Distance(enemy->GetPosition(), player->GetPosition()) > 300)
			{
				E->CastOnUnit(enemy);
			}
		}
	}
	if (enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, Q->Range()))
	{
		if (ComboQ->Enabled() && !QO->Enabled())
		{
			Q->CastOnPlayer();
		}
		if (ComboQ->Enabled() && QO->Enabled())
		{
			if (Distance(player->GetPosition(), enemy->GetPosition()) <= Q->Range() && Distance(enemy->GetPosition(), player->GetPosition()) > 300)
			{
				Q->CastOnPlayer();
			}
		}
	}
}

void Harass()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, E->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, 300))
	{
		if (HarassW->Enabled() && !HarassWAA->Enabled())
		{
			W->CastOnPlayer();
		}
	}
	if (enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, E->Range()))
	{
		if (HarassE->Enabled() && !EO->Enabled())
		{
			Vec3 oout;
			GPrediction->GetFutureUnitPosition(enemy, 80.f, true, oout);
			if (Distance(player->GetPosition(), oout) <= E->Range() - 50)
			{
				E->CastOnUnit(enemy);
			}
		}
		if (HarassE->Enabled() && EO->Enabled())
		{
			Vec3 oout;
			GPrediction->GetFutureUnitPosition(enemy, 80.f, true, oout);
			if (Distance(player->GetPosition(), oout) <= E->Range() - 50 && Distance(enemy->GetPosition(), player->GetPosition()) > 300)
			{
				E->CastOnUnit(enemy);
			}
		}
	}
	if (enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, Q->Range()))
	{
		if (HarassQ->Enabled() && !QO->Enabled())
		{
			Q->CastOnPlayer();
		}
		if (HarassQ->Enabled() && QO->Enabled())
		{
			if (Distance(player->GetPosition(), enemy->GetPosition()) <= Q->Range() && Distance(enemy->GetPosition(), player->GetPosition()) > 300)
			{
				Q->CastOnPlayer();
			}
		}
	}
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

void Farm()
{
	auto player = GEntityList->Player();
	if (CountMinionsNearMe(player, Q->Range()) >= 2 && FarmQ->Enabled())
	{
		Q->CastOnPlayer();
	}
	if (CountMinionsNearMe(player, 300) >= 1 && FarmW->Enabled())
	{
		W->CastOnPlayer();
	}
}

void KS()
{
	auto enemies = GEntityList->GetAllHeros(false, true);
	auto player = GEntityList->Player();

	for (auto enemy : enemies)
	{
		if (KSR->Enabled() && enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, R->Range()) && R->IsReady() && !enemy->HasBuff("judicatorintervention") && !enemy->HasBuff("UndyingRage") && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (enemy->GetHealth() < RDamage() * 0.8)
			{
				R->CastOnUnit(enemy);
			}
		}
		if (KSQ->Enabled() && enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && !enemy->HasBuff("judicatorintervention") && !enemy->HasBuff("UndyingRage") && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotQ))
			{
				Q->CastOnUnit(player);
			}
		}
		if (KSW->Enabled() && enemy != nullptr && enemy->IsHero() && player->IsValidTarget(enemy, W->Range()) && W->IsReady() && !enemy->HasBuff("judicatorintervention") && !enemy->HasBuff("UndyingRage") && !enemy->HasBuffOfType(BUFF_Invulnerability))
		{
			if (enemy->GetHealth() < GDamage->GetSpellDamage(player, enemy, kSlotW))
			{
				W->CastOnUnit(player);
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
			if (ComboWAA->Enabled() && W->IsReady() && (hero->GetPosition() - GEntityList->Player()->GetPosition()).Length() < 350)
			{
				W->CastOnPlayer();

			}
			if (!W->IsReady())
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
			if (HarassWAA->Enabled() && W->IsReady() && (hero->GetPosition() - GEntityList->Player()->GetPosition()).Length() < 350)
			{
				W->CastOnPlayer();
			}
			if (!W->IsReady())
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
			if (FarmW->Enabled() && W->IsReady() && (minion->GetPosition() - GEntityList->Player()->GetPosition()).Length() < 350)
			{
				W->CastOnPlayer();
			}
			if (!W->IsReady())
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
	QLock();
	KS();
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen())
	{
		Combo();
		EMany();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && !GGame->IsChatOpen())
	{
		Harass();
		EMany();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear && !GGame->IsChatOpen())
	{
		Farm();
	}
}

PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "Darius_Base_Q_Ring_Windup.troy"))
	{
		DariusQ = Source;
	}
	
	if (Equals(Source->GetObjectName(), "darius_Base_passive_overhead_max_stack.troy"))
	{
		MaxStack = Source;
	}
}

PLUGIN_EVENT(void) OnDestroyObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "Darius_Base_Q_Ring_Windup.troy"))
	{
		DariusQ = nullptr;
	}
	if (Equals(Source->GetObjectName(), "darius_Base_passive_overhead_max_stack.troy"))
	{
		MaxStack = nullptr;
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
	GEventManager->AddEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->RemoveEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}