#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <map>
#include <vector>
#define PI 3.14159265f

PluginSetup("MyKata")

IMenu* MainMenu;
IMenu* Katarina;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboRkill;
IMenuOption* ComboRhit;
IMenuOption* ComboRHit;
IMenuOption* ComboStop;

IMenu* HarrassMenu;
IMenuOption* HarrassQ;
IMenuOption* HarrassW;
IMenuOption* HarrassE;
IMenuOption* HarrassR;

IMenu* DrawingMenu;
IMenuOption* DrawQRange;
IMenuOption* DrawERange;
IMenuOption* DrawRRange;
IMenuOption* DrawPriority;

IMenu* FleeMenu;
IMenuOption* FleeW;
IMenuOption* FleeE;
IMenuOption* FleeKey;
IMenuOption* FleeED;

IMenu* FarmMenu;
IMenuOption* FarmQ;
IMenuOption* FarmE;
IMenuOption* FarmEmin;
IMenuOption* FarmW;
IMenuOption* FarmWmin;

IMenu* KillstealMenu;
IMenuOption* KSQ;
IMenuOption* KSE;

IMenu* LastMenu;
IMenuOption* LastQ;

IMenu* Extras;
IMenuOption* ComboModeChange;
IMenuOption* KSModeChange;
IMenuOption* RStop;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

IUnit* WDagger;
IUnit* QDagger;

vector<IUnit*> DaggersGr;
IUnit* dagger1;
IUnit* dagger2;

double timer;
double current;
double damages = 0;
double Rtime;
int wdelay;
int ComboMode = 1;
int* ComboM = &ComboMode;

float endtime;
float KeyPre;

bool decider = false;
std::vector<std::string> const& Names = {"Safe", "Medium", "Risky", "Very Risky"};
std::vector<std::string> const& KSM = { "Safe", "Risky" };

Vec3 dag1Pos;
Vec3 dag2Pos;

Vec3 LPos;

void inline LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, true, false, (kCollidesWithYasuoWall));
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, true, false, kCollidesWithNothing);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, true, false, kCollidesWithNothing);
	R = GPluginSDK->CreateSpell2(kSlotR, kCircleCast, false, true, (kCollidesWithYasuoWall));
}

void inline Menu()
{
	MainMenu = GPluginSDK->AddMenu("A-Katarina");
	Katarina = MainMenu->AddMenu("Katarina");
	ComboMenu = Katarina->AddMenu("Combo");
	{
		ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
		ComboW = ComboMenu->CheckBox("Use W in Combo", true);
		ComboE = ComboMenu->CheckBox("Use E in Combo", true);
		ComboRkill = ComboMenu->CheckBox("R Only If Killable", true);
		ComboRHit = ComboMenu->CheckBox("R if x enemies are in range", true);
		ComboRhit = ComboMenu->AddFloat("R If Hits", 1, 5, 1);
		ComboStop = ComboMenu->CheckBox("Cancel R for KS", false);
	}

	HarrassMenu = Katarina->AddMenu("Harrass");
	{
		HarrassQ = HarrassMenu->CheckBox("Use Q in Harrass", true);
		HarrassW = HarrassMenu->CheckBox("Use W in Harrass", true);
		HarrassE = HarrassMenu->CheckBox("Use E in Harrass", true);
	}
	FarmMenu = Katarina->AddMenu("Farming");
	{
		FarmQ = FarmMenu->CheckBox("Lane Clear with Q", true);
		FarmW = FarmMenu->CheckBox("Lane Clear with W", true);
		FarmWmin = FarmMenu->AddFloat("^- If Hits ", 1, 6, 3);
		FarmE = FarmMenu->CheckBox("Lane Clear with E", true);
		FarmEmin = FarmMenu->AddFloat("^- If Dagger hits", 1, 6, 3);
	}

	LastMenu = Katarina->AddMenu("Last Hit");
	{
		LastQ = LastMenu->CheckBox("Last hit Q", true);
	}
	KillstealMenu = Katarina->AddMenu("Killsteal");
	{
		KSQ = KillstealMenu->CheckBox("Killsteal with Q", true);
		KSE = KillstealMenu->CheckBox("Killsteal with E", true);
	}
	FleeMenu = Katarina->AddMenu("Flee");
	{
		FleeKey = FleeMenu->AddKey("Flee", 'Z');
		FleeW = FleeMenu->CheckBox("Flee with W", true);
		FleeE = FleeMenu->CheckBox("Flee with E", true);
		FleeED = FleeMenu->CheckBox("^- Use E on Daggers", true);
	}
	Extras = Katarina->AddMenu("Extra");
	{
		ComboModeChange = Extras->AddKey("Change Combo Mode Key", 'T');
		KSModeChange = Extras->AddSelection("KS Mode", 0, KSM);
		RStop = Extras->CheckBox("Cancel R if No enemies in range", true);
	}
	DrawingMenu = Katarina->AddMenu("Drawings");
	{
		DrawQRange = DrawingMenu->CheckBox("Draw Q Range", true);
		DrawERange = DrawingMenu->CheckBox("Draw E Range", true);
		DrawRRange = DrawingMenu->CheckBox("Draw R Range", true);
		DrawPriority = DrawingMenu->CheckBox("Draw Combo Type", true);
	}
}

