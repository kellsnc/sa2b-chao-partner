#pragma once

#define TARGET_DYNAMIC(name) ((decltype(name##_r)*)name##_t->Target())
#define TARGET_STATIC(name) ((decltype(name##_r)*)name##_t.Target())

enum CustomChaoActs {
	ChaoAct_FollowPlayer,
	ChaoAct_IdlePlayer,
	ChaoAct_Attack
};

enum ChaoLeashModes {
	ChaoLeashMode_Disabled,
	ChaoLeashMode_Free,
	ChaoLeashMode_Fly
};

enum StatusChao : __int16 {
	StatusChao_Held = 0x1000,

	// Custom ones:
	StatusChao_FlyPlayer = 0x2000,
	StatusChao_Attacked = 0x4000
};

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

struct CustomData {
	NJS_POINT3 pre;
	EntityData1* target;
	uint16_t noAutoAttackTimer;
	uint16_t noAttackTimer;
};

struct ChaoLeash {
	ChaoLeashModes mode;
	ChaoData* data;
	CustomData custom;
};

struct TargetEntityStruct {
	EntityData1* entity;
	float distance;
};

extern bool ChaoAssist;
extern bool ChaoLuck;
extern ChaoLeash CarriedChao[8];

DataPointer(ObjectMaster*, ALWControlTask, 0x1A0F94C);
VoidFunc(LoadALWControl, 0x530B80);
VoidFunc(InitWaypoints, 0x534EF0);
VoidFunc(LoadChaoPalette, 0x534350);

DataPointer(uint16_t, TargetEntitiesP1_Count, 0x1DE46B8);
DataPointer(uint16_t, TargetEntitiesP2_Count, 0x1DE50E0);
DataPointer(uint16_t, TargetRingEntitiesP1_Count, 0x1DE8C28);
DataPointer(uint16_t, TargetRingEntitiesP2_Count, 0x1DE9448);

DataArray(TargetEntityStruct, TargetEntitiesP1, 0x1DE6FA0, 257);
DataArray(TargetEntityStruct, TargetEntitiesP2, 0x1DE5100, 257);
DataArray(TargetEntityStruct, TargetRingEntitiesP1, 0x1DE8C40, 913);
DataArray(TargetEntityStruct, TargetRingEntitiesP2, 0x1DE46C0, 913);

FunctionPointer(void, CreateItemBoxSmoke, (NJS_POINT3* pos, NJS_POINT3* spd, Float scl), 0x6EBE50);
FunctionPointer(void, GetActiveCollisions, (float x, float y, float z, float s), 0x47CD60);
DataPointer(uint16_t, ActiveColCount, 0x1DE9484);

//void __usercall Chao_RunMovements(ObjectMaster* obj@<eax>)
static const void* const Chao_RunMovementsPtr = (void*)0x0053DC40;
static inline void Chao_RunMovements(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call Chao_RunMovementsPtr
	}
}

//void __usercall Chao_PlayAnimation(MotionTableData* mtable@<esi>)
static const void* const Chao_PlayAnimationPtr = (void*)0x007938D0;
static inline void Chao_PlayAnimation_(MotionTableData* mtable)
{
	__asm
	{
		mov esi, mtable
		call Chao_PlayAnimationPtr
	}
}

static void Chao_PlayAnimation(ObjectMaster* obj) {
	if (obj->Data1.Chao->Flags & 4) {
		Chao_PlayAnimation_(&obj->Data1.Chao->MotionTable);
	}
}

//void __usercall Chao_RunEmotionBall(ObjectMaster *obj@<eax>)
static const void* const Chao_RunEmotionBallPtr = (void*)0x0053CB70;
static inline void Chao_RunEmotionBall(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call Chao_RunEmotionBallPtr
	}
}

//void __usercall Chao_RunActions(ObjectMaster *obj@<eax>)
static const void* const Chao_RunActionsPtr = (void*)0x0053A3E0;
static inline void Chao_RunActions(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call Chao_RunActionsPtr
	}
}

//void __usercall Chao_MoveEmotionBall(ObjectMaster* a1@<ebx>)
static const void* const Chao_MoveEmotionBallPtr = (void*)0x0056CCF0;
static inline void Chao_MoveEmotionBall(ObjectMaster* obj)
{
	__asm
	{
		mov ebx, obj
		call Chao_MoveEmotionBallPtr
	}
}

//void __usercall Chao_RunGravity(ObjectMaster *obj@<eax>)
static const void* const Chao_RunGravityPtr = (void*)0x00796780;
static inline void Chao_RunGravity(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call Chao_RunGravityPtr
	}
}

//void __usercall Chao_RunPhysics(ObjectMaster *obj@<eax>)
static const void* const Chao_RunPhysicsPtr = (void*)0x0053DF00;
static inline void Chao_RunPhysics(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call Chao_RunPhysicsPtr
	}
}

//void __usercall RunChaoBehaviour@<eax>(ObjectMaster* obj@<eax>, void* func, int idk)
static const void* const RunChaoBehaviourPtr = (void*)0x53D890;
static inline void RunChaoBehaviour(ObjectMaster* obj, void* func, int idk)
{
	__asm
	{
		push[idk]
		push[func]
		mov eax, [obj]
		call RunChaoBehaviourPtr
		add esp, 8
	}
}

//char __usercall SE_CallV2@<al>(int id@<edi>, NJS_VECTOR* position@<esi>, ObjectMaster* entity, char bank, char volume)
static const void* const SE_CallV2Ptr = (void*)0x4372E0;
static inline void SE_CallV2(int id, NJS_VECTOR* position, ObjectMaster* entity, char bank, char volume)
{
	__asm
	{
		movzx eax, [volume]
		push eax
		movzx eax, [bank]
		push eax
		push[entity]
		mov esi, [position]
		mov edi, [id]
		call SE_CallV2Ptr
		add esp, 12
	}
}

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
