#include "cheat.h"
#include "../injector/injector.hpp"
#include "../IniReader.h"
#include "../KeyBind.h"
#include <cSlowRateManager.h>
#include <Trigger.h>
#include <GameMenuStatus.h>
#include <Pl0000.h>
#include <cGameUIManager.h>
#include <PlayerManagerImplement.h>

/* TODO #
*  Make functions for each cheat instead (since return will cancel the handle cheats instead of if condition  
*/

DWORD OneHitKillCaveExit = shared::base + 0x68EE3A;
DWORD InfiniteRocketsCaveExit = shared::base + 0x5499F9;
DWORD InfiniteGrenadeCaveExit = shared::base + 0x54D8D6;
DWORD GroundCheatCaveExit = shared::base + 0xE6B464;
DWORD VRTimerCaveExit = shared::base + 0x81B44A;

typedef void(__stdcall * __funcA)(void *entity, const char *name, unsigned int index, int *pInfo);
typedef void(__stdcall * __funcB)(int a, float b);

typedef void(__stdcall * __funcC)(uintptr_t* structer, const char* name, unsigned int index, void* pInfo, ...);
__funcC createEnemy = reinterpret_cast<__funcC>(shared::base + 0x682090);
__funcA FuncA;
__funcB FuncB;

void __declspec(naked) OneHitKillCave() noexcept
{
	__asm {
		mov eax, [ecx+0x870]
		sub [ecx+0x870], eax
		jmp OneHitKillCaveExit
	}
}

void __declspec(naked) InfiniteRocketsCave() noexcept
{
	__asm {
		mov [ecx + 0x68], eax
		mov edx, [ecx + 0x60]
		dec edx
		mov [ecx + 0x5C], edx
		jmp InfiniteRocketsCaveExit
	}
}

void __declspec(naked) InfiniteGrenadeCave() noexcept
{
	__asm {
		mov eax, [ecx + 0x58]
		mov [ecx + 0x54], eax
		mov eax, [ecx + 0x54]
		jmp InfiniteGrenadeCaveExit
	}
}

void __declspec(naked) GroundCheatCave() noexcept
{
	__asm {
			cmp cheat::groundEnabled, 1
			je enabled
			jmp otherwise

			enabled:
				mov [eax], 2
				jmp GroundCheatCaveExit

			otherwise:
				mov [eax], 0
				jmp GroundCheatCaveExit
		
	}
}

float vrTimer = 0.10f;

void __declspec(naked) InfiniteVRTimerCave() noexcept
{
	__asm {
		fld dword ptr [vrTimer]
		jmp VRTimerCaveExit
	}
}

// Infinite fuel container
void cheat::MugenZangekiCheat() noexcept
{
	Trigger::GameFlags.GAME_MUGEN_ZANGEKI = infiniteFc;
}

// You can't die in game
void cheat::InfiniteHealthCheat() noexcept
{
	static bool once = false;
	if (infiniteHealth && !once) // its just patches the game, but doesn't write the value into
	{
		injector::MakeNOP(shared::base + 0x787859, 6, true);
		injector::MakeNOP(shared::base + 0x787865, 6, true);
		once = true;
	}
	else if (!infiniteHealth && once)
	{
		unsigned char rawBytes1[6] = { 0x29, 0x81, 0x70, 0x08, 0x00, 0x00 };
		injector::WriteMemoryRaw(shared::base + 0x787859, rawBytes1, 6, true);

		unsigned char rawBytes2[6] = { 0x29, 0x91, 0x70, 0x08, 0x00, 0x00 };
		injector::WriteMemoryRaw(shared::base + 0x787865, rawBytes2, 6, true);
		once = false;
	}
}

// One hit kills enemies (bosses included)
void cheat::OneHitKillCheat() noexcept
{
	static bool once = false;
	if (oneHitKill && !once)
	{
		injector::MakeJMP(shared::base + 0x68EE34, &OneHitKillCave, true); // 29 81 70 08 00 00
		injector::MakeNOP(shared::base + 0x68EE39, 1, true);
		once = true;
	}
	else if (!oneHitKill && once)
	{
		unsigned char rawBytes[6] = { 0x29, 0x81, 0x70, 0x08, 0x00, 0x00 };
		injector::WriteMemoryRaw(shared::base + 0x68EE34, rawBytes, 6, true);
		once = false;
	}
}

