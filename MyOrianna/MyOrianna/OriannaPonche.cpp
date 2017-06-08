#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <sstream>
#include <iostream>
#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include <math.h>
#define PI 3.14159265

PluginSetup("OriannaPonche");

IMenu* MainMenu;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboEQ;
IMenuOption* ComboAR;
IMenuOption* ComboR;

IMenu* RMenu;
IMenuOption* RRange;
IMenuOption* RAuto;
IMenuOption* RInitiator;
IMenuOption* RCancel;
IMenuOption* RMin;

IMenu* ShieldMenu;
IMenuOption* ShieldAuto;
IMenuOption* ShieldCombo;
IMenuOption* ShieldHarass;
IMenuOption* ShieldDamage;
IMenuOption* ShieldLife;
IMenuOption* ShieldDisabled;

IMenu* FleeMenu;
IMenuOption* FleeW;
IMenuOption* FleeE;
IMenuOption* FleeKey;

IMenu* HarassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassW;
IMenuOption* HarassEQ;
IMenuOption* HarassMin;
IMenuOption* HarassMana;
IMenuOption* HarassToggleKey;
IMenuOption* HarassDisabled;

IMenu* LaneClearMenu;
IMenuOption* LaneClearQ;
IMenuOption* LaneClearW;
IMenuOption* LaneClearMin;
IMenuOption* LaneClearMana;
IMenuOption* LaneClearDisabled;

IMenu* KillStealMenu;
IMenuOption* KillStealQ;
IMenuOption* KillStealW;
IMenuOption* KillStealE;
IMenuOption* KillStealR;
IMenuOption* KillStealIgnite;
IMenuOption* KillStealMin;
IMenuOption* KillStealDisabled;

IMenu* DrawMenu;
IMenu* DrawQMenu;
IMenuOption* DrawQ;
IMenuOption* DrawQColor;
IMenu* DrawWMenu;
IMenuOption* DrawW;
IMenuOption* DrawWEnabled;
IMenuOption* DrawWColor;
IMenu* DrawEMenu;
IMenuOption* DrawE;
IMenuOption* DrawEColor;
IMenu* DrawRMenu;
IMenuOption* DrawR;
IMenuOption* DrawRColor;
IMenu* DrawHarassMenu;
IMenuOption* DrawHarass;
IMenuOption* DrawHarassColor;
IMenu* DrawBallMenu;
IMenuOption* DrawBall;
IMenuOption* DrawBallColor;
IMenuOption* DrawCD;
IMenuOption* DrawDisabled;

IMenu* MiscMenu;
IMenuOption* MiscEQ;
IMenuOption* MiscW;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
ISpell2* QR;
ISpell2* Ignite = nullptr;
IUnit* Ball;

Vec3 BallLocation;
Vec2 colliPos;
int IgniteSlot = 1;
bool HarassToggle = false;
int LastChanged = 0;
IUnit *Player;

Vec3 deneme;
double delayers = 0;
double delayer3 = 0;
Vec3 epos123;
Vec3 ppos123;
Vec3 direction5;
Vec3 empt;
bool stopper = false;
bool stoper = false;
bool check = false;
bool lol = false;

std::vector<Vec3> epos;
std::vector<double> travelDistances;
double delayer = 0;
double delayer1 = 0;
double delayer2 = 0;
double delayer5 = 0;
double delayer6 = 0;
double D = 0;
double averageDist = 0;



void LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, kCollidesWithYasuoWall);
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, kCollidesWithNothing);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, true, false, kCollidesWithNothing);
	R = GPluginSDK->CreateSpell2(kSlotR, kCircleCast, false, true, kCollidesWithNothing);
	QR = GPluginSDK->CreateSpell2(kSlotQ, kCircleCast, false, true, kCollidesWithYasuoWall);

	Q->SetSkillshot(0.0001f, 145.f, 1100.f, 815.f);
	W->SetSkillshot(0.25f, 250.f, HUGE_VAL, 1250.f);
	E->SetSkillshot(0.0f, 0.f, 1800.f, 1095.f);
	R->SetSkillshot(0.5f, 410.f, HUGE_VAL, 1250.f);
	QR->SetSkillshot(0.5f, 410.f, 1200.f, 815.f);
}