inline int CountEnemy(Vec3 Location, int range)//From Kornis <3
{
	int Count = 0;

	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if ((Enemy->GetPosition() - Location).Length2D() < range && Enemy->IsValidTarget() && !Enemy->IsDead())
		{
			Count++;
		}
	}
	return (Count);
}

void inline Flee() //From Kornis <3
{
	if (!GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused())
	{
		GGame->IssueOrder(GEntityList->Player(), kMoveTo, GGame->CursorPosition());
		if (FleeW->Enabled() && W->IsReady())
		{
			W->CastOnPlayer();
		}
		if (FleeE->Enabled() && E->IsReady())
		{
			for (auto target : GEntityList->GetAllHeros(true, false))
			{
				if (target != GEntityList->Player())
				{
					if (target->IsValidTarget() && target->IsValidTarget(GEntityList->Player(), E->Range()) && !target->IsDead())
					{
						if ((target->GetPosition() - GGame->CursorPosition()).Length2D() < 200)
						{
							E->CastOnUnit(target);
						}
					}
				}
			}
			for (auto Minion : GEntityList->GetAllMinions(true, true, true))
			{
				if (!Minion->IsDead() && Minion != nullptr && Minion->IsValidTarget())
				{
					if ((Minion->GetPosition() - GGame->CursorPosition()).Length2D() < 200)
					{
						E->CastOnUnit(Minion);
					}
				}
			}
		}
		if (FleeED->Enabled())
		{
			for (auto dagger : GEntityList->GetAllUnits())
			{
				if (std::string(dagger->GetObjectName()) == "HiddenMinion")
				{
					if ((std::string(dagger->GetObjectName()).find("Katarina_Base_Q") || std::string(dagger->GetObjectName()).find("Katarina_Base_Dagger")) && dagger != nullptr && !dagger->IsDead() && dagger->IsValidObject() && dagger->GetTeam() == GEntityList->Player()->GetTeam() && !dagger->IsDead())
					{

						if ((dagger->GetPosition() - GGame->CursorPosition()).Length2D() < 200)
						{
							E->CastOnUnit(dagger);
						}
					}
				}
			}
		}

	}
}