// Infinite sub-weapon ammo
void cheat::InfiniteSubWeaponCheat() noexcept
{
	static bool once = false;
	if (infiniteSubWeapon && !once)
	{
		injector::MakeJMP(shared::base + 0x5499F3, &InfiniteRocketsCave, true);
		injector::MakeNOP(shared::base + 0x5499F8, 1, true);

		injector::MakeJMP(shared::base + 0x54D8D0, &InfiniteGrenadeCave, true);
		injector::MakeNOP(shared::base + 0x54D8D5, 1, true);
		once = true;
	}
	else if (!infiniteSubWeapon && once)
	{
		unsigned char rocketArray[6] = { 0x89, 0x41, 0x68, 0x89, 0x51, 0x5C };
		unsigned char grenadeArray[6] = { 0xFF, 0x49, 0x54, 0x8B, 0x41, 0x54 };

		injector::WriteMemoryRaw(shared::base + 0x5499F3, rocketArray, 6, true);
		injector::WriteMemoryRaw(shared::base + 0x54D8D0, grenadeArray, 6, true);
		once = false;
	}
}

// No damage status for battles
void cheat::NoDamageStatCheat() noexcept
{
	static bool once = false;
	if (noDamageStat && !once)
	{
		unsigned char patched[1] = { 0x00 };
		injector::WriteMemoryRaw(shared::base + 0x81B482, patched, 1, true);
		once = true;
	}
	else if (!noDamageStat && once)
	{
		unsigned char raw[1] = { 0x01 };
		injector::WriteMemoryRaw(shared::base + 0x81B482, raw, 1, true);
		once = false;
	}
}

// Stealth cheat, i think you get it
void cheat::StealthCheat() noexcept
{
	static bool once = false;
	if (stealth && !once)
	{
		unsigned char patched[2] = { 0xEB, 0x19 };
		injector::WriteMemoryRaw(shared::base + 0x849286, patched, 2, true);
		once = true;
	}
	else if (!stealth && once)
	{
		unsigned char original[2] = { 0x74, 0x19 };
		injector::WriteMemoryRaw(shared::base + 0x849286, original, 2, true);
		once = false;
	}
}

// Changes height
void cheat::HeightChangeCheat() noexcept
{
	if (g_GameMenuStatus != InGame || !OnFocus)
		return;

	if (heightChange)
	{
		Pl0000* player = (Pl0000*)g_cGameUIManager.m_pPlayer;

		if (!player)
			return;

		if (shared::IsKeyPressed(VK_ADD, VK_ADD, false))
			player->m_vecOffset.y += heightRate;
		else if (shared::IsKeyPressed(VK_SUBTRACT, VK_SUBTRACT, false))
			player->m_vecOffset.y -= heightRate;

		if (shared::IsKeyPressed(VK_SUBTRACT) || shared::IsKeyPressed(VK_ADD))
		{
			player->m_pCharacterControl->m_nOnGroundState = 2;
			player->m_vecVelocity.y = 0.0f;
		}
	}
}

// Set ground for specific height
void cheat::GroundCheat() noexcept
{
	static bool once = false;
	if (groundCheat)
	{
		if (KeyBind::IsKeyPressed(groundCheatHotkey) && g_GameMenuStatus == InGame && OnFocus)
			groundEnabled = !groundEnabled;
		if (!once)
		{
			injector::MakeJMP(shared::base + 0xE6B45E, &GroundCheatCave, true);
			injector::MakeNOP(shared::base + 0xE6B463, 1, true);
			injector::MakeNOP(shared::base + 0x4E98CD, 3, true);
			once = true;
		}
	}
	else if (!groundCheat && once)
	{
		unsigned char rawBytes[6] = { 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00 };
		unsigned char rawBytes1[3] = { 0x89, 0x46, 0x10 };

		injector::WriteMemoryRaw(shared::base + 0xE6B45E, rawBytes, 6, true);
		injector::WriteMemoryRaw(shared::base + 0x4E98CD, rawBytes1, 3, true);
		once = false;
	}
}

// Toggles visor
void cheat::TemporaryVisorCheat() noexcept
{
	if (KeyBind::IsKeyPressed(temporaryVisorHotkey) && OnFocus)
		Trigger::GameFlags.GAME_PLAYER_VISOR_ENABLED ^= true;
}

