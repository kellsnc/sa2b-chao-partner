#include "stdafx.h"

extern "C"
{
	__declspec(dllexport) void Init(const char* path, const HelperFunctions& helperFunctions) {
		Chao_Init();
	}

	__declspec(dllexport) void OnFrame() {
		
	}

	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };
}