inline int ChangePriority()
{
	if (GetAsyncKeyState(ComboModeChange->GetInteger()) && !GGame->IsChatOpen() && GGame->Time() > KeyPre)
	{
		if (ComboMode == 1)
		{
			ComboMode = 2;
			KeyPre = GGame->Time() + 0.250;
		}
		else if (ComboMode == 2)
		{
			ComboMode = 3;
			KeyPre = GGame->Time() + 0.250;
		}
		else if (ComboMode == 3)
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

void inline StopR()
{
	float endtime;
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range() + 250);
	if (GEntityList->Player()->IsCastingImportantSpell(&endtime) && CountEnemy(GEntityList->Player()->GetPosition(), R->Range()) == 0)
	{
		if (enemy != nullptr)
		{
			LPos = enemy->GetPosition();
		}
		Vec3 positions = Extend(GEntityList->Player()->GetPosition(), LPos, 50);
		GOrbwalking->Orbwalk(GEntityList->Player(), positions);
	}
}

void inline Combo()
{
	float endtime;
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range() + 400);

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			Q->CastOnUnit(enemy);
		}

		if (ComboW->Enabled() && W->IsReady() && player->IsValidTarget(enemy, 250) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			W->CastOnPlayer();
		}

		if (ComboE->Enabled() && E->IsReady() && player->IsValidTarget(enemy, E->Range() + 400) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			if (((dagger1 != nullptr) || (dagger2 != nullptr) || (QDagger != nullptr) || (WDagger != nullptr)) && current - timer > 50 && Distance(player, enemy) <= E->Range())
			{
				if (dagger1 == nullptr && dagger2 == nullptr)
				{
					E->CastOnUnit(enemy);
				}
				if (dagger1 != nullptr)
				{
					int bound = enemy->BoundingRadius();
					Vec3 enemyPos = enemy->GetPosition();
					Vec3 dagger1Pos = dagger1->GetPosition();
					Vec3 cast = Extend(enemyPos, dagger1Pos, (enemy->BoundingRadius()));
					E->CastOnPosition(cast);
				}
				if (dagger2 != nullptr)
				{
					int bound = enemy->BoundingRadius();
					Vec3 enemyPos = enemy->GetPosition();
					Vec3 dagger2Pos = dagger2->GetPosition();
					Vec3 cast = Extend(enemyPos, dagger2Pos, (enemy->BoundingRadius()));
					E->CastOnPosition(cast);
				}
			}
			if (dagger1 != nullptr && Distance(dagger1, player) <= E->Range() && Distance (dagger1, enemy) < Distance(player, enemy) && Distance(enemy, player) >E->Range())
			{
				E->CastOnUnit(dagger1);
			}
			if (dagger2 != nullptr && Distance(dagger2, player) <= E->Range() && Distance(dagger2, enemy) < Distance(player, enemy) && Distance(enemy, player) > E->Range())
			{
				E->CastOnUnit(dagger2);
			}
		}
	}
	if (ComboRkill->Enabled() && R->IsReady())
	{
		if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && enemy->IsHero() && player->IsValidTarget(enemy, R->Range() - 50))
		{
			if (ComboRkill->Enabled())
			{
				double RDamage = GDamage->GetSpellDamage(GEntityList->Player(), enemy, kSlotR);
				if ((RDamage * 12) >= enemy->GetHealth())
				{
					if (E->IsReady())
					{
						E->CastOnUnit(enemy);
					}
					R->CastOnPlayer();
					decider = true;
					Rtime = GGame->TickCount();
				}
			}
		}
	}
	if (ComboRHit->Enabled() && R->IsReady())
	{
		if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && player->IsValidTarget(enemy, R->Range() - 50))
		{
			if (ComboRhit->GetFloat() <= CountEnemy(GEntityList->Player()->GetPosition(), R->Range() - 50))
			{
				if (E->IsReady())
				{
					E->CastOnUnit(enemy);
				}
				R->CastOnPlayer();
				decider = true;
				Rtime = GGame->TickCount();
			}

		}
	}
}

