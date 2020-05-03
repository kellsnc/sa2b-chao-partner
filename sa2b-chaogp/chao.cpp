#include "stdafx.h"

Trampoline* LoadLevelInit_t		= nullptr;
Trampoline* LoadLevelManager_t	= nullptr;
Trampoline* LoadLevelDestroy_t	= nullptr;
Trampoline* ChangeChaoStage_t	= nullptr;

ChaoData* CarriedChao = nullptr;

void __cdecl ChaoHandler(ObjectMaster* obj) {
	EntityData1* data = obj->Data1.Entity;
	
	switch (data->Action) {
	case 0:
		CreateChao(CarriedChao, 0, 0, &data->Position, 0);
		data->Action = 1;
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

void Chao_Init() {
	LoadLevelInit_t = new Trampoline(0x43CB10, 0x43CB16, LoadLevelInit_Chao);
	LoadLevelManager_t = new Trampoline(0x43CB50, 0x43CB56, LoadLevelManager_Chao);
	LoadLevelDestroy_t = new Trampoline(0x454CC0, 0x454CC8, LoadLevelDestroy_Chao);
	ChangeChaoStage_t = new Trampoline(0x52B5B0, 0x52B5B6, ChangeChaoStage_Chao);
}