// Regenerates health
void cheat::AutoHPUpCheat() noexcept
{
	Trigger::GameFlags.GAME_AUTO_HPUP = autoHpUp;
}

// Ninja run speed, you just need to type speed
void cheat::NinjaRunSpeedCheat() noexcept
{
	Pl0000* player = (Pl0000*)g_cGameUIManager.m_pPlayer;

	if (!player)
		return;

	ninjaRunSpeedRate = player->m_fNinjaRunSpeedRate;
}

// Literally time stop
void cheat::TimeStop() noexcept
{
	static cSlowRateManager* SlowRateManager = GetcSlowRateManager();
	if (KeyBind::IsKeyPressed(timeStopHotkey) && timeStop && OnFocus && g_GameMenuStatus == InGame)
	{
		timeStopSwitch = SlowRateManager->GetSlowRate(0) == 1.0f;

		if (!timeStopSwitch)
		{
			SlowRateManager->SetSlowRate(0, 1.0f);
			SlowRateManager->SetSlowRate(1, 1.0f);
			SlowRateManager->SetSlowRate(2, 1.0f);
		}
	}
	if (timeStopSwitch)
	{
		SlowRateManager->SetSlowRate(0, 1 * 3.33564095e-9f);
		SlowRateManager->SetSlowRate(1, 1 / 3.33564095e-9f);
		SlowRateManager->SetSlowRate(2, 1 * 3.33564095e-9f);
	}
}

void cheat::ChangeMission(unsigned int phaseId, const char* phaseName) {
	injector::WriteMemory<unsigned int>(shared::base + 0x1764670, phaseId, true);
	injector::WriteMemory<const char*>(shared::base + 0x1764674, phaseName, true);
}


void cheat::ChangeSamChargeObjectId() {
	if (SamChargeObject) injector::WriteMemory<unsigned int>(shared::base + 0x46C6C8, SamChargeObject, true);
}

void cheat::playerAttackType() {

	uintptr_t playerAttackTypeAddr = 0x00000000;

	int playerType = injector::ReadMemory<int>(shared::base + 0x017EA030, true);

	if (playerType == 8) playerAttackTypeAddr = shared::base + 0x129EBB4;

	if (playerType != 8 && playerType != 9) playerAttackTypeAddr = shared::base + 0x129CA1C;

	switch (cPlayerAttackType)
	{
		//RAIDEN
	case 0:
		injector::WriteMemory<unsigned int>(playerAttackTypeAddr, shared::base + 0x7E6E90, true);
		break;
		//SAM
	case 1:
		injector::WriteMemory<unsigned int>(playerAttackTypeAddr, shared::base + 0x46BC60, true);
		break;
		//BOSS SAM
	case 2:
		injector::WriteMemory<unsigned int>(playerAttackTypeAddr, shared::base + 0x1EE70, true);
		break;
		//ARMSTRONG
	case 3:
		injector::WriteMemory<unsigned int>(playerAttackTypeAddr, shared::base + 0x1B3060, true);
		break;
	default:
		break;
	}
}

void cheat::enemyNoDamageTo() {
	//Enemy startup change
	injector::WriteMemory<int>(shared::base + 0x6C7C80, enemyNoDamageToType, true);
}