void LoadMenu()
{
	auto nb_champ = 0;
	MainMenu = GPluginSDK->AddMenu("OriannaPonche");

	ComboMenu = MainMenu->AddMenu("Combo Manager");
	ComboQ = ComboMenu->CheckBox("Use Q", true);
	ComboW = ComboMenu->CheckBox("Use W", true);
	ComboE = ComboMenu->CheckBox("Use E", true);
	ComboEQ = ComboMenu->CheckBox("Use EQ", true);
	//ComboAR = ComboMenu->CheckBox("Automatic Return", false);
	ComboR = ComboMenu->CheckBox("Use R", true);

	RMenu = MainMenu->AddMenu("R Manager");
	RAuto = RMenu->CheckBox("Auto R", true);
	RInitiator = RMenu->CheckBox("Use Initiator", true);
	RCancel = RMenu->CheckBox("Cancel R", true);
	RRange = RMenu->AddInteger("R Range", 0, 410, 380);
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		RMenu->AddInteger(Enemy->ChampionName(), 0, 3, 1);
		nb_champ++;
	}
	RMin = RMenu->AddInteger("Min to R", 1, nb_champ * 3, 2);

	ShieldMenu = MainMenu->AddMenu("Shield Manager");
	ShieldAuto = ShieldMenu->CheckBox("Use Auto Shield", false);
	ShieldCombo = ShieldMenu->CheckBox("Use in Combo", true);
	ShieldHarass = ShieldMenu->CheckBox("Use in Harass", true);
	ShieldDamage = ShieldMenu->AddInteger("Min Damage %", 1, 100, 10);
	ShieldLife = ShieldMenu->AddInteger("Min Health %", 1, 100, 90);
	ShieldDisabled = ShieldMenu->CheckBox("Disable All", false);

	FleeMenu = MainMenu->AddMenu("Flee Manager");
	FleeW = FleeMenu->CheckBox("Use W", true);
	FleeE = FleeMenu->CheckBox("Use E", true);
	FleeKey = FleeMenu->AddKey("Key", 84);

	HarassMenu = MainMenu->AddMenu("Harass Manager");
	HarassQ = HarassMenu->CheckBox("Use Q", true);
	HarassW = HarassMenu->CheckBox("Use W", true);
	HarassEQ = HarassMenu->CheckBox("Use EQ", false);
	HarassMin = HarassMenu->CheckBox("Use Min Mana", true);
	HarassMana = HarassMenu->AddInteger("Min Mana %", 1, 100, 60);
	HarassToggleKey = HarassMenu->AddKey("Key", 87);
	HarassDisabled = HarassMenu->CheckBox("Disable All", false);

	LaneClearMenu = MainMenu->AddMenu("LaneClear Manager");
	LaneClearQ = LaneClearMenu->CheckBox("Use Q", true);
	LaneClearW = LaneClearMenu->CheckBox("Use W", true);
	LaneClearMin = LaneClearMenu->CheckBox("Use Min Mana", true);
	LaneClearMana = LaneClearMenu->AddInteger("Min Mana %", 1, 100, 60);
	LaneClearDisabled = LaneClearMenu->CheckBox("Disable All", false);

	KillStealMenu = MainMenu->AddMenu("KillSteal Manager");
	KillStealQ = KillStealMenu->CheckBox("Use Q", true);
	KillStealW = KillStealMenu->CheckBox("Use W", true);
	KillStealE = KillStealMenu->CheckBox("Use E", true);
	KillStealR = KillStealMenu->CheckBox("Use R", true);
	if (strcmp(Player->GetSpellName(kSummonerSlot1), "SummonerDot") == 0)
	{
		Ignite = GPluginSDK->CreateSpell2(kSummonerSlot1, kTargetCast, false, false, kCollidesWithNothing);
		Ignite->SetOverrideRange(600);
		Ignite->SetOverrideDelay(5);
		KillStealIgnite = KillStealMenu->CheckBox("Use Ignite", true);
	}
	if (strcmp(Player->GetSpellName(kSummonerSlot2), "SummonerDot") == 0)
	{
		IgniteSlot = 2;
		Ignite = GPluginSDK->CreateSpell2(kSummonerSlot2, kTargetCast, false, false, kCollidesWithNothing);
		Ignite->SetOverrideRange(600);
		Ignite->SetOverrideDelay(5);
		KillStealIgnite = KillStealMenu->CheckBox("Use Ignite", true);
	}
	KillStealMin = KillStealMenu->AddInteger("Min to Hit With R", 1, 5, 2);
	KillStealDisabled = KillStealMenu->CheckBox("Disable All", false);

	DrawMenu = MainMenu->AddMenu("Draw Manager");
	DrawQMenu = DrawMenu->AddMenu("Q Draw Manager");
	DrawQ = DrawQMenu->CheckBox("Enabled", true);
	DrawQColor = DrawQMenu->AddColor("Q Color Picker", 255, 255, 0, 255);
	DrawWMenu = DrawMenu->AddMenu("W Draw Manager");
	DrawW = DrawWMenu->CheckBox("Enabled", true);
	DrawWColor = DrawWMenu->AddColor("W Color Picker", 255, 255, 0, 255);
	DrawEMenu = DrawMenu->AddMenu("E Draw Manager");
	DrawE = DrawEMenu->CheckBox("Enabled", true);
	DrawEColor = DrawEMenu->AddColor("E Color Picker", 255, 255, 0, 255);
	DrawRMenu = DrawMenu->AddMenu("R Draw Manager");
	DrawR = DrawRMenu->CheckBox("Enabled", true);
	DrawRColor = DrawRMenu->AddColor("R Color Picker", 255, 255, 0, 255);
	DrawHarassMenu = DrawMenu->AddMenu("Harass Draw Manager");
	DrawHarass = DrawHarassMenu->CheckBox("Enabled", true);
	DrawHarassColor = DrawHarassMenu->AddColor("Harass Color Picker", 255, 255, 0, 255);
	DrawBallMenu = DrawMenu->AddMenu("Ball Draw Manager");
	DrawBall = DrawBallMenu->CheckBox("Enabled", true);
	DrawBallColor = DrawBallMenu->AddColor("Ball Color Picker", 255, 255, 0, 255);
	DrawCD = DrawMenu->CheckBox("Disable on CD", true);
	DrawDisabled = DrawMenu->CheckBox("Disable All", false);

	MiscMenu = MainMenu->AddMenu("Misc Manager");
	MiscEQ = MiscMenu->AddInteger("Min Range to EQ", 0, 300, 100);
	MiscW = MiscMenu->AddInteger("W Range", 0, 250, 220);
}