void inline Combo2()
{
	float endtime;
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range() + 400);

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			Q->CastOnUnit(enemy);
		}

		if (ComboW->Enabled() && W->IsReady() && player->IsValidTarget(enemy, 250) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			W->CastOnPlayer();
		}

		if (ComboE->Enabled() && E->IsReady() && player->IsValidTarget(enemy, E->Range() + 400) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			if ((((dagger1 != nullptr && Distance(dagger1, enemy) <= 350 && Distance(player, enemy) <= E->Range()) || ((dagger2 != nullptr && Distance(dagger2, enemy) <= 350 && Distance(player, enemy) <= E->Range()))) || (QDagger != nullptr) && !GEntityList->Player()->IsCastingImportantSpell(&endtime)) && current - timer > 50 )
			{
				if (dagger1 != nullptr)
				{
					int bound = enemy->BoundingRadius();
					Vec3 enemyPos = enemy->GetPosition();
					Vec3 dagger1Pos = dagger1->GetPosition();
					Vec3 cast = Extend(enemyPos, dagger1Pos, (enemy->BoundingRadius()));
					E->CastOnPosition(cast);
				}
				if (dagger2 != nullptr)
				{
					int bound = enemy->BoundingRadius();
					Vec3 enemyPos = enemy->GetPosition();
					Vec3 dagger2Pos = dagger2->GetPosition();
					Vec3 cast = Extend(enemyPos, dagger2Pos, (enemy->BoundingRadius()));
					E->CastOnPosition(cast);
				}
				if (dagger1 == nullptr && dagger2 == nullptr)
				{
					E->CastOnUnit(enemy);
				}
			}
			if ((((dagger1 != nullptr && Distance(dagger1, enemy) <= 400 && Distance(player, enemy) > E->Range() && Distance(player, dagger1) <= E->Range()) || ((dagger2 != nullptr && Distance(dagger2, enemy) <= 400 && Distance(player, enemy) > E->Range() && Distance(player, dagger2) <= E->Range()))) || (QDagger != nullptr) && !GEntityList->Player()->IsCastingImportantSpell(&endtime)) && current - timer > 50)
			{
				if (dagger1 != nullptr)
				{
					E->CastOnUnit(dagger1);
				}
				if (dagger2 != nullptr)
				{
					E->CastOnUnit(dagger2);
				}
			}
		}
	}
	if (ComboRkill->Enabled() && R->IsReady())
	{
		if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && enemy->IsHero() && player->IsValidTarget(enemy, R->Range() - 50))
		{
			if (ComboRkill->Enabled())
			{
				double RDamage = GDamage->GetSpellDamage(GEntityList->Player(), enemy, kSlotR);
				if ((RDamage * 12) >= enemy->GetHealth())
				{
					if (E->IsReady())
					{
						E->CastOnUnit(enemy);
					}
					R->CastOnPlayer();
					decider = true;
					Rtime = GGame->TickCount();
				}
			}
		}
	}
	if (ComboRHit->Enabled() && R->IsReady())
	{
		if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && player->IsValidTarget(enemy, R->Range() - 50))
		{
			if (ComboRhit->GetFloat() <= CountEnemy(GEntityList->Player()->GetPosition(), R->Range() - 50))
			{
				if (E->IsReady())
				{
					E->CastOnUnit(enemy);
				}
				R->CastOnPlayer();
				decider = true;
				Rtime = GGame->TickCount();
			}

		}
	}
}

void inline Combo3()
{
	float endtime;
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			Q->CastOnUnit(enemy);
		}

		if (ComboW->Enabled() && W->IsReady() && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && player->IsValidTarget(enemy, 250) && decider == false)
		{
			W->CastOnPlayer();
		}

		if (ComboE->Enabled() && E->IsReady())
		{
			if (((dagger1 != nullptr && (Distance(dagger1, enemy) <= 400 && Distance(player, dagger1) <= E->Range())) || ((dagger2 != nullptr && Distance(dagger2, enemy) <= 400 && Distance(player, dagger2) <= E->Range()))) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
			{
				if (dagger1 != nullptr)
				{
					E->CastOnUnit(dagger1);
				}
				if (dagger2 != nullptr)
				{
					E->CastOnUnit(dagger2);
				}
			}
		}
		if (ComboRkill->Enabled() && R->IsReady())
		{
			if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && enemy->IsHero() && player->IsValidTarget(enemy, R->Range() - 50))
			{
				if (ComboRkill->Enabled())
				{
					double RDamage = GDamage->GetSpellDamage(GEntityList->Player(), enemy, kSlotR);
					if ((RDamage * 12) >= enemy->GetHealth())
					{
						if (E->IsReady())
						{
							E->CastOnUnit(enemy);
						}
						R->CastOnPlayer();
						decider = true;
						Rtime = GGame->TickCount();
					}
				}
			}
		}
		if (ComboRHit->Enabled() && R->IsReady())
		{
			if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && player->IsValidTarget(enemy, R->Range() - 50))
			{
				if (ComboRhit->GetFloat() <= CountEnemy(GEntityList->Player()->GetPosition(), R->Range() - 50))
				{
					if (E->IsReady())
					{
						E->CastOnUnit(enemy);
					}
					R->CastOnPlayer();
					decider = true;
					Rtime = GGame->TickCount();
				}

			}
		}
	}
}

