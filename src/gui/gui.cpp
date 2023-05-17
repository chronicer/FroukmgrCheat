#include "gui.h"
#include <Windows.h>
#include <string>
#include "../cheat/cheat.h"
#include "../injector/injector.hpp"
#include "../IniReader.h"
#include "../KeyBinds.h"
#include "../KeyBind.h"
#include "../kiero.h"
#include "../minhook/include/MinHook.h"

#include <Pl0000.h>
#include <cGameUIManager.h>
#include <GameMenuStatus.h>
#include <PlayerManagerImplement.h>
#include <Trigger.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx9.h"

#pragma warning(disable : 4996)

bool once1 = false;

static const char* enemyNoDamageTypes[] = { "0 - damage to all","1 - no damage to player","2 - no damage for enemies (default)" };
static const char* enemyNoDamageTo_selected = enemyNoDamageTypes[2];

static const char* playerAttackTypes[] = { "Raiden", "Sam", "Boss Sam", "Armstrong" };


/*************************** MISSIONS ***************************/

static const char* missions[7] = { "1","2","3","4","5","6","7" };
static const char* selectedMission = missions[0];
int selectedMissionID = 0;

static const char* phaseIds[7][7] = {
	{"0118","0128","0138","0140","0150","0168","0170"},
	{"0210","0220"}
};

static const char* phaseNames[25][25][25] =
{
	{
		{"P118_START","P118_BEACH","P118_BTL","P118_BTL_IN","P118_BTL_END","P118_CYB_EVENT","P118_TUTORIAL","P118_EVENT_END","P118_END"},
		{"P128_START","P128_GATE","P128_GATE_2F","P128_GATE2","P128_STREET","P128_LOAD","P128_STREET_RESTART","P128_BTL","P128_BTL_START","P128_BTL_ADD","P128_BTL_END","P128_STREET2","P128_EVENT","P128_END"},
		{"P138_START","P138_MOVIE","P138_WOLF","P138_WOLF_SHOW","P138_WOLF_FADE","P138_IN","P138_RE_START","P138_BREAKDOWN_1","P138_BREAKDOWN_2","P138_HELI01_START","P138_HELI01","P138_HELI01_END","P138_SET"},
		{"P140_START","P140_IN","P140_DOOR_CHECK","P140_ADD_SET","P140_DOGTAG_GET","P140_DOGTAG_OUT","P140_GOOD_OPEN","P140_BAD_OPEN","P140_HOTEL_BTL","P140_HOTEL_BTL_END","P140_HOTEL_IN","P140_EVENT"},
		{"P150_START","P150_RESTART","P150_BTL","P150_BTL_SET","P150_BTL_END","P150_HELI"},
		{"P168_START","P168_FACTORY","P168_BTL_END","P168_IN","P168_FLOOR","P168_FLOOR2","P168_FLOOR3","P168_END"},
		{"P170_START","P170_MOVIE","P170_MISTRAL","P170_MISTRAL01","P170_MISTRAL02","P170_MISTRAL03","P170_MIST_RESULT","P170_MIST_DEAD","P170_END","P170_TEST"}
	},
	{
		{"P210_SEWER_START","P210_SEWER_MOVIE","P210_SEWER_START_RADIO","P210_SEWER","P210_MASTIFF_EV","P210_MASTIFF","P210_SEWER_2","P210_SEWER_BURAN","P210_EVENT","P210_SEWER_END"}
	}
};


static const char* selectedPhaseId = phaseIds[0][0];
static const char* selectedPhaseName = phaseNames[0][0][0];
int selectedPhase = 0;
int selectedPhaseN = 0;

/*************************** MISSIONS ***************************/

static const char* playerAttack_selected = playerAttackTypes[0];

