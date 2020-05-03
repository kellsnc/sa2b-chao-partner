#include "stdafx.h"

static Trampoline* LoadLevelInit_t = nullptr;
static Trampoline* LoadLevelManager_t = nullptr;
static Trampoline* LoadLevelDestroy_t = nullptr;
static Trampoline* ChangeChaoStage_t = nullptr;

static ChaoData* CarriedChao = nullptr;

enum class ChaoHandlerActions {
	LoadChao,
	Free,
	Attached
};

struct ChaoHandlerData1 {
	ChaoHandlerActions Action;
	EntityData1* chaodata;
	Uint8 player;
	NJS_VECTOR dest;
	float dist;
	bool attached;
};

NJS_VECTOR GetChaoPointToFollow(NJS_VECTOR* pos, float rotx, float roty, float rotz) {
	float* matrix = njPushUnitMatrix();
	njTranslateV(matrix, pos);
	njRotateZ(matrix, rotx);
	njRotateY(matrix, roty);
	njRotateX(matrix, rotz);
	NJS_VECTOR point = njCalcPoint(matrix, 5, 10, -2);
	njPopMatrix(1u);

	return { point.x + pos->x, point.y + pos->y, point.z + pos->z };
}

NJS_VECTOR GetChaoPointPlayerPtr(EntityData1* playerdata) {
	return GetChaoPointToFollow(&playerdata->Position, playerdata->Rotation.x, -playerdata->Rotation.y + 0x4000, playerdata->Rotation.z);
}

void __cdecl ChaoHandler(ObjectMaster* obj) {
	ChaoHandlerData1* data = (ChaoHandlerData1*)obj->Data1.Undefined;
	EntityData1* playerdata = MainCharObj1[data->player];

	switch (data->Action) {
	case ChaoHandlerActions::LoadChao:
		data->chaodata = &CreateChao(CarriedChao, 0, 0, &GetChaoPointPlayerPtr(playerdata), 0)->Data1.Chao->entity;
		data->Action = ChaoHandlerActions::Attached;
		break;
	case ChaoHandlerActions::Free:
		if (Controllers[data->player].press & Buttons_L) {
			data->Action = ChaoHandlerActions::Attached;
		}

		break;
	case ChaoHandlerActions::Attached:
		if (Controllers[data->player].press & Buttons_L) {
			data->Action = ChaoHandlerActions::Free;
		}

		data->dest = GetChaoPointPlayerPtr(playerdata);
		data->dist = GetDistance(&data->dest, &data->chaodata->Position);
		data->chaodata->Action = 1;

		if (data->dist < 3) {
			data->chaodata->Rotation.y = -playerdata->Rotation.y + 0x4000;
			data->chaodata->Position = data->dest;
		}
		else {
			data->chaodata->Position = TransformSpline(&data->chaodata->Position, &data->dest, data->dist / (100 + (400 - data->dist)));
			data->chaodata->Rotation.y = -fPositionToRotation(&data->chaodata->Position, &data->dest).y - 0x4000;
		}

		break;
	}
}

// Load chao data at the level initialization
void LoadLevelInit_Chao() {
	DynamicVoidFunc(original, LoadLevelInit_t->Target());
	original();

	// The carried chao is removed if going back to the chao world
	// If it's a level, we load chao data

	if (CarriedChao) {
		if (CurrentLevel == LevelIDs_ChaoWorld) {
			delete CarriedChao;
			CarriedChao = nullptr;
		}
		else {
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
void LoadLevelManager_Chao() {
	DynamicVoidFunc(original, LoadLevelManager_t->Target());
	original();

	if (CurrentLevel != LevelIDs_ChaoWorld && CarriedChao) {
		LoadObject(2, "ChaoInLevelHandler", ChaoHandler, LoadObj_Data1);
	}
}

// Delete the chao files when the level is finished/exited
void* LoadLevelDestroy_Chao() {
	if (CarriedChao) {
		FreeTexList((NJS_TEXLIST*)0x1366AE4);
		FreeTexList((NJS_TEXLIST*)0x13669FC);
		FreeTexList((NJS_TEXLIST*)0x1366AD4);
		FreeTexList((NJS_TEXLIST*)0x1366ACC);
		FreeTexList((NJS_TEXLIST*)0x1366AC4);
		FreeTexList((NJS_TEXLIST*)0x1366ADC);
		FreeTexList((NJS_TEXLIST*)0x1366AB4);
		FreeTexList((NJS_TEXLIST*)0x1366ABC);
	}

	NonStaticFunctionPointer(void*, original, (), LoadLevelDestroy_t->Target());
	return original();
}

// Select the chao when leaving a garden
BYTE* __cdecl ChangeChaoStage_Chao(int area) {
	if (area == 7) {
		if (!CarriedChao) {
			if (MainCharObj2[0]->HeldObject) {
				ChaoData1* data = MainCharObj2[0]->HeldObject->Data1.Chao;

				if (data && data->ChaoDataBase_ptr) {

					// Loop through the chao slots to get if it's a valid chao
					for (uint8_t i = 0; i < 24; ++i) {
						if (&ChaoSlots[i].data == data->ChaoDataBase_ptr) {

							if (data->ChaoDataBase_ptr->Type != ChaoType_Empty && data->ChaoDataBase_ptr->Type != ChaoType_Egg) {
								CarriedChao = new ChaoData;
								memcpy(CarriedChao, &ChaoSlots[i], sizeof(ChaoData));
							}
						}
					}
				}
			}
		}
		else {
			delete CarriedChao;
			CarriedChao = nullptr;
		}
	}

	NonStaticFunctionPointer(BYTE*, original, (int area), ChangeChaoStage_t->Target());
	return original(area);
}

extern "C"
{
	__declspec(dllexport) void Init(const char* path, const HelperFunctions& helperFunctions) {
		LoadLevelInit_t = new Trampoline(0x43CB10, 0x43CB16, LoadLevelInit_Chao);
		LoadLevelManager_t = new Trampoline(0x43CB50, 0x43CB56, LoadLevelManager_Chao);
		LoadLevelDestroy_t = new Trampoline(0x454CC0, 0x454CC8, LoadLevelDestroy_Chao);
		ChangeChaoStage_t = new Trampoline(0x52B5B0, 0x52B5B6, ChangeChaoStage_Chao);

		#ifndef NDEBUG
		if (!CarriedChao) {
			CarriedChao = new ChaoData();
		}
		#endif
	}

	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };
}