void inline Combo4()
{
	float endtime;
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range() + 400);

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (ComboQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			Q->CastOnUnit(enemy);
		}

		if (ComboW->Enabled() && W->IsReady() && player->IsValidTarget(enemy, 250) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			W->CastOnPlayer();
		}

		if (ComboE->Enabled() && E->IsReady() && player->IsValidTarget(enemy, E->Range() + 400) && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && decider == false)
		{
			if (Distance(player, enemy) <= E->Range())
			{
				if (dagger1 == nullptr && dagger2 == nullptr)
				{
					E->CastOnUnit(enemy);
				}
				if (dagger1 != nullptr)
				{
					int bound = enemy->BoundingRadius();
					Vec3 enemyPos = enemy->GetPosition();
					Vec3 dagger1Pos = dagger1->GetPosition();
					Vec3 cast = Extend(enemyPos, dagger1Pos, (enemy->BoundingRadius()));
					E->CastOnPosition(cast);
				}
				if (dagger2 != nullptr)
				{
					int bound = enemy->BoundingRadius();
					Vec3 enemyPos = enemy->GetPosition();
					Vec3 dagger2Pos = dagger2->GetPosition();
					Vec3 cast = Extend(enemyPos, dagger2Pos, (enemy->BoundingRadius()));
					E->CastOnPosition(cast);
				}
			}
			if ((dagger1 != nullptr && Distance(enemy, player) > E->Range() && Distance(player, dagger1) <= E->Range() && Distance(enemy, dagger1) < Distance(player, enemy)) || (dagger2 != nullptr && Distance(enemy, player) > E->Range() && Distance(player, dagger2) <= E->Range()) && Distance(enemy, dagger2) < Distance(player, enemy))
			{
				if (dagger1 != nullptr)
				{
					E->CastOnUnit(dagger1);
				}
				if (dagger2 != nullptr)
				{
					E->CastOnUnit(dagger2);
				}
			}
		}
	}
	if (ComboRkill->Enabled() && R->IsReady())
	{
		if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && enemy->IsHero() && player->IsValidTarget(enemy, R->Range() - 50))
		{
			if (ComboRkill->Enabled())
			{
				double RDamage = GDamage->GetSpellDamage(GEntityList->Player(), enemy, kSlotR);
				if ((RDamage * 12) >= enemy->GetHealth())
				{
					if (E->IsReady())
					{
						E->CastOnUnit(enemy);
					}
					R->CastOnPlayer();
					decider = true;
					Rtime = GGame->TickCount();
				}
			}
		}
	}
	if (ComboRHit->Enabled() && R->IsReady())
	{
		if (enemy != nullptr && enemy->IsValidTarget() && !enemy->IsDead() && player->IsValidTarget(enemy, R->Range() - 50))
		{
			if (ComboRhit->GetFloat() <= CountEnemy(GEntityList->Player()->GetPosition(), R->Range() - 50))
			{
				if (E->IsReady())
				{
					E->CastOnUnit(enemy);
				}
				R->CastOnPlayer();
				decider = true;
				Rtime = GGame->TickCount();
			}

		}
	}
}

