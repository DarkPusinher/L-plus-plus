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


PluginSetup("MyAnivia")

IMenu* MainMenu;
IMenu* Anivia;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboEI;
IMenuOption* ComboR;

IMenu* FarmMenu;
IMenuOption* FarmQ;
IMenuOption* FarmE;
IMenuOption* FarmR;
IMenuOption* FarmQMin;
IMenuOption* FarmRMin;
IMenuOption* FarmELast;

IMenu* LastHitMenu;
IMenuOption* LastHitE;

IMenu* HarrassMenu;
IMenuOption* HarrassQ;
IMenuOption* HarrassW;
IMenuOption* HarrassE;
IMenuOption* HarrassEI;

IMenu* KSMenu;
IMenuOption* KSQ;
IMenuOption* KSR;
IMenuOption* KSE;

IMenu* DiscMenu;
IMenuOption* RCancel;
IMenuOption* QHitChance;
IMenuOption* ComboAA;
IMenuOption* ComboAAkey;
IMenuOption* ComboAALevel;

IMenu* RenderMenu;
IMenuOption* RenderQ;
IMenuOption* RenderW;
IMenuOption* RenderE;
IMenuOption* RenderR;
IMenuOption* HitChanceRender;
					
ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

IUnit* AniviaQ;
IUnit* QSlow;
IUnit* AniviaR;
Vec3 Qpos;

int time;
int r = 0;
bool check = false;
int timeR;
bool checkR = false;
int timeF;
bool checkF = false;
int timeFQ;
bool checkFQ = false;
int timeKQ;
bool checkKQ = false;
int timeKR;
bool checkKR = false;

std::vector<Vec3> epos;
std::vector<double> travelDistances;
double delayer = 0;
double delayer1 = 0;
double delayer2 = 0;
double delayer3 = 0;
double delayer5 = 0;
double delayer6 = 0;
double D = 0;
double averageDist = 0;

double theta;
bool stoper = false;
bool stopper = false;
bool stoper2 = false;
bool once = true;

double delayers = 0;
Vec3 epos123;
Vec3 ppos123;
Vec3 direction5;
Vec3 deneme;
Vec3 empt;

std::vector<std::string> const& Names = { "High", "VeryHigh"};
float KeyPre;
int ComboMode = 1;

void inline LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, (kCollidesWithNothing));
	Q->SetOverrideDelay(0.375f);
	Q->SetOverrideSpeed(900.f);
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, false, (kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, true, false, kCollidesWithYasuoWall);
	R = GPluginSDK->CreateSpell2(kSlotR, kCircleCast, false, true, (kCollidesWithNothing));
}

void inline Menu()
{
	MainMenu = GPluginSDK->AddMenu("MyAnivia");
	Anivia = MainMenu->AddMenu("Anivia");
	ComboMenu = Anivia->AddMenu("Combo");
	{
		ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
		ComboW = ComboMenu->CheckBox("Use W in Combo", true);
		ComboE = ComboMenu->CheckBox("Use E in Combo", true);
		ComboEI = ComboMenu->CheckBox("No E Unless Q-R", true);
		ComboR = ComboMenu->CheckBox("Use R in Combo", true);
	}
	HarrassMenu = Anivia->AddMenu("Harrass Menu");
	{
		HarrassQ = HarrassMenu->CheckBox("Use Q in Harrass", true);
		HarrassW = HarrassMenu->CheckBox("Use W in Harrass", true);
		HarrassE = HarrassMenu->CheckBox("Use E in Harrass", true);
		HarrassEI =HarrassMenu->CheckBox("No E Unless Q-R", true);
	}
	FarmMenu = Anivia->AddMenu("Farm");
	{
		FarmQ = FarmMenu->CheckBox("Use Q in Farm", true);
		FarmE = FarmMenu->CheckBox("Use E in Farm", true);
		FarmR = FarmMenu->CheckBox("Use R in Farm", true);
		FarmQMin = FarmMenu->AddFloat("Qfor # Minions", 1, 3, 2);
		FarmRMin = FarmMenu->AddFloat("R for # Minions",1 ,7 ,5);
		FarmELast = FarmMenu->CheckBox("E Only for Last hit", true);
	}
	LastHitMenu = Anivia->AddMenu("LastHit");
	{
		LastHitE = LastHitMenu->CheckBox("Last Hit with E", true);
	}
	KSMenu = Anivia->AddMenu("KS Menu");
	{
		KSQ = KSMenu->CheckBox("Use Q to KS", true);
		KSE = KSMenu->CheckBox("Use E to KS", true);
		KSR = KSMenu->CheckBox("Use R to KS", true);
	}
	DiscMenu = Anivia->AddMenu("Extra");
	{
		RCancel = DiscMenu->CheckBox("Cancel R if No minnions or enemies", true);
		QHitChance = DiscMenu->AddKey("Q Hit Chance Changer", 'T');
		ComboAALevel = DiscMenu->AddInteger("At what level disable AA", 1, 18, 6);
		ComboAA = DiscMenu->CheckBox("Disable AA", true);
		ComboAAkey = DiscMenu->AddKey("Disable key", 32);

	}
	RenderMenu = Anivia->AddMenu("Drawings");
	{
		RenderQ = RenderMenu->CheckBox("Draw Q Range", true);
		RenderW = RenderMenu->CheckBox("Draw W Range", true);
		RenderE = RenderMenu->CheckBox("Draw E Range", true);
		RenderR = RenderMenu->CheckBox("Draw R Range", true);
		HitChanceRender = RenderMenu->CheckBox("Draw HitChance on player", true);
	}
}