void cheat::changeModelID() {
	int a3[24];
	a3[20] = 0;
	a3[21] = 0;
	a3[22] = 0;
	a3[23] = 244;
	a3[1] = 0;
	a3[0] = 0;
	uintptr_t *abc = NULL;
	//;
	//memcpy(abc, &shared::base + 0x017E9A98, sizeof(&shared::base + 0x017E9A98));
	//FuncA = (__funcA)(shared::base + 0x682090);
	//FuncA(abc,"Kamaitati", 0x3D070,a3);

	//FuncB = (__funcB)(shared::base + 0x46C610);
	//createEnemy(abc, "Kamaitati", 0x3D070, a3);

	uintptr_t modelAddress = 0x00000000;
	uintptr_t hairAddress = 0x00000000;
	uintptr_t headAddress = 0x00000000;
	uintptr_t sheathAddress = 0x00000000;
	uintptr_t visorAddress = 0x00000000;

	int currentModelID = injector::ReadMemory<int>(shared::base + 0x17E9FB4, true);

	if (!samModel) {

		//Standart armor
		if (currentModelID == 0) {
			modelAddress = shared::base + 0x14A9828;
			hairAddress = shared::base + 0x14A982C;
			headAddress = shared::base + 0x14A9838;
			sheathAddress = shared::base + 0x14A9834;
			visorAddress = shared::base + 0x14A9830;
		}

		//Blue
		if (currentModelID == 1) {
			modelAddress = shared::base + 0x14A983C;
			hairAddress = shared::base + 0x14A9840;
			headAddress = shared::base + 0x14A984C;
			sheathAddress = shared::base + 0x14A9848;
			visorAddress = shared::base + 0x14A9844;
		}

		
		//Red
		if (currentModelID == 2) {
			modelAddress = shared::base + 0x14A9850;
			hairAddress = shared::base + 0x14A9854;
			headAddress = shared::base + 0x14A9860;
			sheathAddress = shared::base + 0x14A985C;
			visorAddress = shared::base + 0x14A9858;
		}

		//Yellow
		if (currentModelID == 3) {
			modelAddress = shared::base + 0x14A9864;
			hairAddress = shared::base + 0x14A9868;
			headAddress = shared::base + 0x14A9874;
			sheathAddress = shared::base + 0x14A9870;
			visorAddress = shared::base + 0x14A986C;
		}

		//Desperado body
		if (currentModelID == 4) {
			modelAddress = shared::base + 0x14A9878;
			hairAddress = shared::base + 0x14A987C;
			headAddress = shared::base + 0x14A9888;
			sheathAddress = shared::base + 0x14A9884;
			visorAddress = shared::base + 0x14A9880;
		}

		//Costume
		if (currentModelID == 5) {
			modelAddress = shared::base + 0x14A988C;
			hairAddress = shared::base + 0x14A9890;
			headAddress = shared::base + 0x14A989C;
			sheathAddress = shared::base + 0x14A9898;
			visorAddress = shared::base + 0x14A9894;
		}

		//Mariachi
		if (currentModelID == 6) {
			modelAddress = shared::base + 0x14A98A0;
			hairAddress = shared::base + 0x14A98A4;
			headAddress = shared::base + 0x14A98B0;
			sheathAddress = shared::base + 0x14A98AC;
			visorAddress = shared::base + 0x14A98A8;
		}

		//Standart armor
		if (currentModelID == 7) {
			modelAddress = shared::base + 0x14A9918;
			hairAddress = shared::base + 0x14A991C;
			headAddress = shared::base + 0x14A9920;
			sheathAddress = shared::base + 0x14A9924;
			visorAddress = shared::base + 0x14A9928;
		}

		//Original
		if (currentModelID == 8) {
			modelAddress = shared::base + 0x14A9918;
			hairAddress = shared::base + 0x14A991C;
			headAddress = shared::base + 0x14A9920;
			sheathAddress = shared::base + 0x14A9924;
			visorAddress = shared::base + 0x14A9928;
		}

		//Gray Fox
		if (currentModelID == 9) {
			modelAddress = shared::base + 0x14A9918;
			hairAddress = shared::base + 0x14A991C;
			headAddress = shared::base + 0x14A9920;
			sheathAddress = shared::base + 0x14A9924;
			visorAddress = shared::base + 0x14A9928;
		}

		//White
		if (currentModelID == 10) {
			modelAddress = shared::base + 0x14A9918;
			hairAddress = shared::base + 0x14A991C;
			headAddress = shared::base + 0x14A9920;
			sheathAddress = shared::base + 0x14A9924;
			visorAddress = shared::base + 0x14A9928;
		}

		//Inferno
		if (currentModelID == 11) {
			modelAddress = shared::base + 0x14A9918;
			hairAddress = shared::base + 0x14A991C;
			headAddress = shared::base + 0x14A9920;
			sheathAddress = shared::base + 0x14A9924;
			visorAddress = shared::base + 0x14A9928;
		}


		//Commandos armor
		if (currentModelID == 12) {
			modelAddress = shared::base + 0x14A9918;
			hairAddress = shared::base + 0x14A991C;
			headAddress = shared::base + 0x14A9920;
			sheathAddress = shared::base + 0x14A9924;
			visorAddress = shared::base + 0x14A9928;
		}
	}

	if (samModel) {
		modelAddress = shared::base + 0x14A9954;
		hairAddress = shared::base + 0x14A9958;
		headAddress = shared::base + 0x14A9964;
		sheathAddress = shared::base + 0x14A9960;
		visorAddress = shared::base + 0x14A995C;
	}



	if (modelAddress != 0x00000000 && modelId)   injector::WriteMemory<unsigned int>(modelAddress, modelId, true);
	if (hairAddress != 0x00000000 && hairId)     injector::WriteMemory<unsigned int>(hairAddress, hairId, true);
	if (headAddress != 0x00000000 && headId)     injector::WriteMemory<unsigned int>(headAddress, headId, true);
	if (sheathAddress != 0x00000000 && sheathId)   injector::WriteMemory<unsigned int>(sheathAddress, sheathId, true);
	if (visorAddress != 0x00000000 && visorId)    injector::WriteMemory<unsigned int>(visorAddress, visorId, true);
		
}

