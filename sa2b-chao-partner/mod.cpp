#include "stdafx.h"
#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"
#include "common.h"
#include "utils.h"

FunctionHook<void> LoadLevelInit_hook(0x43CB10);
FunctionHook<void> LoadLevelManager_hook(0x43CB50);
FunctionHook<void> LoadLevelDestroy_hook(0x454CC0);
FunctionHook<BYTE*, int> ChangeChaoStage_hook(0x52B5B0);
UsercallFunc(BOOL, EnemyCheckDamage_hook, (EntityData1* data, EnemyData* edata), (data, edata), 0x47AA70, rEAX, rEAX, stack4);

static bool ChaoPowerups = false;
bool ChaoAssist = false;
//bool ChaoLuck = true;

ChaoLeash CarriedChao[2] = {};

static void(*ChaoConstructor_CWE)();

static void ChaoConstructor_Level()
{
	if (ChaoConstructor_CWE)
	{
		ChaoConstructor_CWE();
	}
	else
	{
		LoadTextureList((char*)"AL_MINI_PARTS_TEX", (NJS_TEXLIST*)0x1366AE4);
	}

	LoadTextureList((char*)"AL_BODY", (NJS_TEXLIST*)0x13669FC);
	LoadTextureList((char*)"AL_jewel", (NJS_TEXLIST*)0x1366AD4);
	LoadTextureList((char*)"AL_ICON", (NJS_TEXLIST*)0x1366ACC);
	LoadTextureList((char*)"AL_EYE", (NJS_TEXLIST*)0x1366AC4);
	LoadTextureList((char*)"AL_MOUTH", (NJS_TEXLIST*)0x1366ADC);
	LoadTextureList((char*)"AL_COMMON_TEX", (NJS_TEXLIST*)0x1366AB4);
	LoadTextureList((char*)"AL_TEX_COMMON", (NJS_TEXLIST*)0x1366ABC);
	LoadChaoPalette();
}

static void SetChaoPowerups(int id, ChaoData* chaodata)
{
	if (ChaoPowerups == true)
	{
		auto pwp = MainCharObj2[id];

		if (pwp)
		{
			pwp->PhysData.RunSpeed += static_cast<float>(min(99, chaodata->data.StatLevels[ChaoStat_Run])) / 500.0f;
			pwp->PhysData.JumpSpeed += static_cast<float>(min(99, chaodata->data.StatLevels[ChaoStat_Power])) / 300.0f;
			pwp->PhysData.SpeedCapH += static_cast<float>(min(99, chaodata->data.StatLevels[ChaoStat_Stamina])) / 200.0f;
		}
	}
}

VoidFunc(sub_530B80, 0x530B80);
DataPointer(void*, dword_1A0F94C, 0x1A0F94C);

static void LoadChaoLevel(int id)
{
	if (!dword_1A0F94C)
	{
		sub_530B80();
	}

	auto player = MainCharObj1[id];
	NJS_VECTOR pos = { 0.0f, 0.0f, 0.0f };

	if (player && CarriedChao[id].mode == ChaoLeashMode_Free)
	{
		PutBehindPlayer(&pos, player, 5.0f);
	}

	auto chao = CreateChao(CarriedChao[id].data, 0, 0, &pos, 0);
	chao->Data1.Chao->entity.Index = id;
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
	LoadLevelInit_hook.Original();

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
			ChaoConstructor_Level();
		}
	}
}

// Load the chao itself at each load/restart
static void LoadLevelManager_r()
{
	LoadLevelManager_hook.Original();

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
static void LoadLevelDestroy_r()
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

	LoadLevelDestroy_hook.Original();
}

// Select the chao when leaving a garden
static BYTE* __cdecl ChangeChaoStage_r(int area) {
	if (area == 7)
	{
		for (int i = 0; i < 2; ++i)
		{
			if (CarriedChao[i].mode != ChaoLeashMode_Disabled)
			{
				continue;
			}

			auto pwp = MainCharObj2[i];

			if (!pwp || !pwp->HeldObject)
			{
				continue;
			}

			ChaoData1* data = pwp->HeldObject->Data1.Chao;

			if (!data || !data->ChaoDataBase_ptr)
			{
				continue;
			}

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

	return ChangeChaoStage_hook.Original(area);
}

BOOL EnemyCheckDamage_r(EntityData1* data, EnemyData* edata)
{
	if (data->Collision->CollidingObject && data->Collision->CollidingObject->Object &&
		data->Collision->CollidingObject->Object->MainSub == Chao_Main)
	{
		auto chao_data = data->Collision->CollidingObject->Object->Data1.Chao;
		
		if (chao_data && chao_data->entity.NextAction == ChaoAct_Attack)
		{
			return TRUE;
		}
	}

	return EnemyCheckDamage_hook.Original(data, edata);
}

extern "C"
{
	__declspec(dllexport) void Init(const char* path, const HelperFunctions& helperFunctions)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		ChaoPowerups = config->getBool("Functionalities", "EnablePowerups", false);
		ChaoAssist = config->getBool("Functionalities", "EnableChaoAssist", false);
		//ChaoLuck = config->getBool("Functionalities", "EnableChaoLuck", true);
		delete config;

		LoadLevelInit_hook.Hook(LoadLevelInit_r);
		LoadLevelManager_hook.Hook(LoadLevelManager_r);
		LoadLevelDestroy_hook.Hook(LoadLevelDestroy_r);
		ChangeChaoStage_hook.Hook(ChangeChaoStage_r);
		EnemyCheckDamage_hook.Hook(EnemyCheckDamage_r);

		#ifndef NDEBUG
		CarriedChao[0].data = new ChaoData();
		CarriedChao[0].data->data.Type = ChaoType_Child;
		CarriedChao[0].data->data.StatLevels[ChaoStat_Stamina] = 99;
		CarriedChao[0].data->data.StatLevels[ChaoStat_Fly] = 99;
		CarriedChao[0].data->data.StatLevels[ChaoStat_Run] = 99;
		CarriedChao[0].data->data.StatLevels[ChaoStat_Power] = 99;
		CarriedChao[0].mode = ChaoLeashMode_Fly;
		#endif

		// Compatibility with Chao World Extended
		auto cwe = GetModuleHandle(L"CWE");
		if (cwe != nullptr)
		{
			ChaoConstructor_CWE = (void(*)())GetProcAddress(cwe, "ChaoMain_Constructor_Hook");
		}
	}

	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };
}