//Vec3 LinearPrediction(float accuracy, ISpell2* Q, IUnit* enemy)
//{
//	auto player = GEntityList->Player();
//
//	double D;
//	double S;
//	double t1;
//	double t2;
//	Vec3 direct;
//	empt.Set(0, 0, 0);
//	double travelDistance;
//
//	if (accuracy == 0)
//	{
//		if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && player->IsValidTarget(enemy, Q->Range()))
//		{
//			if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
//			{
//				if (enemy->IsMoving() == false)
//				{
//					return enemy->GetPosition();
//				}
//
//				D = Distance(enemy, player);
//				S = enemy->MovementSpeed();
//			}
//			if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && GGame->TickCount() > delayer)
//			{
//				epos.push_back(enemy->GetPosition());
//				//delayer = GGame->TickCount() + 75;
//			}
//			if (!epos.size() > 2)
//			{
//				return empt;
//			}
//			if (enemy != nullptr && epos.size() >= 2)
//			{
//				direct = (epos[epos.size() - 1] - epos[epos.size() - 2]).VectorNormalize();
//				Vec3 future = epos[epos.size() - 1] + (direct * 1000);
//				float p12f = Distance(player->GetPosition(), enemy->GetPosition());
//				float p13f = Distance(enemy->GetPosition(), future);
//				float p23f = Distance(player->GetPosition(), future);
//				double incos1 = ((p12f * p12f) + (p13f * p13f) - (p23f * p23f)) / (2 * p12f * p13f);
//				double result1 = acos(incos1) * 180.0 / PI;
//				theta = result1;
//				stoper1 = true;
//
//				double b = 2 * D*S*cos(theta);
//				double a = pow(Q->Speed(), 2) - pow(S, 2);
//				double c = -D*D;
//
//				t1 = (-b + sqrt((b*b) - 4 * a*c)) / (2 * a);
//				t2 = (-b - sqrt((b*b) - 4 * a*c)) / (2 * a);
//
//				if (t1 > 0 && t2 > 0)
//				{
//					if (t1 > t2)
//					{
//						travelDistance = (t2 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//						return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//					}
//					if (t2 > t1)
//					{
//						travelDistance = (t1 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//						return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//					}
//				}
//				if (t2 > 0)
//				{
//					travelDistance = (t2 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//					return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//				}
//				if (t1 > 0)
//				{
//					travelDistance = (t1 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//					return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//				}
//				if (t1 < 0 && t2 < 0)
//				{
//					return empt;
//				}
//			}
//
//				//if (travelDistances.size() > 1)
//				//{
//				//	averageDist = travelDistances[travelDistances.size() - 1] + travelDistances[travelDistances.size() - 2] / 2;
//				//	PrePos = epos[epos.size() - 1] + (direct*(averageDist - Q->Radius() + enemy->BoundingRadius() - 100));
//				//	averageDist = 0;
//				//	return PrePos;
//				//}
//			else
//				return empt;
//		}
//	}
//
//	if (accuracy == 1)
//	{
//		if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && player->IsValidTarget(enemy, Q->Range()))
//		{
//			if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget())
//			{
//				if (enemy->IsMoving() == false)
//				{
//					return enemy->GetPosition();
//				}
//
//				D = Distance(enemy, player);
//				S = enemy->MovementSpeed();
//			}
//			if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && GGame->TickCount() > delayer)
//			{
//				epos.push_back(enemy->GetPosition());
//				//delayer = GGame->TickCount() + 75;
//			}
//			if (!epos.size() > 2)
//			{
//				return empt;
//			}
//			if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && epos.size() > 2 && stoper == false)
//			{
//				if ((epos[epos.size() - 1] - epos[epos.size() - 2]).VectorNormalize() != (epos[epos.size() - 2] - epos[epos.size() - 3]).VectorNormalize())
//				{
//					Vec3 loc1 = enemy->GetPosition() + ((epos[epos.size() - 1] - epos[epos.size() - 2]).VectorNormalize()) * 1000;
//					Vec3 loc2 = enemy->GetPosition() + ((epos[epos.size() - 2] - epos[epos.size() - 3]).VectorNormalize()) * 1000;
//					if (Distance(loc1, loc2) > 10)
//					{
//						stoper = true;
//						delayer2 = GGame->TickCount() + 300;
//					}
//				}
//				else
//					return empt;
//			}
//			if (GGame->TickCount() <= delayer2)
//			{
//				if (stoper == true && enemy != nullptr)
//				{
//					direct = (epos[epos.size() - 1] - epos[epos.size() - 2]).VectorNormalize();
//					Vec3 future = epos[epos.size() - 1] + (direct * 1000);
//					float p12f = Distance(player->GetPosition(), enemy->GetPosition());
//					float p13f = Distance(enemy->GetPosition(), future);
//					float p23f = Distance(player->GetPosition(), future);
//					double incos1 = ((p12f * p12f) + (p13f * p13f) - (p23f * p23f)) / (2 * p12f * p13f);
//					double result1 = acos(incos1) * 180.0 / PI;
//					theta = result1;
//					stoper1 = true;
//				}
//
//				if (stoper == true && stoper1 == true/* && delayer3 <= GGame->TickCount()*/)
//				{
//					double b = 2 * D*S*cos(theta);
//					double a = pow(Q->Speed(), 2) - pow(S, 2);
//					double c = -D*D;
//
//					t1 = (-b + sqrt((b*b) - 4 * a*c)) / (2 * a);
//					t2 = (-b - sqrt((b*b) - 4 * a*c)) / (2 * a);
//					//delayer3 = GGame->TickCount() + 75;
//				}
//
//				if (stoper1 == true && stoper == true)
//				{
//					if (t1 > 0 && t2 > 0)
//					{
//						if (t1 > t2)
//						{
//							travelDistance = (t2 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//							return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//						}
//						if (t2 > t1)
//						{
//							travelDistance = (t1 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//							return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//						}
//					}
//					if (t2 > 0)
//					{
//						travelDistance = (t2 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//						return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//					}
//					if (t1 > 0)
//					{
//						travelDistance = (t1 + Q->GetDelay() + (GGame->Latency()) / 1000)*S;
//						return epos[epos.size() - 1] + (direct*(travelDistance - Q->Radius() + enemy->BoundingRadius() - 100));
//					}
//					if (t1 < 0 && t2 < 0)
//					{
//						return empt;
//					}
//					//if (travelDistances.size() > 1)
//					//{
//					//	averageDist = travelDistances[travelDistances.size() - 1] + travelDistances[travelDistances.size() - 2] / 2;
//					//	PrePos = epos[epos.size() - 1] + (direct*(averageDist - Q->Radius() + enemy->BoundingRadius() - 100));
//					//	averageDist = 0;
//					//	return PrePos;
//					//}
//				}
//				else
//					return empt;
//			}
//			if (delayer2 < GGame->TickCount())
//			{
//				stoper1 = false;
//				stoper = false;
//				return empt;
//			}
//		}
//	}
//}

