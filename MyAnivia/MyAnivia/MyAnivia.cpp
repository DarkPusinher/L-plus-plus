#include "PluginSDK.h"
#include "PluginData.h"
#include "Template.h"
#include "cmath"
#include "Geometry.h"
#include "Extention.h"
#include <map>
#include <vector>

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

std::vector<std::string> const& Names = { "High", "VeryHigh"};
float KeyPre;
int ComboMode = 1;

void inline LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, false, true, (kCollidesWithYasuoWall));
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

void autoQ()
{
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (AniviaQ != nullptr && enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		Qpos = AniviaQ->GetPosition();
		if (Distance(Qpos, enemy->GetPosition()) <= Q->Radius() + 20)
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

void Combo()
{
	auto player = GEntityList->Player();
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range() + Q->Radius() + 15);

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, R->Range()) && R->IsReady() && ComboR->Enabled() && AniviaR == nullptr && checkR == true)
		{
			R->CastOnUnit(enemy);
			timeR = GGame->TickCount();
			checkR = false;
		}

		if (player->IsValidTarget(enemy, W->Range() - 200) && W->IsReady() && ComboW->Enabled() && Q->IsReady() && E->IsReady())
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && ComboQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			Q->CastOnTarget(enemy, 6);
			time = GGame->TickCount();
			check = false;
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
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range() + Q->Radius() + 15);

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, R->Range()) && R->IsReady() && ComboR->Enabled() && AniviaR == nullptr && checkR == true)
		{
			R->CastOnUnit(enemy);
			timeR = GGame->TickCount();
			checkR = false;
		}

		if (player->IsValidTarget(enemy, W->Range() - 200) && W->IsReady() && ComboW->Enabled() && Q->IsReady() && E->IsReady())
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && ComboQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			Q->CastOnTarget(enemy, 5);
			time = GGame->TickCount();
			check = false;
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
				Q->CastOnTarget(Enemy, 5);
				timeKQ = GGame->TickCount();
				checkKQ = false;
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
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range() + Q->Radius() + 15);

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, W->Range() - 200) && W->IsReady() && HarrassW->Enabled() && Q->IsReady() && E->IsReady())
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && HarrassQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			Q->CastOnTarget(enemy, 6);
			time = GGame->TickCount();
			check = false;
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
	auto enemy = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range() + Q->Radius() + 15);

	if (enemy != nullptr && enemy->IsValidTarget() && enemy->IsHero())
	{
		if (player->IsValidTarget(enemy, W->Range() - 200) && W->IsReady() && HarrassW->Enabled() && Q->IsReady() && E->IsReady())
		{
			Vec3 enemyPos = enemy->GetPosition();
			Vec3 cast = Extend(enemyPos, player->GetPosition(), -190);
			W->CastOnPosition(cast);
		}

		if (player->IsValidTarget(enemy, Q->Range()) && Q->IsReady() && HarrassQ->Enabled() && check == true && AniviaQ == nullptr)
		{
			Q->CastOnTarget(enemy, 5);
			time = GGame->TickCount();
			check = false;
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
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen() && ComboMode == 1)
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo && !GGame->IsChatOpen() && ComboMode == 0)
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