Vec2 LinearPrediction()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	double h = BallLocation.x;
	double k = BallLocation.y;
	double SS = Q->Speed();
	double temporar = 1000;
	int t;
	std::vector<double> Ds;

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && player->IsValidTarget(enemy, Q->Range()))
	{
		if (!enemy->IsMoving())
		{
			return ToVec2(enemy->GetPosition());
		}


		if (enemy->IsDashing())
		{
			return ToVec2(empt);
		}

		double ES = enemy->MovementSpeed();

		if (GGame->TickCount() >= delayer)
		{
			epos.push_back(enemy->GetPosition());
			delayer = GGame->TickCount() + 75;
		}

		if (epos.size() > 3 && stoper == false)
		{
			if (ToVec2((epos[epos.size() - 2] - epos[epos.size() - 3]).VectorNormalize()) != ToVec2((epos[epos.size() - 3] - epos[epos.size() - 4]).VectorNormalize()))
			{
				Vec2 loc1 = ToVec2(enemy->GetPosition() + ((epos[epos.size() - 2] - epos[epos.size() - 3]).VectorNormalize())) * 1000;
				Vec2 loc2 = ToVec2(enemy->GetPosition() + ((epos[epos.size() - 3] - epos[epos.size() - 4]).VectorNormalize())) * 1000;
				if (Distance(loc1, loc2) > 10)
				{
					stoper = true;
					delayer2 = GGame->TickCount() + 300;
				}
				else
					return ToVec2(empt);
			}
			else
				return ToVec2(empt);
		}

		if (stoper = true && GGame->TickCount() <= delayer2)
		{
			Vec3 direction = (epos[epos.size() - 1] - epos[epos.size() - 2]).VectorNormalize();
			for (int i = 0; i < 3000; i++)
			{
				Vec3 EF = enemy->GetPosition() + direction*(ES / 1000)*i;
				D = abs(sqrt(pow((EF.x - h), 2) + pow(EF.y - k, 2)) - (SS / 1000)*i);
				Ds.push_back(D);
				if (Ds[Ds.size() - 1] <= temporar)
				{
					temporar = Ds[Ds.size() - 1];
				}
				else
				{
					t = i - 1;
					break;
				}
			}
			t = t + (Q->GetDelay() * 1000) + (GGame->Latency() / 1);
			Vec3 EFuture = enemy->GetPosition() + direction*(ES / 1000)*t;
			Vec3 fut = Extend(EFuture, enemy->GetPosition(), (Q->Radius() + enemy->BoundingRadius() - 150));
			return ToVec2(fut);
		}
		else
		{
			stoper = false;
			return ToVec2(empt);
		}

		//if (epos.size() > 1)
		//{
		//	Vec3 direction = (epos[epos.size() - 1] - epos[epos.size() - 2]).VectorNormalize();
		//	for (int i = 0; i < 2000; i++)
		//	{
		//		Vec3 EF = enemy->GetPosition() + direction*(ES / 1000)*i;
		//		D = abs(sqrt(pow((EF.x - h), 2) + pow(EF.y - k, 2)) - (SS / 1000)*i);
		//		Ds.push_back(D);
		//		if (Ds[Ds.size() - 1] <= temporar)
		//		{
		//			temporar = Ds[Ds.size() - 1];
		//		}
		//		else
		//		{
		//			t = i - 1;
		//			break;
		//		}
		//	}
		//	t = t + (Q->GetDelay() * 1000) + (GGame->Latency() / 1);
		//	Vec3 EFuture = enemy->GetPosition() + direction*(ES / 1000)*t;
		//	Vec3 fut = Extend(EFuture, enemy->GetPosition(), (Q->Radius() - 25));
		//	return fut;
		//}
		//else
		//	return empt;
	}
	else
		return ToVec2(empt);
}