bool CollisionChecker(IUnit* enemy, IUnit* player, ISpell2* Q)
{
	auto collision = false;
	Vec3 porte = enemy->GetPosition();
	Vec2 from = ToVec2(player->GetPosition());
	Vec2 to = ToVec2(porte);
	float m = ((to.y - from.y) / (to.x - from.x));
	float X;
	float Y;
	float m2 = (-(to.x - from.x) / (to.y - from.y));
	auto minions = GEntityList->GetAllMinions(false, true, true);

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
		if (Distance(colliPos, minionPos) <= Q->Radius() + minion->BoundingRadius() - 10 && Distance(colliPos, ToVec2(porte)) < Distance(from, ToVec2(porte)) && Distance(from, colliPos) < Distance(ToVec2(enemy->GetPosition()), from))
		{
			collision = true;
			break;
		}
	}

	return collision;
}

Vec3 LinearPrediction()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	double h = player->GetPosition().x;
	double k = player->GetPosition().y;
	double SS = Q->Speed();
	double temporar = 1000;
	int t;
	std::vector<double> Ds;

	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && player->IsValidTarget(enemy, Q->Range()))
	{
		if (!enemy->IsMoving())
		{
			return enemy->GetPosition();
		}
		
		
		if (enemy->IsDashing())
		{
			return empt;
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
					return empt;
			}
			else
				return empt;
		}
		
		if (stoper = true && GGame->TickCount() >= delayer2)
		{
			Vec3 direction = (epos[epos.size() - 1] - epos[epos.size() - 2]).VectorNormalize();
			for (int i = 0; i < 2000; i++)
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
			return fut;
		}
		else
		{
			stoper = false;
			return empt;
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
		return empt;
}