void cheat::enablePl1400RipperMode() // noexcept - необезательно(используется для оптимизации)
{
	static bool once = false;

	if (fixPl1400Ripper && !once)
	{
		injector::WriteMemory<int>(shared::base + 0x129EBB4, shared::base + 0x7E6E90, true);
		injector::WriteMemory<int>(shared::base + 0x129EDC8, shared::base + 0x7C3370, true);
		injector::WriteMemory<int>(shared::base + 0x129EAE4, shared::base + 0x8104B0, true);

		once = true;
	}
	else if (!fixPl1400Ripper && once)
	{
		injector::WriteMemory<int>(shared::base + 0x129EBB4, shared::base + 0x46BC60, true);
		injector::WriteMemory<int>(shared::base + 0x129EDC8, shared::base + 0x6C3700, true);
		injector::WriteMemory<int>(shared::base + 0x129EAE4, shared::base + 0x69D3D0, true);

		once = false;
	}
}

void cheat::Deal0Damage() noexcept
{
	static bool once = false;
	if (dealZeroDamage && !once)
	{
		injector::MakeNOP(shared::base + 0x68EE34, 6, true);
		once = true;
	}
	else if (!dealZeroDamage && once)
	{
		unsigned char original[6] = { 0x29, 0x81, 0x70, 0x08, 0x00, 0x00 };
		injector::WriteMemoryRaw(shared::base + 0x68EE34, original, 6, true);
		once = false;
	}
}

void cheat::InfVRTimer() noexcept
{
	static bool readOnce = false;
	static unsigned char original[6];
	if (!readOnce)
	{
		injector::ReadMemoryRaw(shared::base + 0x81B440, original, 6, true);
		readOnce = true;
	}
	static bool once = false;
	if (infTimer && !once)
	{
		injector::MakeNOP(shared::base + 0x81B440, 6, true);
		injector::MakeJMP(shared::base + 0x81B440, &InfiniteVRTimerCave, true);
		once = true;
	}
	else if (!infTimer && once)
	{
		injector::WriteMemoryRaw(shared::base + 0x81B440, original, 6, true);
		once = false;
	}
}

void cheat::BattlePointsChange() noexcept
{
	auto playerManager = g_pPlayerManagerImplement;
	if (!playerManager)
		return;

	battlePoints = playerManager->m_nBattlePoints;
}

void cheat::ChangePlayerSlowRate() noexcept
{
	Pl0000* player = (Pl0000*)g_cGameUIManager.m_pPlayer;

	if (!player)
		return;

	playerSlowRate = player->m_pEntity->m_pSlowRateUnit->m_fCurrentSlowRate;
}

// Handles all cheats at once
void cheat::HandleCheats() noexcept
{
	static bool once = false;
	if (!once)
	{
		LoadConfig();
		once = true;
	}
	// Player
	enablePl1400RipperMode();
	playerAttackType();
	MugenZangekiCheat();
	InfiniteHealthCheat();
	InfiniteSubWeaponCheat();
	HeightChangeCheat();
	TemporaryVisorCheat();
	AutoHPUpCheat();
	NinjaRunSpeedCheat();
	TimeStop();
	BattlePointsChange();
	ChangePlayerSlowRate();

	// Enemies
	OneHitKillCheat();
	Deal0Damage();
	enemyNoDamageTo();

	// Battle
	NoDamageStatCheat();
	StealthCheat();
	InfVRTimer();
	
	// Entities
	GroundCheat();
	ChangeSamChargeObjectId();
}