PLUGIN_EVENT(void) OnRender()
{
	Vec4 Color;
	Vec2 BallPos;
	Vec2 HeroPos;
	Vec2 Pos;

	//GPluginSDK->GetRenderer()->DrawOutlinedCircle(ToVec3(LinearPrediction()), Pink(), 50);
	GPluginSDK->GetRenderer()->DrawOutlinedCircle(BallLocation, Pink(), 50);
	//GPluginSDK->GetRenderer()->DrawOutlinedCircle(deneme, Green(), 50);

	if (!DrawDisabled->Enabled())
	{
		if (DrawCD->Enabled())
		{
			if (Q->IsReady() && DrawQ->Enabled())
			{
				DrawQColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(Player->GetPosition(), Color, Q->Range());
			}
			if (W->IsReady() && DrawW->Enabled())
			{
				DrawWColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(BallLocation, Color, MiscW->GetInteger());
			}
			if (E->IsReady() && DrawE->Enabled())
			{
				DrawEColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(Player->GetPosition(), Color, E->Range());
			}
			if (R->IsReady() && DrawR->Enabled())
			{
				DrawRColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(BallLocation, Color, RRange->GetInteger());
			}
		}
		else
		{
			if (DrawQ->Enabled())
			{
				DrawQColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(Player->GetPosition(), Color, Q->Range());
			}
			if (DrawW->Enabled())
			{
				DrawWColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(BallLocation, Color, MiscW->GetInteger());
			}
			if (DrawE->Enabled())
			{
				DrawEColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(Player->GetPosition(), Color, E->Range());
			}
			if (DrawR->Enabled())
			{
				DrawRColor->GetColor(&Color);
				GRender->DrawOutlinedCircle(BallLocation, Color, RRange->GetInteger());
			}
		}
		if (DrawBall->Enabled())
		{
			GGame->Projection(BallLocation, &BallPos);
			GGame->Projection(Player->GetPosition(), &HeroPos);
			DrawBallColor->GetColor(&Color);
			GRender->DrawLine(BallPos, HeroPos, Color);
		}
		if (DrawHarass->Enabled() && HarassToggle && Player->GetHPBarPosition(Pos))
		{
			DrawHarassColor->GetColor(&Color);
			Pos.x += 20;
			Pos.y += 50;
			GRender->DrawText(Pos, Color, "Harass Toggle Enabled");
		}
	}
}