void autoQ()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (AniviaQ != nullptr && enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		Qpos = AniviaQ->GetPosition();
		if (Distance(Qpos, enemy->GetPosition()) <= Q->Radius() + enemy->BoundingRadius() - 75)
		{
			Q->CastOnPlayer();
		}
	}
}

inline int CountEnemy(Vec3 Location, int range)
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

inline int CountMinions(Vec3 Location, int range)
{
	int Count = 0;

	for (auto Minion : GEntityList->GetAllMinions(false, true, true))
	{
		if ((Minion->GetPosition() - Location).Length2D() < range && Minion->IsValidTarget() && !Minion->IsDead())
		{
			Count++;
		}
	}
	return (Count);
}

void autoQFarm()
{
	for (auto Minion : GEntityList->GetAllMinions(false, true, true))
	{
		if (Minion->IsEnemy(GEntityList->Player()) && !Minion->IsDead() && Minion->IsValidTarget() && Minion->IsCreep() || Minion->IsJungleCreep())
		{
			if (AniviaQ != nullptr)
			{
				Qpos = AniviaQ->GetPosition();
				if (CountMinions(Qpos, Q->Radius() + 20) >= FarmQMin->GetFloat())
				{
					Q->CastOnPlayer();
				}
			}
		}
	}
}

void RStop()
{
	if (AniviaR != nullptr && RCancel->Enabled())
	{
		if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
		{
			if (CountEnemy(AniviaR->GetPosition(), 500) == 0)
			{
				R->CastOnPlayer();
			}
		}
		if (CountEnemy(AniviaR->GetPosition(), 500) == 0 && CountMinions(AniviaR->GetPosition(), 500) == 0)
		{
			R->CastOnPlayer();
		}
	}
}

void autoQFull()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && ComboQ->Enabled() && check == true && AniviaR != nullptr)
	{
		empt.Set(0,0,0);
		if (LinearPrediction() != empt)
		{
			Q->CastOnPosition(LinearPrediction());
			time = GGame->TickCount();
			check = false;
		}
	}
}

void Combo()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, R->Range()) && R->IsReady() && ComboR->Enabled() && AniviaR == nullptr && checkR == true && (Q->IsReady() || E->IsReady()))
		{
			R->CastOnUnit(enemy);
			timeR = GGame->TickCount();
			checkR = false;
		}

		if (player->IsValidTarget(enemy, W->Range() - 100) && Distance(enemy->GetPosition(), player->GetPosition()) > R->Range() && R->IsReady() && ComboW->Enabled() && ComboR->Enabled() && AniviaR == nullptr && checkR == true && (Q->IsReady() || E->IsReady()))
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(player->GetPosition(), enemyPos, R->Range());
			R->CastOnPosition(cast);
			timeR = GGame->TickCount();
			checkR = false;
		}

		if (player->IsValidTarget(enemy, W->Range() - 100) && W->IsReady() && ComboW->Enabled() && (Q->IsReady() && E->IsReady()) || (R->IsReady() && E->IsReady() && ComboW->Enabled()))
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && ComboQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			if (LinearPrediction() != empt)
			{
				delayer3 = GGame->TickCount() + (Q->GetDelay()* 1000);
				Q->CastOnPosition(LinearPrediction());
				time = GGame->TickCount();
				check = false;
			}
		}

		if (player->IsValidTarget(enemy, player->AttackRange()) && E->IsReady() && ComboE->Enabled())
		{
			if (ComboEI->Enabled())
			{
				if (enemy->HasBuff("Stun") || enemy->HasBuff("aniviaiced"))
				{
					E->CastOnUnit(enemy);
				}
			}
			else
			{
				E->CastOnUnit(enemy);
			}
		}
	}
}

