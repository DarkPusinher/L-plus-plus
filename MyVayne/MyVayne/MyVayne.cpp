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
IMenuOption* EOnly;
IMenuOption* AutoE;
IMenuOption* AutoR;
IMenuOption* AutoRX;

IMenu* ExtraMenu;
IMenuOption* EGap;
IMenuOption* EInt;
IMenuOption* LockW;
IMenuOption* QModeChange;

IMenu* RenderMenu;
IMenuOption* DrawReady;
IMenuOption* DrawQ;
IMenuOption* DrawE;
IMenuOption* DrawPriority;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

IUnit* WStacked;
IUnit* closestTurret;

std::vector<std::string> const& Names = { "Safe", "Risky" };
int ComboMode = 1;
float KeyPre = 0.f;

void  DrawMenu()
{
	MainMenu = GPluginSDK->AddMenu("A-Series Vayne");

	ComboMenu = MainMenu->AddMenu("Combo");
	ComboQ = ComboMenu->CheckBox("Use Q", true);
	QSmart = ComboMenu->CheckBox("Smart Q", true);
	Keeper = ComboMenu->CheckBox("Keep invisibility", true);
	InvisH = ComboMenu->AddFloat("Keep invisibilty if HP < %", 0, 100, 35);
	AutoE = ComboMenu->CheckBox("Auto Condemn", true);
	EOnly = ComboMenu->CheckBox("E Only if combo or harrass mode", true);
	AutoR = ComboMenu->CheckBox("Auto R if more than x", true);
	AutoRX = ComboMenu->AddInteger("Enemies in range", 1, 5, 2);
	
	ExtraMenu = MainMenu->AddMenu("Extra");
	EGap = ExtraMenu->CheckBox("Auto Anti-GapCloser", true);
	EInt = ExtraMenu->CheckBox("Auto interrupter", true);
	LockW = ExtraMenu->CheckBox("Lock on W target", true);
	QModeChange = ExtraMenu->AddKey("Change Q Mode Key", 'T');

	RenderMenu = MainMenu->AddMenu("Drawing Settings");
	DrawReady = RenderMenu->CheckBox("Draw Only Ready Spells", true);
	DrawQ = RenderMenu->CheckBox("Draw Q", true);
	DrawE = RenderMenu->CheckBox("Draw E", true);
	DrawPriority = RenderMenu->CheckBox("Draw QMode", true);
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

inline int ChangePriority()
{
	if (GetAsyncKeyState(QModeChange->GetInteger()) && !GGame->IsChatOpen() && GGame->Time() > KeyPre)
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

void  ELogic() // Unique
{
	auto enemies = GEntityList->GetAllHeros(false, true);
	auto player = GEntityList->Player();

	for (auto target : enemies)
	{
		if (target != nullptr && target->IsHero())
		{
			if (player->IsValidTarget(target, E->Range()) && E->IsReady() && (LineEquation(target, player, 425) == true || buildingCheck(target, player, 425)))
			{
				E->CastOnUnit(target);
			}
		}
	}
}

Vec3 SmartQLogic()                 
{
	IUnit* temporar = GEntityList->GetEnemyNexus();
	if (ComboMode == 0)
	{
		auto player = GEntityList->Player();
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, Q->Range() + player->AttackRange());
		auto enemySafe = GTargetSelector->FindTarget(ClosestPriority, PhysicalDamage, Q->Range() + player->AttackRange());
		auto nexus = GEntityList->GetTeamNexus();
		auto turrets = GEntityList->GetAllTurrets(true, false);
		for (int m = 0; m < turrets.size(); m ++)
		{
			if (Distance(turrets[m], GEntityList->Player()) < Distance(temporar, GEntityList->Player()) && !turrets[m]->IsDead())
			{
				temporar = turrets[m];
				closestTurret = turrets[m];
			}
		}

		if (enemySafe != nullptr && enemySafe->IsValidTarget() && enemySafe->IsHero() && enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero() && ComboQ->Enabled() && QSmart->Enabled() && Q->IsReady())
		{
			if (player->IsValidTarget(enemySafe, Q->Range() + player->AttackRange()) && player->IsValidTarget(enemy, Q->Range() + player->AttackRange()) && E->IsReady())
			{
				for (int i = 5; i < 360; i += 5)
				{
					Vec3 postQ1;
					Vec3 posQ1;
					postQ1 = RotateAround(enemy->GetPosition(), player->GetPosition(), i);
					posQ1 = Extend(player->GetPosition(), postQ1, Q->Range());
					if (LineEquations(enemy, posQ1, player, 425) == true || buildingChecks(enemy, posQ1, player, 425) == true)
					{
						return posQ1;
						break;
					}
				}
			}
			if (EnemiesInRange(1500, player->GetPosition()) > 2)
			{
				if (EnemiesInRange(player->AttackRange() + 100, player->GetPosition()) >= 2)
				{
					std::vector<Vec3> temp;
					for (int i = 5; i < 360; i += 5)
					{
						Vec3 posteQ1;
						Vec3 poseQ1;
						posteQ1 = RotateAround(enemySafe->GetPosition(), player->GetPosition(), i);
						poseQ1 = Extend(player->GetPosition(), posteQ1, Q->Range());
						if (Distance(poseQ1, enemySafe->GetPosition()) < player->AttackRange() - 100)
						{
							temp.push_back(poseQ1);
						}
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
					return GGame->CursorPosition();
			}
			//if (EnemiesInRange(1500, player->GetPosition()) > 2)
			//{
			//	if (EnemiesInRange(player->AttackRange() + 100, player->GetPosition()) >= 2 && Distance(GGame->CursorPosition(), enemy->GetPosition()) < Distance(player->GetPosition(),enemy->GetPosition()))
			//	{
			//		Vec3 post1;
			//		Vec3 post2;
			//		Vec3 pos1;
			//		Vec3 pos2;
			//		post1 = RotateAround(enemy->GetPosition(), player->GetPosition(), 90);
			//		post2 = RotateAround(enemy->GetPosition(), player->GetPosition(), -90);
			//		pos1 = Extend(player->GetPosition(), post1, Q->Range());
			//		pos2 = Extend(player->GetPosition(), post2, Q->Range());
			//		if (Distance(pos1, nexus->GetPosition()) <= Distance(pos2, nexus->GetPosition()))
			//		{
			//			return pos1;
			//		}
			//		else
			//			return pos2;
			//	}
			//	else
			//		return GGame->CursorPosition();
			//}
			else
				return GGame->CursorPosition();
		}
		else if (ComboQ->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && !QSmart->Enabled())
		{
			return GGame->CursorPosition();
		}
	}
	if (ComboMode == 1)
	{
		auto player = GEntityList->Player();
		auto enemy = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, Q->Range() + player->AttackRange());
		auto nexus = GEntityList->GetTeamNexus();

		if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero() && ComboQ->Enabled() && QSmart->Enabled() && Q->IsReady())
		{
			if (player->IsValidTarget(enemy, Q->Range() + player->AttackRange()) && E->IsReady())
			{
				for (int i = 5; i < 360; i += 5)
				{
					Vec3 postQ1;
					Vec3 posQ1;
					postQ1 = RotateAround(enemy->GetPosition(), player->GetPosition(), i);
					posQ1 = Extend(player->GetPosition(), postQ1, Q->Range());
					if (LineEquations(enemy, posQ1, player, 425) == true || buildingChecks(enemy, posQ1, player, 425) == true)
					{
						return posQ1;
						break;
					}
				}
			}
			if(ComboQ->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && QSmart->Enabled())
				return GGame->CursorPosition();
		}
		else if (ComboQ->Enabled() && Q->IsReady() && GOrbwalking->GetOrbwalkingMode() == kModeCombo && !QSmart->Enabled())
		{
			return GGame->CursorPosition();
		}
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
	if (AutoE->Enabled() && !EOnly->Enabled())
	{
		ELogic();
	}
	ChangePriority();

	if (LockW->Enabled())
	{
		auto player = GEntityList->Player();
		auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, player->AttackRange());
		auto enemies = GEntityList->GetAllHeros(false, true);
		for (auto enemy : enemies)
		{
			if (enemy != nullptr && enemy->IsHero() && enemy->IsValidTarget() && WStacked != nullptr)
			{
				if (player->IsValidTarget(enemy, player->AttackRange()) && enemy->HasBuff("VayneSilveredDebuff")
					&& Distance(WStacked->GetPosition(), enemy->GetPosition()) < 50)
				{
					GOrbwalking->SetOverrideTarget(enemy);
					break;
				}
			}
			else
				GOrbwalking->SetOverrideTarget(target);
		}
	}

	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		RLogic();
		if (EOnly->Enabled())
		{
			ELogic();
		}
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		if (EOnly->Enabled())
		{
			ELogic();
		}
	}
}

PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "vayne_base_W_ring2.troy"))
	{
		WStacked = Source;
	}
}

PLUGIN_EVENT(void) OnDestroyObject(IUnit* Source)
{
	if (Equals(Source->GetObjectName(), "vayne_base_W_ring2.troy"))
	{
		WStacked = nullptr;
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
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->AddEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->AddEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOrbwalkAfterAttack, OnAfterAttack);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnGapCloser);
	GEventManager->RemoveEventHandler(kEventOnInterruptible, OnInterruptible);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}