int IsEnoughR(Vec3 Location, int range)
{
	Vec3 Pos;
	auto Count = 0;

	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (Enemy != nullptr &&  Enemy->IsValidObject() && Enemy->IsVisible() && !Enemy->IsDead() && Enemy->IsValidTarget() && !Enemy->HasBuff("sionpassivezombie"))
		{
			GPrediction->GetFutureUnitPosition(Enemy, R->GetDelay(), true, Pos);
			if ((Pos - Location).Length2D() < range)
			{
				Count += RMenu->GetOption(Enemy->ChampionName())->GetInteger();
			}
		}
	}
	return (Count);
}

int IsEnough(Vec3 Location, int range)
{
	Vec3 Pos;
	auto Count = 0;

	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (Enemy != nullptr && Enemy->IsValidObject() && Enemy->IsVisible() && !Enemy->IsDead() && Enemy->IsValidTarget())
		{
			GPrediction->GetFutureUnitPosition(Enemy, W->GetDelay(), true, Pos);
			if ((Pos - Location).Length2D() <= range)
			{
				Count++;
			}
		}
	}
	return (Count);
}

float IsEnoughMinions(Vec3 Location, int range)
{
	Vec3 Pos;

	for (auto Minion : GEntityList->GetAllMinions(false, true, true))
	{
		if (Minion != nullptr && Minion->IsValidObject() && Minion->IsVisible() && !Minion->IsDead() && Minion->IsValidTarget())
		{
			GPrediction->GetFutureUnitPosition(Minion, W->GetDelay(), true, Pos);
			if ((Pos - Location).Length2D() < range)
			{
				return (true);
			}
		}
	}
	return (false);
}

void CastQTarget(IUnit* target)
{
	Q->SetFrom(BallLocation);
	Q->CastOnTarget(target, kHitChanceHigh);
	check = true;
	delayer3 = GGame->TickCount() + 1000;
}

void CastQPosition(Vec3 Position)
{
	Q->CastOnPosition(Position);
}

void CastW()
{
	W->CastOnPlayer();
}

void CastE(IUnit* target)
{
	E->CastOnTarget(target);
}

void CastR()
{
	R->CastOnPlayer();
}

void CheckEQ(Vec3 Pos)
{
	IUnit* BestAlly = nullptr;
	auto BestDistance = (BallLocation - Pos).Length2D();
	float FromAllyToTarget;
	float FromBallToTarget;

	if ((Pos - Player->GetPosition()).Length2D() > Q->Range())
		return;
	for (auto Ally : GEntityList->GetAllHeros(true, false))
	{
		if (Ally->IsValidObject() && Ally->IsVisible() && !Ally->IsDead() && Ally->IsValidTarget(Player, E->Range()))
		{
			FromAllyToTarget = (Ally->GetPosition() - Pos).Length2D();
			FromBallToTarget = (BallLocation - Pos).Length2D();
			if (FromAllyToTarget < BestDistance && FromAllyToTarget + MiscEQ->GetInteger() < FromBallToTarget)
			{
				BestAlly = Ally;
				BestDistance = FromAllyToTarget;
			}
		}
	}
	if (BestAlly != nullptr)
	{
		CastE(BestAlly);
	}
}

void CheckR()
{
	int Enemies;
	Vec3 Position;

	if (R->IsReady())
	{
		auto Min = RMin->GetInteger();

		if (Min <= IsEnoughR(BallLocation, RRange->GetInteger()))
		{
			CastR();
		}
		if (E->IsReady())
		{
			for (auto Ally : GEntityList->GetAllHeros(true, false))
			{
				if (!Ally->IsDead() && Ally->IsValidObject() && Ally->IsVisible() && Ally->IsValidTarget(Player, E->Range()) && Min <= IsEnoughR(Ally->GetPosition(), RRange->GetInteger()))
				{
					CastE(Ally);
				}
			}
		}
		if (Q->IsReady())
		{
			QR->SetOverrideRadius(RRange->GetInteger());
			QR->SetFrom(BallLocation);
			QR->FindBestCastPosition(false, true, Position, Enemies);
			if (Min <= IsEnoughR(Position, RRange->GetInteger()))
			{
				if (E->IsReady())
				{
					CheckEQ(Position);
				}
				CastQPosition(Position);
			}
		}
	}
}