void Combo2()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, R->Range()) && R->IsReady() && ComboR->Enabled() && AniviaR == nullptr && checkR == true && (Q->IsReady() || E->IsReady()))
		{
			R->CastOnUnit(enemy);
			timeR = GGame->TickCount();
			checkR = false;
		}

		if (player->IsValidTarget(enemy, W->Range() - 100) && Distance(enemy->GetPosition(), player->GetPosition()) > R->Range() && R->IsReady() && ComboR->Enabled() && AniviaR == nullptr && checkR == true && (Q->IsReady() || E->IsReady()))
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(player->GetPosition(), enemyPos, R->Range());
			R->CastOnPosition(cast);
			timeR = GGame->TickCount();
			checkR = false;
		}

		if (player->IsValidTarget(enemy, W->Range() - 100) && W->IsReady() && ComboW->Enabled() && Q->IsReady() && E->IsReady() || (R->IsReady() && E->IsReady() && ComboW->Enabled()))
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && ComboQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			if (LinearPrediction() != empt)
			{
				delayer3 = GGame->TickCount() + (Q->GetDelay() * 1000);
				Q->CastOnPosition(LinearPrediction());
				time = GGame->TickCount();
				check = false;
			}
		}

		if (player->IsValidTarget(enemy, player->AttackRange()) && E->IsReady() && ComboE->Enabled())
		{
			if (ComboEI->Enabled())
			{
				if (enemy->HasBuff("Stun") || enemy->HasBuff("aniviaiced"))
				{
					E->CastOnUnit(enemy);
				}
			}
			else
			{
				E->CastOnUnit(enemy);
			}
		}
	}
}

void Farm()
{
	for (auto Minion : GEntityList->GetAllMinions(false, true, true))
	{
		if (Minion->IsEnemy(GEntityList->Player()) && !Minion->IsDead() && Minion->IsValidTarget() && Minion->IsCreep() || Minion->IsJungleCreep())
		{
			if (FarmR->Enabled() && R->IsReady() && Minion->IsValidTarget(GEntityList->Player(), R->Range()))
			{
				Vec3 cas;
				int Nhit;
				GPrediction->FindBestCastPosition(R->Range(), 425, false, true, false, cas, Nhit, R->GetDelay());
				if (Nhit >= FarmRMin->GetFloat() && AniviaR == nullptr && checkF == true)
				{
					R->CastOnPosition(cas);
					timeF = GGame->TickCount();
					checkF = false;
				}
			}
			if (FarmQ->Enabled() && Q->IsReady() && Minion->IsValidTarget(GEntityList->Player(), Q->Range()))
			{
				Vec3 casPos;
				int Nhits;
				GPrediction->FindBestCastPosition(Q->Range(), Q->Radius() + 20, false, true, false, casPos, Nhits, Q->GetDelay());
				if (Nhits >= FarmQMin->GetFloat() && checkFQ == true)
				{
					Q->CastOnPosition(casPos);
					timeFQ = GGame->TickCount();
					checkFQ = false;
				}
			}
			if (FarmE->Enabled() && !FarmELast->Enabled() && E->IsReady() && Minion->IsValidTarget(GEntityList->Player(), E->Range()))
			{
				E->CastOnUnit(Minion);
			}
			if (FarmELast->Enabled() && E->IsReady() && Minion->IsValidTarget(GEntityList->Player(), E->Range()))
			{
				if(Minion->GetHealth() + 10 <= (GDamage->GetSpellDamage(GEntityList->Player(), Minion, kSlotE)))
					{
						E->CastOnUnit(Minion); 
					}
			}
		}
	}
}