// Loads config (ini file)
void cheat::LoadConfig() noexcept
{
	CIniReader iniReader("ModMenu.ini");

	infiniteFc = iniReader.ReadInteger("Player", "InfFuelContainer", 0) == 1;
	infiniteHealth = iniReader.ReadInteger("Player", "InfHealth", 0) == 1;
	infiniteSubWeapon = iniReader.ReadInteger("Player", "InfSubWeapon", 0) == 1;
	heightChange = iniReader.ReadInteger("Player", "HeightChange", 0) == 1;
	heightRate = iniReader.ReadFloat("Player", "HeightRate", 0.0f);
	temporaryVisorHotkey = iniReader.ReadInteger("Player", "VisorHotkey", 80);
	autoHpUp = iniReader.ReadInteger("Player", "AutoHpUp", 0) == 1;
	dealZeroDamage = iniReader.ReadInteger("Player", "DealZeroDamage", 0) == 1;
	
	

	timeStop = iniReader.ReadInteger("Entities", "TimeStop", 0) == 1;
	timeStopHotkey = iniReader.ReadInteger("Entities", "TimeStopHotkey", 84);
	groundCheat = iniReader.ReadInteger("Entities", "GroundCheatEnabled", 0) == 1;
	groundCheatHotkey = iniReader.ReadInteger("Entities", "GroundCheatHotkey", 75);

	oneHitKill = iniReader.ReadInteger("Enemies", "OneHitKill", 0) == 1;

	noDamageStat = iniReader.ReadInteger("Battle", "NoDamageStat", 0) == 1;
	stealth = iniReader.ReadInteger("Battle", "Stealth", 0) == 1;
	infTimer = iniReader.ReadInteger("Battle", "InfiniteTimer", 0) == 1;

	fixPl1400Ripper = iniReader.ReadBoolean("Player", "fixPl1400Ripper", 0) == 1;
	enemyNoDamageToType = iniReader.ReadInteger("Player", "enemyNoDamageToType", 2);


}

// Saves config (ini file)
void cheat::SaveConfig() noexcept
{
	CIniReader iniReader("ModMenu.ini");

	iniReader.WriteInteger("Player", "InfFuelContainer", infiniteFc);
	iniReader.WriteInteger("Player", "InfHealth", infiniteHealth);
	iniReader.WriteInteger("Player", "InfSubWeapon", infiniteSubWeapon);
	iniReader.WriteInteger("Player", "HeightChange", heightChange);
	iniReader.WriteFloat("Player", "HeightRate", heightRate);
	iniReader.WriteInteger("Player", "VisorHotkey", temporaryVisorHotkey);
	iniReader.WriteInteger("Player", "AutoHpUp", autoHpUp);
	iniReader.WriteInteger("Player", "DealZeroDamage", dealZeroDamage);

	iniReader.WriteInteger("Entities", "TimeStop", timeStop);
	iniReader.WriteInteger("Entities", "TimeStopHotkey", timeStopHotkey);
	iniReader.WriteInteger("Entities", "GroundCheatEnabled", groundCheat);
	iniReader.WriteInteger("Entities", "GroundCheatHotkey", groundCheatHotkey);

	iniReader.WriteInteger("Enemies", "OneHitKill", oneHitKill);
	iniReader.WriteInteger("Enemies", "enemyNoDamageTo", enemyNoDamageToType);

	iniReader.WriteInteger("Battle", "NoDamageStat", noDamageStat);
	iniReader.WriteInteger("Battle", "Stealth", stealth);
	iniReader.WriteInteger("Battle", "InfiniteTimer", infTimer);

	iniReader.WriteBoolean("Player", "fixPl1400Ripper", fixPl1400Ripper);
}

// Resets cheats
void cheat::Reset() noexcept
{
	infiniteFc = false;
	infiniteHealth = false;
	oneHitKill = false;
	infiniteSubWeapon = false;
	noDamageStat = false;
	stealth = false;
	heightRate = 0.0f;
	heightChange = false;
	groundCheat = false;
	groundEnabled = false;
	groundCheatHotkey = 75;
	temporaryVisorHotkey = 80;
	visorSwitch = false;
	autoHpUp = false;
	ninjaRunSpeedRate = 0.9f;
	timeStop = false;
	timeStopHotkey = 84;
	dealZeroDamage = false;
	infTimer = false;
	playerSlowRate = 1.0f;
	modelId = 0x00000000;
}