#include "stdafx.h"

Trampoline* LoadLevelInit_t = nullptr;
Trampoline* LoadLevelManager_t = nullptr;
Trampoline* LoadLevelDestroy_t = nullptr;

void __cdecl ChaoHandler(ObjectMaster* obj) {
	if (ChaoSlots && ++obj->Data1.Entity->Scale.x == 40) {
		ChaoData* chao = &ChaoSlots[0];
		NJS_VECTOR pos = { 0, 0, 0 };
		CreateChao(chao, 0, 0, &pos, 0);
	}
}

// Load chao data at the level initialization
void LoadLevelInit_Chao() {
	DynamicVoidFunc(original, LoadLevelInit_t->Target());
	original();

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

// Load the chao itself at each load/restart
void LoadLevelManager_Chao() {
	DynamicVoidFunc(original, LoadLevelManager_t->Target());
	original();

	LoadObject(2, "ChaoInLevelHandler", ChaoHandler, LoadObj_Data1);
	LoadChaoSaveSystem();
}

// Delete the chao files when the level is finished/exited
void* LoadLevelDestroy_Chao() {
	FreeTexList((NJS_TEXLIST*)0x1366AE4);
	FreeTexList((NJS_TEXLIST*)0x13669FC);
	FreeTexList((NJS_TEXLIST*)0x1366AD4);
	FreeTexList((NJS_TEXLIST*)0x1366ACC);
	FreeTexList((NJS_TEXLIST*)0x1366AC4);
	FreeTexList((NJS_TEXLIST*)0x1366ADC);
	FreeTexList((NJS_TEXLIST*)0x1366AB4);
	FreeTexList((NJS_TEXLIST*)0x1366ABC);

	NonStaticFunctionPointer(void*, original, (), LoadLevelDestroy_t->Target());
	return original();
}

void Chao_Init() {
	LoadLevelInit_t = new Trampoline(0x43CB10, 0x43CB16, LoadLevelInit_Chao);
	LoadLevelManager_t = new Trampoline(0x43CB50, 0x43CB56, LoadLevelManager_Chao);
	LoadLevelDestroy_t = new Trampoline(0x454CC0, 0x454CC8, LoadLevelDestroy_Chao);
}