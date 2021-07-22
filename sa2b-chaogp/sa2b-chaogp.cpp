#include "stdafx.h"

static Trampoline* LoadLevelInit_t = nullptr;
static Trampoline* LoadLevelManager_t = nullptr;
static Trampoline* LoadLevelDestroy_t = nullptr;
static Trampoline* ChangeChaoStage_t = nullptr;

static bool ChaoPowerups = false;
bool ChaoAssist = true;
bool ChaoLuck = true;

ChaoLeash CarriedChao[2] = {};

static void SetChaoPowerups(int id, ChaoData* chaodata)
{
	if (ChaoPowerups == true) {
		CharObj2Base* co2 = MainCharObj2[id];

		if (co2)
		{
			co2->PhysData.RunSpeed += static_cast<float>(min(99, chaodata->data.StatLevels[ChaoStat_Run])) / 500.0f;
			co2->PhysData.JumpSpeed += static_cast<float>(min(99, chaodata->data.StatLevels[ChaoStat_Power])) / 300.0f;
			co2->PhysData.SpeedCapH += static_cast<float>(min(99, chaodata->data.StatLevels[ChaoStat_Stamina])) / 200.0f;
		}
	}
}

static void LoadChaoLevel(int id)
{
	EntityData1* player = MainCharObj1[id];
	NJS_VECTOR pos = { 0, 0, 0 };

	if (player && CarriedChao[id].mode == ChaoLeashMode_Free)
	{
		PutBehindPlayer(&pos, player, 5.0f);
	}

	ObjectMaster* chao = CreateChao(CarriedChao[id].data, 0, 0, &pos, 0);
	chao->Data1.Chao->entity.field_2 = id;
	SetChaoPowerups(id, CarriedChao[id].data);

	if (CarriedChao[id].mode == ChaoLeashMode_Fly)
	{
		chao->Data1.Chao->entity.Status |= StatusChao_FlyPlayer;
	}
}

static void ClearChao(int player)
{
	if (CarriedChao[player].mode != ChaoLeashMode_Disabled)
	{
		delete CarriedChao[player].data;
		CarriedChao[player].mode = ChaoLeashMode_Disabled;
	}
}

// Load chao data at the level initialization
static void LoadLevelInit_r()
{
	TARGET_DYNAMIC(LoadLevelInit)();

	// The carried chao is removed if going back to the chao world
	// If it's a level, we load chao data

	if ((CarriedChao[0].mode != ChaoLeashMode_Disabled || CarriedChao[1].mode != ChaoLeashMode_Disabled) && 
		(CurrentLevel < 70 || CurrentLevel > 71))
	{
		if (CurrentLevel == LevelIDs_ChaoWorld)
		{
			ClearChao(0);
			ClearChao(1);
		}
		else
		{
			LoadTextureList((char*)"AL_MINI_PARTS_TEX", (NJS_TEXLIST*)0x1366AE4);
			LoadTextureList((char*)"AL_BODY", (NJS_TEXLIST*)0x13669FC);
			LoadTextureList((char*)"AL_jewel", (NJS_TEXLIST*)0x1366AD4);
			LoadTextureList((char*)"AL_ICON", (NJS_TEXLIST*)0x1366ACC);
			LoadTextureList((char*)"AL_EYE", (NJS_TEXLIST*)0x1366AC4);
			LoadTextureList((char*)"AL_MOUTH", (NJS_TEXLIST*)0x1366ADC);
			LoadTextureList((char*)"AL_COMMON_TEX", (NJS_TEXLIST*)0x1366AB4);
			LoadTextureList((char*)"AL_TEX_COMMON", (NJS_TEXLIST*)0x1366ABC);
			LoadChaoPalette();
		}
	}
}

// Load the chao itself at each load/restart
static void LoadLevelManager_r()
{
	TARGET_DYNAMIC(LoadLevelManager)();

	if (CurrentLevel != LevelIDs_ChaoWorld && (CurrentLevel < 70 || CurrentLevel > 71))
	{
		if (CarriedChao[0].mode != ChaoLeashMode_Disabled)
		{
			LoadChaoLevel(0);
		}
		
		if (CarriedChao[1].mode != ChaoLeashMode_Disabled)
		{
			LoadChaoLevel(1);
		}
	}
}

// Delete the chao files when the level is finished/exited
static void* LoadLevelDestroy_r()
{
	if (CarriedChao && (CurrentLevel < 70 || CurrentLevel > 71))
	{
		FreeTexList((NJS_TEXLIST*)0x1366AE4);
		FreeTexList((NJS_TEXLIST*)0x13669FC);
		FreeTexList((NJS_TEXLIST*)0x1366AD4);
		FreeTexList((NJS_TEXLIST*)0x1366ACC);
		FreeTexList((NJS_TEXLIST*)0x1366AC4);
		FreeTexList((NJS_TEXLIST*)0x1366ADC);
		FreeTexList((NJS_TEXLIST*)0x1366AB4);
		FreeTexList((NJS_TEXLIST*)0x1366ABC);
	}

	return TARGET_DYNAMIC(LoadLevelDestroy)();
}

// Select the chao when leaving a garden
static BYTE* __cdecl ChangeChaoStage_r(int area) {
	if (area == 7)
	{
		for (int i = 0; i < 2; ++i)
		{
			if (CarriedChao[i].mode == ChaoLeashMode_Disabled)
			{
				if (MainCharObj2[i] && MainCharObj2[i]->HeldObject)
				{
					ChaoData1* data = MainCharObj2[i]->HeldObject->Data1.Chao;

					if (data && data->ChaoDataBase_ptr)
					{
						// Loop through the chao slots to get if it's a valid chao
						for (uint8_t j = 0; j < 24; ++j)
						{
							if (&ChaoSlots[j].data == data->ChaoDataBase_ptr)
							{
								if (data->ChaoDataBase_ptr->Type != ChaoType_Empty && data->ChaoDataBase_ptr->Type != ChaoType_Egg)
								{
									CarriedChao[i].mode = ChaoLeashMode_Fly;
									CarriedChao[i].data = new ChaoData;
									memcpy(CarriedChao[i].data, &ChaoSlots[j], sizeof(ChaoData));
								}
							}
						}
					}
				}
			}
		}
	}

	return TARGET_DYNAMIC(ChangeChaoStage)(area);
}

extern "C"
{
	__declspec(dllexport) void Init(const char* path, const HelperFunctions& helperFunctions)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		ChaoPowerups = config->getBool("Functionalities", "EnablePowerups", false);
		ChaoAssist = config->getBool("Functionalities", "EnableChaoAssist", true);
		ChaoLuck = config->getBool("Functionalities", "EnableChaoLuck", true);
		delete config;

		LoadLevelInit_t = new Trampoline(0x43CB10, 0x43CB16, LoadLevelInit_r);
		LoadLevelManager_t = new Trampoline(0x43CB50, 0x43CB56, LoadLevelManager_r);
		LoadLevelDestroy_t = new Trampoline(0x454CC0, 0x454CC8, LoadLevelDestroy_r);
		ChangeChaoStage_t = new Trampoline(0x52B5B0, 0x52B5B6, ChangeChaoStage_r);

		#ifndef NDEBUG
		CarriedChao[0].data = new ChaoData();
		CarriedChao[0].data->data.Type == ChaoType_Child;
		CarriedChao[0].mode = ChaoLeashMode_Fly;
		#endif
	}

	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };
}