// Renders gui for cheats
void gui::RenderGUI() noexcept
{
	Pl0000* player = (Pl0000*)g_cGameUIManager.m_pPlayer;

	if (!once1)
	{
		LoadConfig();
		once1 = true;
	}

	static bool paused = false;

	if (KeyBind::IsKeyPressed(menuKey) && cheat::OnFocus)
		show = !show;

	if (show && g_GameMenuStatus == InGame)
	{
		Trigger::StaFlags.STA_PAUSE = true;
		paused = true;
	}

	if (!show && paused && g_GameMenuStatus == InGame)
	{
		Trigger::StaFlags.STA_PAUSE = false;
		paused = false;
	}

	/* :: REMOVED, REASON: Its lagging game because of hooking EndScene
	if (!show)
		Sleep(20);
	*/
	if (!show)
		Trigger::StpFlags.STP_GAME_UPDATE = false;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (show)
	{
		Trigger::StpFlags.STP_GAME_UPDATE = true;
		ImGui::Begin("Mod Menu", NULL, ImGuiWindowFlags_NoCollapse);
		ImGui::SetNextWindowSize({width, height});
		ImGuiIO io = ImGui::GetIO();
		io.MouseDrawCursor = true;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		if (ImGui::BeginTabBar("NOTITLE", ImGuiTabBarFlags_NoTooltip))
		{
			if (ImGui::BeginTabItem("Player"))
			{
				ImGui::Checkbox("Enable Sam ripper mode", &cheat::fixPl1400Ripper);

				ImGui::Checkbox("Infinite Fuel Container", &cheat::infiniteFc);
				ImGui::Checkbox("Infinite Health", &cheat::infiniteHealth);
				ImGui::Checkbox("Infinite Subweapon Ammo & Supplies", &cheat::infiniteSubWeapon);

				if (ImGui::BeginCombo("Player Attack Type", playerAttack_selected))
				{
					for (int n = 0; n < IM_ARRAYSIZE(playerAttackTypes); n++)
					{
						bool is_selected = (playerAttack_selected == playerAttackTypes[n]);
						if (ImGui::Selectable(playerAttackTypes[n], is_selected)) {
							playerAttack_selected = playerAttackTypes[n];
							cheat::cPlayerAttackType = n;
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}



				ImGui::Checkbox("Height Change (numpad +, -)", &cheat::heightChange);
				ImGui::SliderFloat("Height Change Rate", &cheat::heightRate, 0.0f, 100.0f, "%.3f", 1.0f);
				KeyBind::Hotkey("Visor Hotkey: ", &cheat::temporaryVisorHotkey);
				ImGui::Checkbox("Auto HP Up", &cheat::autoHpUp);
				if (ImGui::InputFloat("Ninja Run Speed Rate", &cheat::ninjaRunSpeedRate) && player)
						player->m_fNinjaRunSpeedRate = cheat::ninjaRunSpeedRate;
				if (ImGui::Button("Toggle ripper mode") && player)
					if (player->m_nRipperModeEnabled)
						player->DisableRipperMode(false);
					else
						player->EnableRipperMode();
				if (ImGui::Button("Toggle ripper mode effect") && player)
				{
					static bool ripperModeEffectSwitch = false;
					ripperModeEffectSwitch ^= true;
					if (ripperModeEffectSwitch && !player->m_nRipperModeEnabled)
						player->CallEffect(100, &player->field_3470);
					else
						player->field_3470.SetEffectDuration(0.1f, 0.0f);
				}
				if (ImGui::InputInt("Battle points", &cheat::battlePoints, 100, 500) && g_pPlayerManagerImplement)
					g_pPlayerManagerImplement->m_nBattlePoints = cheat::battlePoints;

				ImGui::InputInt("Battle points in customize menu", &*(int*)(shared::base + 0x177589C), 100, 500);
				if (player)
				{
					ImGui::InputFloat("Player speed", &cheat::playerSlowRate);
					player->m_pEntity->m_pSlowRateUnit->m_fCurrentSlowRate = cheat::playerSlowRate;
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Entities"))
			{
				ImGui::Checkbox("Ground Cheat", &cheat::groundCheat);
				if (cheat::groundCheat)
					KeyBind::Hotkey("Hotkey: ", &cheat::groundCheatHotkey);
				ImGui::Checkbox("Time Stop", &cheat::timeStop);
				if (cheat::timeStop)
					KeyBind::Hotkey("Time Stop hotkey: ", &cheat::timeStopHotkey);

				ImGui::InputScalar("Sam Charge Object ID", ImGuiDataType_U32, &cheat::SamChargeObject, NULL, NULL, "%X", 2);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Enemies"))
			{
				ImGui::Checkbox("One Hit Kill", &cheat::oneHitKill);
				ImGui::Checkbox("Deal Zero Damage", &cheat::dealZeroDamage);



				if (ImGui::BeginCombo("Enemy no damage to", enemyNoDamageTo_selected))
				{
					for (int n = 0; n < IM_ARRAYSIZE(enemyNoDamageTypes); n++)
					{
						bool is_selected = (enemyNoDamageTo_selected == enemyNoDamageTypes[n]);
						if (ImGui::Selectable(enemyNoDamageTypes[n], is_selected)) {
							enemyNoDamageTo_selected = enemyNoDamageTypes[n];
							cheat::enemyNoDamageToType = n;
						}
							
							if (is_selected)
								ImGui::SetItemDefaultFocus(); 
					}
					ImGui::EndCombo();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Battle"))
			{
				ImGui::Checkbox("No Damage Status for battle", &cheat::noDamageStat);
				ImGui::Checkbox("Stealth Cheat", &cheat::stealth);
				ImGui::Checkbox("Infinite battle/VR timer", &cheat::infTimer);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Model"))
			{
				ImGui::InputScalar("Model ID", ImGuiDataType_U32, &cheat::modelId, NULL, NULL, "%X", 2);
				ImGui::InputScalar("Hair ID", ImGuiDataType_U32, &cheat::hairId, NULL, NULL, "%X", 2);
				ImGui::InputScalar("Head ID", ImGuiDataType_U32, &cheat::headId, NULL, NULL, "%X", 2);
				ImGui::InputScalar("Sheath ID", ImGuiDataType_U32, &cheat::sheathId, NULL, NULL, "%X", 2);
				ImGui::InputScalar("Visor ID", ImGuiDataType_U32, &cheat::visorId, NULL, NULL, "%X", 2);
				ImGui::Checkbox("For Sam", &cheat::samModel);
				if (ImGui::Button("Change model")) {
					cheat::changeModelID();
				}

				ImGui::EndTabItem();
			}


			if (ImGui::BeginTabItem("Missions"))
			{

				if (ImGui::BeginCombo("Mission", selectedMission))
				{
					for (int n = 0; n < IM_ARRAYSIZE(missions); n++)
					{
						bool is_selected = (selectedMission == missions[n]);
						if (ImGui::Selectable(missions[n], is_selected)) {
							selectedMission = missions[n];
							selectedMissionID = n;
							selectedPhase = 0;
							selectedPhaseId = phaseIds[selectedMissionID][0];
							selectedPhaseName = phaseNames[selectedMissionID][0][0];
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("Phase ID", selectedPhaseId))
				{
					for (int n = 0; n < IM_ARRAYSIZE(phaseIds[selectedMissionID]); n++)
					{
						if (phaseIds[selectedMissionID][n] == "" || phaseIds[selectedMissionID][n] == NULL) break;

						bool is_selected = (selectedPhaseId == phaseIds[selectedMissionID][n]);
						if (ImGui::Selectable(phaseIds[selectedMissionID][n], is_selected)) {
							selectedPhaseId = phaseIds[selectedMissionID][n];
							selectedPhase = n;
							selectedPhaseName = phaseNames[selectedMissionID][selectedPhase][0];
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (ImGui::BeginCombo("Phase name", selectedPhaseName))
				{
					for (int n = 0; n < IM_ARRAYSIZE(phaseNames[selectedMissionID][selectedPhase]); n++)
					{
						if (phaseNames[selectedMissionID][selectedPhase][n] == "" || phaseNames[selectedMissionID][selectedPhase][n] == NULL) break;
						bool is_selected = (selectedPhaseName == phaseNames[selectedMissionID][selectedPhase][n]);
						if (ImGui::Selectable(phaseNames[selectedMissionID][selectedPhase][n], is_selected)) {
							selectedPhaseName = phaseNames[selectedMissionID][selectedPhase][n];
							selectedPhaseN = n;
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}


				if (ImGui::Button("Change mission")) {
					cheat::ChangeMission(std::strtoul(phaseIds[selectedMissionID][selectedPhase],nullptr,16),phaseNames[selectedMissionID][selectedPhase][selectedPhaseN]);
				}

				ImGui::EndTabItem();
			}


			if (ImGui::BeginTabItem("Menu"))
			{
				KeyBind::Hotkey("Menu Key: ", &menuKey);
				if (ImGui::Button("Save Config"))
				{
					cheat::SaveConfig();
					SaveConfig();
				}
				if (ImGui::Button("Load Config"))
				{
					cheat::LoadConfig();
					LoadConfig();
				}
				if (ImGui::Button("Reset Config"))
				{
					cheat::Reset();
					Reset();

					cheat::SaveConfig();
					SaveConfig();
				}
				if (ImGui::Button("Donate"))
					ShellExecute(0, "open", "https://donatello.to/Frouk3", NULL, NULL, 0);
				ImGui::EndTabItem();
			}



			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

// Currently saves only menu hotkey
void gui::SaveConfig() noexcept
{
	CIniReader iniReader("ModMenu.ini");

	iniReader.WriteInteger("Menu", "OpenMenuHotkey", menuKey);
}

// Loads only hotkey variable
void gui::LoadConfig() noexcept
{
	CIniReader iniReader("ModMenu.ini");

	menuKey = iniReader.ReadInteger("Menu", "OpenMenuHotkey", 45);
}

// Resets gui variables
void gui::Reset() noexcept
{
	width = 900.0f;
	height = 600.0f;
	menuKey = 45;
}