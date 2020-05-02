#include "stdafx.h"

Trampoline* LoadLevelInit_t = nullptr;
Trampoline* LoadLevelManager_t = nullptr;
Trampoline* LoadLevelDestroy_t = nullptr;

// Load chao data at the level initialization
void LoadLevelInit_Chao() {


	DynamicVoidFunc(original, LoadLevelInit_t->Target());
	original();
}

// Load the chao itself at each load/restart
void LoadLevelManager_Chao() {


	DynamicVoidFunc(original, LoadLevelManager_t->Target());
	original();
}

// Delete the chao files when the level is finished/exited
void* LoadLevelDestroy_Chao() {


	NonStaticFunctionPointer(void*, original, (), LoadLevelDestroy_t->Target());
	return original();
}

void Chao_Init() {
	LoadLevelInit_t = new Trampoline(0x43CB10, 0x43CB16, LoadLevelInit_Chao);
	LoadLevelManager_t = new Trampoline(0x43CB50, 0x43CB56, LoadLevelManager_Chao);
	LoadLevelDestroy_t = new Trampoline(0x454CC0, 0x454CC8, LoadLevelDestroy_Chao);
}