void Combo()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (ComboR->Enabled() && R->IsReady() && !RAuto->Enabled())
	{
		CheckR();
	}
	if (target != nullptr)
	{
		if (ComboEQ->Enabled() && Q->IsReady() && E->IsReady())
		{
			CheckEQ(target->GetPosition());
		}
		if (ComboQ->Enabled() && Q->IsReady() && ToVec3(LinearPrediction()) != empt)
		{
			Q->CastOnPosition(ToVec3(LinearPrediction()));
			//check = true;
			//delayer6 = GGame->TickCount() + GGame->Latency();
			//delayer3 = GGame->TickCount() + 1000;
		}
	}
	if (ComboW->Enabled() && W->IsReady() && IsEnough(BallLocation, MiscW->GetInteger()) > 0)
	{
		CastW();
	}
	//if (ComboAR->Enabled() && E->IsReady() && !Q->IsReady())
	//{
	//	CastE(Player);
	//}
}

void Harass()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if ((Player->ManaPercent() >= HarassMana->GetInteger() || !HarassMin->Enabled()) && !HarassDisabled->Enabled())
	{
		if (target != nullptr)
		{
			if (HarassEQ->Enabled() && Q->IsReady() && E->IsReady())
			{
				CheckEQ(target->GetPosition());
			}
			if (HarassQ->Enabled() && Q->IsReady() && ToVec3(LinearPrediction()) != empt)
			{
				Q->CastOnPosition(ToVec3(LinearPrediction()));
			}
		}
		if (HarassW->Enabled() && W->IsReady() && IsEnough(BallLocation, MiscW->GetInteger()) > 0)
		{
			CastW();
		}
	}
}

void LaneClear()
{
	if ((Player->ManaPercent() > LaneClearMana->GetInteger() || !LaneClearMin->Enabled()) && !LaneClearDisabled->Enabled())
	{
		if (LaneClearQ->Enabled() && Q->IsReady())
		{
			for (auto Minion : GEntityList->GetAllMinions(false, true, true))
			{
				if (Minion != nullptr && !Minion->IsDead() && Minion->IsValidTarget(GEntityList->Player(), Q->Range()) && Minion->IsValidObject() && Minion->IsVisible())
				{
					CastQTarget(Minion);
				}
			}
		}
		if (LaneClearW->Enabled() && W->IsReady() && (IsEnoughMinions(BallLocation, MiscW->GetInteger()) || IsEnough(BallLocation, MiscW->GetInteger()) > 0))
		{
			CastW();
		}
	}
}

void GetBallLocation()
{
	for (auto Ally : GEntityList->GetAllHeros(true, false))
	{
		if (Ally->HasBuff("orianaghostself") || Ally->HasBuff("orianaghost"))
		{ 
			BallLocation = Ally->GetPosition();
		}
	}
}