void lastHit()
{
	for (auto Minion : GEntityList->GetAllMinions(false, true, true))
	{
		if (Minion->IsEnemy(GEntityList->Player()) && !Minion->IsDead() && Minion->IsValidTarget() && Minion->IsCreep() || Minion->IsJungleCreep())
		{
			if (LastHitE->Enabled() && E->IsReady() && Minion->IsValidTarget(GEntityList->Player(), E->Range()))
			{
				if (Minion->GetHealth() + 10 <= (GDamage->GetSpellDamage(GEntityList->Player(), Minion, kSlotE)))
				{
					E->CastOnUnit(Minion);
				}
			}
		}
	}
}

void KS()
{
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		auto QDamage = GDamage->GetSpellDamage(GEntityList->Player(), Enemy, kSlotQ);
		auto EDamage = GDamage->GetSpellDamage(GEntityList->Player(), Enemy, kSlotE);
		auto RDamage = GDamage->GetSpellDamage(GEntityList->Player(), Enemy, kSlotR);

		if (Enemy != nullptr && !Enemy->IsDead() & Enemy->IsValidTarget())
		{
			if (KSE->Enabled() && E->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), E->Range()) && EDamage > Enemy->GetHealth())
			{
				E->CastOnUnit(Enemy);
			}
			if (KSQ->Enabled() && Q->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), Q->Range()) && QDamage > Enemy->GetHealth() && AniviaQ == nullptr && checkKQ == true)
			{
				if (LinearPrediction() != empt)
				{
					Q->CastOnPosition(LinearPrediction());
					timeKQ = GGame->TickCount();
					checkKQ = false;
				}
			}
			if (KSR->Enabled() && R->IsReady() && Enemy->IsValidTarget(GEntityList->Player(), R->Range()) && RDamage > Enemy->GetHealth() && AniviaR == nullptr && checkKR == true)
			{
				R->CastOnUnit(Enemy);
				timeKR = GGame->TickCount();
				checkKR = false;
			}
		}
	}
}

void Harrass()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, W->Range() - 100) && W->IsReady() && HarrassW->Enabled() && Q->IsReady() && E->IsReady())
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && HarrassQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			AdvPredictionOutput predic;
			AdvPredictionInput inpute = { player->GetPosition(), player->GetPosition(), true, true, kCollidesWithYasuoWall, Q->GetDelay(), Q->Radius(), Q->Range(), Q->Speed(), kLineCast , enemy };
			GPrediction->RunPrediction(&inpute, &predic);
			if (predic.HitChance >= kHitChanceHigh)
			{
				Vec3 MyPosition = Extend(predic.CastPosition, enemy->GetPosition(), enemy->BoundingRadius() + Q->Radius() -10);
				Q->CastOnPosition(MyPosition);
				time = GGame->TickCount();
				check = false;
			}
		}

		if (player->IsValidTarget(enemy, player->AttackRange()) && E->IsReady() && HarrassE->Enabled())
		{
			if (HarrassEI->Enabled())
			{
				if (enemy->HasBuff("Stun") || enemy->HasBuff("aniviaiced"))
				{
					E->CastOnUnit(enemy);
				}
			}
			else
			{
				E->CastOnUnit(enemy);
			}
		}
	}
}

void Harrass2()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, W->Range() - 100) && W->IsReady() && HarrassW->Enabled() && Q->IsReady() && E->IsReady())
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && HarrassQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			AdvPredictionOutput predic;
			AdvPredictionInput inpute = { player->GetPosition(), player->GetPosition(), true, true, kCollidesWithYasuoWall, Q->GetDelay(), Q->Radius(), Q->Range(), Q->Speed(), kLineCast , enemy };
			GPrediction->RunPrediction(&inpute, &predic);
			if (predic.HitChance >= kHitChanceHigh)
			{
				Vec3 MyPosition = Extend(predic.CastPosition, enemy->GetPosition(), enemy->BoundingRadius() + Q->Radius() - 10);
				Q->CastOnPosition(MyPosition);
				time = GGame->TickCount();
				check = false;
			}
		}

		if (player->IsValidTarget(enemy, player->AttackRange()) && E->IsReady() && HarrassE->Enabled())
		{
			if (HarrassEI->Enabled())
			{
				if (enemy->HasBuff("Stun") || enemy->HasBuff("aniviaiced"))
				{
					E->CastOnUnit(enemy);
				}
			}
			else
			{
				E->CastOnUnit(enemy);
			}
		}
	}
}

