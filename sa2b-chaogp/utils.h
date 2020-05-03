#pragma once

#define DynamicVoidFunc(NAME, ADDRESS) NonStaticFunctionPointer(void,NAME,(void),ADDRESS)

FunctionPointer(void, FreeTexList, (NJS_TEXLIST* texlist), 0x77F9F0);
VoidFunc(LoadChaoPalette, 0x534350);

Rotation fPositionToRotation(NJS_VECTOR* orig, NJS_VECTOR* point);
NJS_VECTOR TransformSpline(NJS_VECTOR* orig, NJS_VECTOR* dest, float state);
float GetDistance(NJS_VECTOR* orig, NJS_VECTOR* dest);

float* njPushUnitMatrix();
void njTranslateV(float* matrix, NJS_VECTOR* pos);
NJS_VECTOR njCalcPoint(float* matrix, float x, float y, float z);