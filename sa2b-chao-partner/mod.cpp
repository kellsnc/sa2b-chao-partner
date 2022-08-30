#include "stdafx.h"
#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"
#include "common.h"
#include "utils.h"
#include "chao.h"
#include "water.h"

FunctionHook<void> LoadLevelInit_hook(0x43CB10);
FunctionHook<void> LoadLevelManager_hook(0x43CB50);
FunctionHook<void> LoadLevelDestroy_hook(0x454CC0);
FunctionHook<BYTE*, int> ChangeChaoStage_hook(0x52B5B0);
FunctionHook<void, ObjectMaster*> ItemBoxAirExec_hook(0x6C8EF0);
UsercallFuncVoid(ItemBoxCollision_hook, (ObjectMaster* obj), (obj), 0x6C8090, rEDI);
UsercallFunc(BOOL, EnemyCheckDamage_hook, (EntityData1* data, EnemyData* edata), (data, edata), 0x47AA70, rEAX, rEAX, stack4);
UsercallFunc(BOOL, AL_GetRandomAttrPos_hook, (void* buf, NJS_POINT3* pos, int num), (buf, pos, num), 0x534F80, rEAX, rEAX, rEDI, stack4);

static bool ChaoPowerups = false;
bool ChaoAssist = false;
//bool ChaoLuck = true;

ChaoLeash CarriedChao[8] = {};

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

static void LoadChaoLevel(int id)
{
	if (!WorldMasterTask)
	{
		ALW_Create();
	}

	// Fake waypoints so that AL_GetRandomAttrPos runs
	if (!*(int*)0x1A1693C) *(int*)0x1A1693C = 1;
	if (!*(int*)0x1A1D958) *(int*)0x1A1D958 = 1;
	if (!*(int*)0x1A1E95C) *(int*)0x1A1E95C = 1;

	auto player = MainCharObj1[id];
	NJS_POINT3 pos = { 0.0f, 0.0f, 0.0f };

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

bool CheckCollisionWithChao(EntityData1* data, int* pnum)
{
	if (data && data->Collision &&
		data->Collision->CollidingObject && data->Collision->CollidingObject->Object &&
		data->Collision->CollidingObject->Object->MainSub == Chao_Main)
	{
		auto chao_data = data->Collision->CollidingObject->Object->Data1.Chao;

		if (chao_data && chao_data->entity.NextAction == ChaoAct_Attack)
		{
			if (pnum) *pnum = chao_data->entity.Index;
			return true;
		}
	}

	return false;
}

BOOL EnemyCheckDamage_r(EntityData1* data, EnemyData* edata)
{
	if (CheckCollisionWithChao(data, nullptr))
	{
		edata->flag |= 0x1000u;
		edata->buyoscale = *(Float*)0xA3D28C;
		return true;
	}

	return EnemyCheckDamage_hook.Original(data, edata);
}

void ItemBoxCollision_r(ObjectMaster* obj)
{
	int pnum;
	if (CheckCollisionWithChao(obj->Data1.Entity, &pnum))
	{
		auto data = obj->Data1.Entity;
		DisplayItemBoxItem(pnum, ItemBox_Items[(int)obj->UnknownA_ptr].Texture);
		ItemBox_Items[(int)obj->UnknownA_ptr].Code(obj, pnum);
		data->Status |= 0x4;
		data->Action = 3;

		NJS_POINT3 pos = { data->Position.x, data->Position.y + 7.5f, data->Position.z };
		NJS_POINT3 spd = { 0.0f, 0.1f, 0.0f };
		CreateItemBoxSmoke(&pos, &spd, 6.0f);

		PlaySoundProbably(32781, 0, 0, 0);

		if (obj->SETData)
		{
			obj->SETData->Flags |= 0x4;
		}
		
		if (data->NextAction && obj->Parent && obj->Parent->SETData)
		{
			obj->Parent->SETData->Flags |= 0x4;
		}
	}

	ItemBoxCollision_hook.Original(obj);
}

void ItemBoxAirExec_r(ObjectMaster* obj)
{
	auto data = obj->Data1.Entity;

	if (data->Action == 2)
	{
		int pnum;
		if (CheckCollisionWithChao(obj->Data1.Entity, &pnum))
		{
			DisplayItemBoxItem(pnum, ItemBox_Items[data->Index].Texture);
			ItemBox_Items[data->Index].Code(obj, pnum);
			data->Status |= 0x4;
			data->Action = 3;

			NJS_POINT3 spd = { 0.0f, 0.1f, 0.0f };
			CreateItemBoxSmoke(&data->Position, &spd, 6.0f);

			PlaySoundProbably(32781, 0, 0, 0);

			if (obj->SETData)
			{
				obj->SETData->Flags |= 0x4;
			}

			if (data->NextAction && obj->Parent && obj->Parent->SETData)
			{
				obj->Parent->SETData->Flags |= 0x4;
			}
		}
	}
	
	ItemBoxAirExec_hook.Original(obj);
}

BOOL __cdecl AL_GetRandomAttrPos_r(void* buf, NJS_POINT3* pos, int num)
{
	if (CurrentLevel == LevelIDs_ChaoWorld || CurrentChaoData == nullptr)
	{
		return AL_GetRandomAttrPos_hook.Original(buf, pos, num);
	}
	else
	{
		*pos = MainCharObj1[CurrentChaoData->entity.Index]->Position;

		pos->x += njCos(rand() % 0x10000) * (njRandom() * 100.0f);
		pos->z += njSin(rand() % 0x10000) * (njRandom() * 100.0f);

		return TRUE;
	}
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
		ItemBoxCollision_hook.Hook(ItemBoxCollision_r);
		ItemBoxAirExec_hook.Hook(ItemBoxAirExec_r);
		AL_GetRandomAttrPos_hook.Hook(AL_GetRandomAttrPos_r);

		Chao_Init();
		PatchWaterDetection();

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
