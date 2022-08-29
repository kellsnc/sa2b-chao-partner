#pragma once

enum ChaoStats {
	ChaoStat_Swim,
	ChaoStat_Fly,
	ChaoStat_Run,
	ChaoStat_Power,
	ChaoStat_Stamina,
	ChaoStat_Luck,
	ChaoStat_Intelligence,
	ChaoStat_Unknown
};

struct TargetEntityStruct {
	EntityData1* entity;
	float distance;
};

VoidFunc(LoadChaoPalette, 0x534350);

DataPointer(uint16_t, TargetEntitiesP1_Count, 0x1DE46B8);
DataPointer(uint16_t, TargetEntitiesP2_Count, 0x1DE50E0);
DataPointer(uint16_t, TargetRingEntitiesP1_Count, 0x1DE8C28);
DataPointer(uint16_t, TargetRingEntitiesP2_Count, 0x1DE9448);

DataArray(TargetEntityStruct, TargetEntitiesP1, 0x1DE6FA0, 257);
DataArray(TargetEntityStruct, TargetEntitiesP2, 0x1DE5100, 257);
DataArray(TargetEntityStruct, TargetRingEntitiesP1, 0x1DE8C40, 913);
DataArray(TargetEntityStruct, TargetRingEntitiesP2, 0x1DE46C0, 913);

NJS_POINT3 LerpPosition(NJS_POINT3* from, NJS_POINT3* to, Float spd);
Angle GetYawAngleToPoint(NJS_POINT3* from, NJS_POINT3* to);
float GetDistance(NJS_POINT3* orig, NJS_POINT3* dest);
NJS_POINT3 GetPointToFollow(NJS_POINT3* pos, NJS_POINT3* dir, Rotation* rot);
void PutBehindPlayer(NJS_POINT3* pos, EntityData1* data, Float dist);

EntityData1* GetClosestAttack(NJS_POINT3* pos, Float range, int playerid);
EntityData1* GetClosestRing(NJS_POINT3* pos, Float range, int playerid);

static const void* const sub_53CAC0Ptr = (void*)0x53CAC0;
static inline void sub_53CAC0(ObjectMaster* a1)
{
	__asm
	{
		mov eax, [a1]
		call sub_53CAC0Ptr
	}
}

static const void* const AdjustAnglePtr = (void*)0x446960;
static inline Angle AdjustAngle(Angle ang0, Angle ang1, Angle dang)
{
	Angle result;
	__asm
	{
		mov edx, [dang]
		mov eax, [ang1]
		mov ecx, [ang0]
		call AdjustAnglePtr
		mov result, eax
	}
	return result;
}

static const void* const DiffAnglePtr = (void*)0x4469C0;
static inline Angle DiffAngle(Angle ang0, Angle ang1)
{
	Angle result;
	__asm
	{
		push[ang0]
		mov eax, [ang1]
		call AdjustAnglePtr
		mov result, eax
		add esp, 4
	}
	return result;
}