void inline Harrass()
{
	float endtime;
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (HarrassQ->Enabled() && Q->IsReady() && player->IsValidTarget(enemy, Q->Range()) && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
		{
			Q->CastOnUnit(enemy);
		}

		if (HarrassW->Enabled() && W->IsReady() && !GEntityList->Player()->IsCastingImportantSpell(&endtime) && player->IsValidTarget(enemy, 250))
		{
			W->CastOnPlayer();
		}

		if (HarrassE->Enabled() && E->IsReady())
		{
			if (((dagger1 != nullptr && (Distance(dagger1, enemy) <= 400 && Distance(player, dagger1) <= E->Range())) || ((dagger2 != nullptr && Distance(dagger2, enemy) <= 400 && Distance(player, dagger2) <= E->Range()))) && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
			{
				if (dagger1 != nullptr)
				{
					E->CastOnUnit(dagger1);
				}
				if (dagger2 != nullptr)
				{
					E->CastOnUnit(dagger2);
				}
			}
		}
	}
}

void inline LastHit() //From Kornis <3
{
	float endtime;
	for (auto Minion : GEntityList->GetAllMinions(false, true, true))
	{
		if (!Minion->IsDead() && Minion != nullptr)
		{
			if (LastQ->Enabled() && Q->IsReady() && Minion->IsValidTarget(GEntityList->Player(), Q->Range()) && GDamage->GetSpellDamage(GEntityList->Player(), Minion, kSlotQ) >= Minion->GetHealth() && (Distance(Minion , GEntityList->Player()) > GEntityList->Player()->AttackRange()) && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
			{
				Q->CastOnUnit(Minion);
			}
		}
	}
}

inline static int GetMinionsW(float range) //From Kornis <3
{
	auto minions = GEntityList->GetAllMinions(false, true, false);
	auto minionsInRange = 0;
	for (auto minion : minions)
	{
		if (minion != nullptr && minion->IsValidTarget() && minion->IsEnemy(GEntityList->Player()) && !minion->IsDead())
		{
			auto minionDistance = (minion->GetPosition() - GEntityList->Player()->GetPosition()).Length2D();
			if (minionDistance < range)
			{
				minionsInRange++;
			}
		}
	}
	return minionsInRange;
}

inline static int CountMinionsNearMe(IUnit* Source, float range) //From Kornis <3
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

void inline Farm() // From Kornis <3
{
	float endtime;
	for (auto Minion : GEntityList->GetAllMinions(false, true, true))
	{
		if (FarmQ->Enabled() && Q->IsReady() && Minion->IsValidTarget() && !Minion->IsDead() && Minion->IsValidTarget(GEntityList->Player(), Q->Range()) && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
		{
			Q->CastOnUnit(Minion);
		}
		if (FarmW->Enabled() && W->IsReady() && Minion->IsValidTarget() && !Minion->IsDead() && Minion->IsValidTarget(GEntityList->Player(), 350) && GetMinionsW(350) >= FarmWmin->GetFloat() && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
		{
			if (W->CastOnPlayer())
			{
				wdelay = GGame->TickCount() + 1200;
			}
		}
		if (FarmE->Enabled() && E->IsReady() && Minion->IsValidTarget() && !Minion->IsDead() && Minion->IsValidTarget(GEntityList->Player(), E->Range()))
		{

			for (auto dagger : GEntityList->GetAllUnits())
			{
				if (std::string(dagger->GetObjectName()) == "HiddenMinion")
				{
					if ((std::string(dagger->GetObjectName()).find("Katarina_Base_Q") || std::string(dagger->GetObjectName()).find("Katarina_Base_Dagger")) && !dagger->IsDead() && dagger->IsValidObject() && dagger->GetTeam() == GEntityList->Player()->GetTeam() && !dagger->IsDead())
					{
						if (dagger != nullptr)
						{
							if (wdelay < GGame->TickCount() && CountMinionsNearMe(dagger, 380) >= FarmEmin->GetFloat() && (GEntityList->Player()->GetPosition() - dagger->GetPosition()).Length2D() <= E->Range() && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
							{
								E->CastOnPosition(dagger->GetPosition());
							}
						}
					}
				}
			}
		}
	}
}

void inline Killsteal()
{
	float endtime;
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		auto QDamage = GDamage->GetSpellDamage(GEntityList->Player(), Enemy, kSlotQ);
		auto EDamage = GDamage->GetSpellDamage(GEntityList->Player(), Enemy, kSlotE);
		auto AADamage = GDamage->GetAutoAttackDamage(GEntityList->Player(), Enemy, false);
		auto passive = damages;

		if (Enemy != nullptr && !Enemy->IsDead() & Enemy->IsValidTarget() && ComboStop->Enabled())
		{
			if (KSQ->Enabled() && Q->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), Q->Range()) && QDamage > Enemy->GetHealth())
			{
				Q->CastOnTarget(Enemy);
			}
			if (KSE->Enabled() && E->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), E->Range()) && EDamage + AADamage > Enemy->GetHealth())
			{
				E->CastOnTarget(Enemy);
			}
		}
		if (Enemy != nullptr && !Enemy->IsDead() & Enemy->IsValidTarget() && !ComboStop->Enabled())
		{
			if (KSQ->Enabled() && Q->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), Q->Range()) && QDamage > Enemy->GetHealth() && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
			{
				Q->CastOnTarget(Enemy);
			}
			if (KSE->Enabled() && E->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), E->Range()) && EDamage + AADamage > Enemy->GetHealth() && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
			{
				E->CastOnTarget(Enemy);
			}
		}
	}
}

