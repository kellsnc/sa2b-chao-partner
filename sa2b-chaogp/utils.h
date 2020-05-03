#pragma once

#define DynamicVoidFunc(NAME, ADDRESS) NonStaticFunctionPointer(void,NAME,(void),ADDRESS)

FunctionPointer(void, FreeTexList, (NJS_TEXLIST* texlist), 0x77F9F0);
VoidFunc(LoadChaoPalette, 0x534350);