void KillSteal()
{
	float EnemyHealth;

	if (!KillStealDisabled->Enabled()) 
	{ 
		for (auto Enemy : GEntityList->GetAllHeros(false, true))
		{
			if (!Enemy->IsDead() && Enemy->IsValidObject() && Enemy->IsVisible())
			{
				EnemyHealth = Enemy->GetHealth();
				if (KillStealW->Enabled() && W->IsReady() && Enemy->IsValidObject() && (Enemy->GetPosition() - BallLocation).Length2D() < MiscW->GetInteger() && GHealthPrediction->GetKSDamage(Enemy, kSlotW, W->GetDelay(), false) > EnemyHealth)
				{
					CastW();
				}
				if (Q->IsReady() && KillStealQ->Enabled() && Enemy->IsValidTarget(Player, Q->Range()) && GHealthPrediction->GetKSDamage(Enemy, kSlotQ, Q->GetDelay(), true) > EnemyHealth)
				{
					if (KillStealE->Enabled() && E->IsReady())
					{
						CheckEQ(Enemy->GetPosition());
					}
					Q->CastOnPosition(ToVec3(LinearPrediction()));
				}
				if (KillStealR->Enabled() && R->IsReady() && Enemy->IsValidTarget() && (Enemy->GetPosition() - BallLocation).Length2D() < RRange->GetInteger() && IsEnough(BallLocation, RRange->GetInteger()) >= KillStealMin->GetInteger() && GHealthPrediction->GetKSDamage(Enemy, kSlotR, R->GetDelay(), false) > EnemyHealth)
				{
					CastR();
				}
				if (Ignite != nullptr && KillStealIgnite->Enabled() && Ignite->IsReady() && Enemy->IsValidTarget(Player, Ignite->Range()))
				{
					if (IgniteSlot == 1)
					{
						if (GDamage->GetSummonerSpellDamage(Player, Enemy, kSummonerSpellIgnite) > Enemy->GetHealth() + Enemy->HPRegenRate() * 5)
						{
							Ignite->CastOnTarget(Enemy);
						}
					}
					else
					{
						if (GDamage->GetSummonerSpellDamage(Player, Enemy, kSummonerSpellIgnite) > Enemy->GetHealth() + Enemy->HPRegenRate() * 5)
						{
							Ignite->CastOnTarget(Enemy);
						}
					}
				}
			}
		}
	}
}

void Flee()
{
	GGame->IssueOrder(Player, kMoveTo, GGame->CursorPosition());
	if (!Player->HasBuff("orianaghostself") && E->IsReady() && FleeE->Enabled())
	{
		CastE(Player);
	}
	if (W->IsReady() && FleeW->Enabled())
	{
		CastW();
	}
}

bool buildingCheck(IUnit* enemy, IUnit* player)
{
	auto collision = false;
	Vec3 porte = Extend(BallLocation, player->GetPosition(), -400);
	Vec2 from = ToVec2(BallLocation);
	Vec2 to = ToVec2(porte);
	float m = ((to.y - from.y) / (to.x - from.x));
	float X;
	float Y;
	float m2 = (-(to.x - from.x) / (to.y - from.y));

	Vec3 minionP = enemy->GetPosition();
	Vec2 minionPos = ToVec2(minionP);
	float px = minionPos.x;
	float py = minionPos.y;
	X = ((m2*px) - (from.x*m) + (from.y - py)) / (m2 - m);
	Y = m * (X - from.x) + from.y;
	Vec2 colliPos;
	colliPos.Set(X, Y);
	if (Distance(colliPos, minionPos) <= enemy->BoundingRadius() + Q->Radius() - 100 && Distance(colliPos, from) <= Distance(from, ToVec2(player->GetPosition()))
		&& Distance(from, ToVec2(player->GetPosition())) > Distance(ToVec2(player->GetPosition()), colliPos))
	{
		collision = true;
	}
	return collision;
}

void autoEDmg()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());
	auto player = GEntityList->Player();

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
	{
		if (buildingCheck(enemy, player) == true && E->IsReady())
		{
			E->CastOnPlayer();
		}
	}
}

void ChangeHarassToggle()
{
	if (HarassToggle)
	{
		HarassToggle = false;
	}
	else
	{
		HarassToggle = true;
	}
	LastChanged = GGame->Time();
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());
	auto player = GEntityList->Player();
	GetBallLocation();
	KillSteal();
	LinearPrediction();
	empt.Set(0,0,0);


	auto missiles = GEntityList->GetAllMissiles(true, false);
	for (auto missile : missiles)
	{
		if (Equals(GMissileData->GetName(missile), "OrianaIzuna"))
		{
			BallLocation = missile->GetPosition();
		}
	}

	//if (GGame->TickCount() > delayer3)
	//{
	//	check = false;
	//}
	//
	//if (check && GGame->TickCount() > delayer6)
	//{
	//	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
	//	auto player = GEntityList->Player();
	//
	//	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && stopper == false)
	//	{
	//		epos123 = enemy->GetPosition();
	//		ppos123 = player->GetPosition();
	//		direction5 = (enemy->GetPosition() - ppos123).VectorNormalize();
	//		deneme = ppos123;
	//		stopper = true;
	//	}
	//
	//	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && GGame->TickCount() >= delayers)
	//	{
	//		double dist = Q->Speed() / 20;
	//
	//		deneme = deneme + (direction5*dist);
	//		delayers = GGame->TickCount() + 50;
	//	}
	//
	//	if (Distance(deneme, player->GetPosition()) > Q->Range())
	//	{
	//		stopper = false;
	//	}
	//}
	//if (AniviaQ == nullptr)
	//{
	//	deneme = GEntityList->Player()->GetPosition();
	//}

	if ((!GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && ComboE->Enabled())
		|| (!GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused() && GOrbwalking->GetOrbwalkingMode() == kModeMixed) && HarassEQ->Enabled())
	{
		autoEDmg();
	}

	if (RAuto->Enabled())
	{
		CheckR();
	}
	if (GetAsyncKeyState(FleeKey->GetInteger()) && !GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused())
	{
		Flee();
	}
	if (GetAsyncKeyState(HarassToggleKey->GetInteger()) && !GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused() && LastChanged + 1 < GGame->Time())
	{
		ChangeHarassToggle();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed || HarassToggle)
	{
		Harass();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		LaneClear();
	}
}