void inline Killsteal2()
{
	float endtime;
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		auto QDamage = GDamage->GetSpellDamage(GEntityList->Player(), Enemy, kSlotQ);
		auto EDamage = GDamage->GetSpellDamage(GEntityList->Player(), Enemy, kSlotE);
		auto AADamage = GDamage->GetAutoAttackDamage(GEntityList->Player(), Enemy, false);
		auto passive = damages;

		if (Enemy != nullptr && !Enemy->IsDead() & Enemy->IsValidTarget() && ComboStop->Enabled())
		{
			if (KSQ->Enabled() && Q->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), Q->Range()) && QDamage + AADamage > Enemy->GetHealth())
			{
				Q->CastOnTarget(Enemy);
			}
			if (KSE->Enabled() && E->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), E->Range()) && EDamage + QDamage + AADamage > Enemy->GetHealth())
			{
				E->CastOnTarget(Enemy);
			}
		}
		if (Enemy != nullptr && !Enemy->IsDead() & Enemy->IsValidTarget() && !ComboStop->Enabled())
		{
			if (KSQ->Enabled() && Q->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), Q->Range()) && QDamage + AADamage > Enemy->GetHealth() && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
			{
				Q->CastOnTarget(Enemy);
			}
			if (KSE->Enabled() && E->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), E->Range()) && EDamage + QDamage + AADamage > Enemy->GetHealth() && !GEntityList->Player()->IsCastingImportantSpell(&endtime))
			{
				E->CastOnTarget(Enemy);
			}
		}
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	if (RStop->Enabled())
	{
		StopR();
	}
	if (current - Rtime >= 100 && decider)
	{
		decider = false;
	}
	if (KSModeChange->GetInteger() == 0)
	{
		Killsteal();
	}
	if (KSModeChange->GetInteger() == 1)
	{
		Killsteal2();
	}
	ChangePriority();

	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && !GGame->IsChatOpen())
	{
		Harrass();
	}

	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen())
	{
		if (ComboMode == 0)
		{
			Combo3();
		}
		if (ComboMode == 1)
		{
			Combo2();
		}
		if (ComboMode == 2)
		{
			Combo();
		}
		if (ComboMode== 3)
		{
			Combo4();
		}
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear && !GGame->IsChatOpen())
	{
		Farm();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLastHit && !GGame->IsChatOpen())
	{
		LastHit();
	}
	if (GetAsyncKeyState(FleeKey->GetInteger()) & 0x8000 && !GGame->IsChatOpen())
	{
		Flee();
	}

	if (E->IsReady() == false)
	{
		timer = GGame->TickCount();
	}

	current = GGame->TickCount();
}

PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{
	auto the = Source->GetObjectName();

	if (Source == nullptr)
	{
		return;
	}

	if (Contains(the, "Katarina_Base_Dagger_Ground"))
	{
		if (dagger1 != nullptr)
		{
			dagger2 = Source;
			dag2Pos = dagger2->GetPosition();
		}

		if (dagger1 == nullptr)
		{
			dagger1 = Source;
			dag1Pos = dagger1->GetPosition();
		}
	}

	if (Source->IsMissile() && GMissileData->GetCaster(Source) == GEntityList->Player())
	{
		auto data = GMissileData->GetName(Source);
		if (Contains(data, "KatarinaQ"))
		{
			QDagger = Source;
		}

		if (Equals(data, "KatarinaWDaggerArc"))
		{
			WDagger = Source;
		}
	}
}

PLUGIN_EVENT(void) OnDestroyObject(IUnit* Source)
{
	if (Source == nullptr)
	{
		return;
	}

	auto the = Source->GetObjectName();


	if (Contains(the, "Katarina_Base_Dagger_Ground"))
	{
		if (dagger1 == nullptr)
		{
			dagger2 = nullptr;
		}

		if (dagger1 != nullptr)
		{
			dagger1 = nullptr;
		}
	}

	if (Source->IsMissile() && GMissileData->GetCaster(Source)->GetNetworkId() == GEntityList->Player()->GetNetworkId())
	{
		auto data = GMissileData->GetName(Source);
		if (Contains(data, "KatarinaQDaggerArc"))
		{
			QDagger = nullptr;
		}

		if (Equals(data, "KatarinaWDaggerArc"))
		{
			WDagger = nullptr;
		}

	}
}

PLUGIN_EVENT(void) OnRender()
{
	if (DrawQRange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }
	if (DrawRRange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }
	if (DrawERange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }
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
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}