inline int ChangePriority()
{
	if (GetAsyncKeyState(QHitChance->GetInteger()) && !GGame->IsChatOpen() && GGame->Time() > KeyPre)
	{
		if (ComboMode == 0)
		{
			ComboMode = 1;
			KeyPre = GGame->Time() + 0.250;
		}
		else
		{
			ComboMode = 0;
			KeyPre = GGame->Time() + 0.250;
		}
	}
	return ComboMode;
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	//autoQFull();
	//auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
	//if (enemy != nullptr)
	//{
	//	LinearPrediction(1, Q,enemy);
	//}
	LinearPrediction();

	//if (GGame->TickCount() > delayer3 && check == false)
	//{
	//	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
	//	auto player = GEntityList->Player();
	//
	//	if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && stopper == false)
	//	{
	//		epos123 = enemy->GetPosition();
	//		ppos123 = player->GetPosition();
	//		direction5 = (epos123 - ppos123).VectorNormalize();
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

	if (GetAsyncKeyState(ComboAAkey->GetInteger()))
	{
		auto level = GEntityList->Player()->GetLevel();
		if (ComboAA->Enabled() && level >= ComboAALevel->GetInteger() && GEntityList->Player()->GetMana() > 100)
		{
			GOrbwalking->SetAttacksAllowed(false);
		}
	}
	if (!GetAsyncKeyState(ComboAAkey->GetInteger()) || GEntityList->Player()->GetMana() < 100)
	{
		{
			GOrbwalking->SetAttacksAllowed(true);
		}
	}
	if (GGame->TickCount() - time >= 1000)
	{
		checkKQ = true;
	}
	if (GGame->TickCount() - timeR >= 1000)
	{
		checkKR = true;
	}
	if (GGame->TickCount() - time >= 1000)
	{
		check = true;
	}
	if (GGame->TickCount() - timeR >= 1000)
	{
		checkR = true;
	}
	if (GGame->TickCount() - timeF >= 1000)
	{
		checkF = true;
	}
	if (GGame->TickCount() - timeFQ >= 1000)
	{
		checkFQ = true;
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen() && ChangePriority() == 0)
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen() && ChangePriority() == 1)
	{
		Combo2();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear && !GGame->IsChatOpen())
	{
		Farm();
		autoQFarm();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLastHit && !GGame->IsChatOpen())
	{
		lastHit();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && !GGame->IsChatOpen() && ComboMode == 1)
	{
		Harrass();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && !GGame->IsChatOpen() && ComboMode == 0)
	{
		Harrass2();
	}
	if (AniviaR == nullptr)
	{
		r = 0;
	}

	RStop();
	autoQ();
	KS();
}

PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "Anivia_Base_BA_Tar.troy"))
	{
		QSlow = Source;
	}
	if (Equals(Source->GetObjectName(), "Anivia_Base_Q_AOE_Mis.troy"))
	{
		AniviaQ = Source;
	}
	if (Equals(Source->GetObjectName(), "Anivia_Base_R_AOE_Green.troy"))
	{
		AniviaR = Source;
	}
	if (Equals(Source->GetObjectName(), "Anivia_Base_Q_Tar.troy"))
	{
		AniviaQ = nullptr;
		QSlow = nullptr;
	}
}

PLUGIN_EVENT(void) OnDestroyObject(IUnit* Source)
{
	if (Distance(Source->GetPosition(), GEntityList->Player()->GetPosition()) <= 1000)
	{
		
	}
	if (Equals(Source->GetObjectName(), "Anivia_Base_Q_AOE_Mis.troy"))
	{
		AniviaQ = nullptr;
	}
	if (Equals(Source->GetObjectName(), "Anivia_Base_BA_Tar.troy"))
	{
		QSlow = nullptr;
	}
	if (Equals(Source->GetObjectName(), "Anivia_Base_R_AOE_Green.troy"))
	{
		AniviaR = nullptr;
	}
}

PLUGIN_EVENT(void) OnRender()
{
	//if (deneme != empt) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(deneme, Vec4(255, 255, 0, 255), 50); }
	//auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
	//if(enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && LinearPrediction() != empt){ GPluginSDK->GetRenderer()->DrawOutlinedCircle(LinearPrediction(), Pink(), 50); }
	if(AniviaQ != nullptr) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(AniviaQ->GetPosition(), Pink(), Q->Radius()); }
	if (RenderQ->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }
	if (RenderW->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range()); }
	if (RenderE->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }
	if (RenderR->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }
	if (HitChanceRender->Enabled())
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
			std::string text = std::string(Names[ChangePriority()]);
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