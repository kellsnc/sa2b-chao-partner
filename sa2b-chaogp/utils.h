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

void LookAt(NJS_VECTOR* from, NJS_VECTOR* to, Angle* outx, Angle* outy);
float GetDistance(NJS_VECTOR* orig, NJS_VECTOR* dest);
NJS_VECTOR GetPointToFollow(NJS_VECTOR* pos, NJS_VECTOR* dir, Rotation* rot);
void MoveForward(EntityData1* entity, float speed);
void PutBehindPlayer(NJS_VECTOR* pos, EntityData1* data, float dist);

EntityData1* GetClosestAttack(NJS_VECTOR* pos, Float range, int playerid);
EntityData1* GetClosestRing(NJS_VECTOR* pos, Float range, int playerid);

static const void* const sub_53CAC0Ptr = (void*)0x53CAC0;
static inline void sub_53CAC0(ObjectMaster* a1)
{
	__asm
	{
		mov eax, [a1]
		call sub_53CAC0Ptr
	}
}