PLUGIN_EVENT(void) OnProcessSpell(CastedSpell const& Args)
{
	if (Args.Target_ == Player && Args.Caster_->IsEnemy(Player) && E->IsReady() && Player->HealthPercent() <= ShieldLife->GetInteger() && !ShieldDisabled->Enabled() && (ShieldAuto->Enabled() || (ShieldCombo->Enabled() && GOrbwalking->GetOrbwalkingMode() == kModeCombo) || (ShieldHarass->Enabled() && GOrbwalking->GetOrbwalkingMode() == kModeMixed) || (FleeE->Enabled() && GetAsyncKeyState(FleeKey->GetInteger() && !GGame->IsChatOpen() && GUtility->IsLeagueWindowFocused()))) && Args.Damage_ * 100 / Player->GetMaxHealth() >= ShieldDamage->GetInteger())
	{
		CastE(Player);
	}
}

PLUGIN_EVENT(bool) OnPreCast(int Slot, IUnit* Target, Vec3* StartPosition, Vec3* EndPosition)
{
	if (Slot == kSlotR && RCancel->Enabled())
	{
		if (IsEnoughR(BallLocation, RRange->GetInteger()) == 0)
		{
			return (false);
		}
	}
	return (true);
}

PLUGIN_EVENT(void) OnDash(UnitDash* Args)
{
	if (RInitiator->Enabled() && (RAuto->Enabled() || GOrbwalking->GetOrbwalkingMode() == kModeCombo && ComboR->Enabled()) && Args->Source->IsValidObject() && !Args->Source->IsEnemy(Player) && E->IsReady() && Args->Source->IsValidTarget(Player, E->Range()) && ((Args->EndPosition - Player->GetPosition()).Length2D()) <= 1250 && RMin->GetInteger() <= IsEnoughR(Args->EndPosition, RRange->GetInteger()))
	{
		CastE(Args->Source);
	}
}

PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{
	if (Source->IsMissile())
	{
		if (strcmp(GMissileData->GetName(Source), "OrianaIzuna") == 0)
		{
			Ball = Source;

		}
	}
}


void InstallEventHandlers()
{
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOnSpellCast, OnProcessSpell);
	GEventManager->AddEventHandler(kEventOnPreCast, OnPreCast);
	GEventManager->AddEventHandler(kEventOnDash, OnDash);
}

void UninstallEventHandlers()
{
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOnSpellCast, OnProcessSpell);
	GEventManager->RemoveEventHandler(kEventOnPreCast, OnPreCast);
	GEventManager->RemoveEventHandler(kEventOnDash, OnDash);
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	LoadSpells();
	InstallEventHandlers();
	Player = GEntityList->Player();
	LoadMenu();
	if (strcmp(Player->ChampionName(), "Orianna") == 0)
	{
		GGame->PrintChat("Orianna Ponche : Loaded");
	}
	else
	{
		GGame->PrintChat("Orianna Ponche : You are not playing Orianna...");
	}
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	UninstallEventHandlers();
	GGame->PrintChat("Orianna Ponche : Unloaded");
}