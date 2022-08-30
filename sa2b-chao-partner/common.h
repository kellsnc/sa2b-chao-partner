#pragma once

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

enum : __int32
{
	EM_MD_PLEASURE = 0,
	EM_MD_ANGER,
	EM_MD_SORROW,
	EM_MD_FEAR,
	EM_MD_SURPRISE,
	EM_MD_PAIN,
	EM_MD_RELAX,
	EM_MD_TOTAL,
};

enum : __int32
{
	EM_ST_SLEEPY,
	EM_ST_SLEEP_DEPTH,
	EM_ST_HUNGER,
	EM_ST_BREED,
	EM_ST_TEDIOUS,
	EM_ST_LONELY,
	EM_ST_TIRE,
	EM_ST_STRESS,
	EM_ST_NOURISHMENT,
	EM_ST_CONDITION,
	EM_ST_THIRSTY
};

enum : __int32
{
	EM_PER_CURIOSITY,
	EM_PER_KINDNESS,
	EM_PER_AGRESSIVE,
	EM_PER_SLEEPY_HEAD,
	EM_PER_SOLITUDE,
	EM_PER_VITALITY,
	EM_PER_GLUTTON,
	EM_PER_REGAIN,
	EM_PER_SKILLFUL,
	EM_PER_CHARM,
	EM_PER_CHATTY,
	EM_PER_CALM,
	EM_PER_FICKLE
};

// This one is not from symbols, can't find any enum
enum : __int32
{
	EM_ILL_COUGH,
	EM_ILL_COLD,
	EM_ILL_RASH,
	EM_ILL_NOSE,
	EM_ILL_HICCUPS,
	EM_ILL_STOMACH
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

struct XYZ_SHADOW_WORK
{
	NJS_POINT3 pos;
	xssunit hit[6];
	xssunit pre_hit[6];
};

struct MOVE_WORK
{
	NJS_POINT3 Velo;
	NJS_POINT3 Acc;
	Angle3 AimAng;
	Angle3 RotSpd;
	float rad;
	float height;
	float weight;
	char gap[4];
	unsigned __int16 Flag;
	unsigned __int16 Timer;
	float Spd;
	float Gravity;
	int ViewAngle;
	float ViewRange;
	NJS_POINT3 AimPos;
	NJS_POINT3 PrePos;
	NJS_POINT3 HomePos;
	Angle3 HomeAng;
	Angle3 Phase;
	NJS_LINE FrontWall;
	NJS_POINT3 Offset;
	float Top;
	float Side;
	float Bottom;
	float CliffHeight;
	float BoundSide;
	float BoundFloor;
	float BoundCeiling;
	float BoundFriction;
	float TopY;
	float BottomY;
	float WaterY;
	XYZ_SHADOW_WORK Shadow;
};

extern bool ChaoAssist;
extern bool ChaoLuck;
extern ChaoLeash CarriedChao[8];

DataPointer(ObjectMaster*, WorldMasterTask, 0x1A0F94C);
VoidFunc(ALW_Create, 0x530B80);
VoidFunc(AL_InitLandMark, 0x534EF0);
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

FunctionPointer(int, ALBHV_Swim, (ObjectMaster* obj), 0x562330);

static const void* const AL_BehaviorControlPtr = (void*)0x0053DC40;
static inline void AL_BehaviorControl(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call AL_BehaviorControlPtr
	}
}

static const void* const MotionControlPtr = (void*)0x007938D0;
static inline void MotionControl(MotionTableData* mtable)
{
	__asm
	{
		mov esi, mtable
		call MotionControlPtr
	}
}

static void AL_MotionControl(ObjectMaster* obj)
{
	if (((CHAOWK*)obj->Data1.Chao)->ChaoFlag & 4)
	{
		MotionControl(&obj->Data1.Chao->MotionTable);
	}
}

static const void* const AL_IconControlPtr = (void*)0x0053CB70;
static inline void AL_IconControl(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call AL_IconControlPtr
	}
}

static const void* const AL_FaceControlPtr = (void*)0x0053A3E0;
static inline void AL_FaceControl(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call AL_FaceControlPtr
	}
}

static const void* const AL_ShapeControlPtr = (void*)0x0056CCF0;
static inline void AL_ShapeControl(ObjectMaster* obj)
{
	__asm
	{
		mov ebx, obj
		call AL_ShapeControlPtr
	}
}

static const void* const MOV_ControlPtr = (void*)0x00796780;
static inline void MOV_Control(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call MOV_ControlPtr
	}
}

static void AL_MOV_Control(ObjectMaster* obj)
{
	if (((CHAOWK*)obj->Data1.Chao)->ChaoFlag & 2)
	{
		MOV_Control(obj);
	}
}

static const void* const AL_DetectCollisionPtr = (void*)0x0053DF00;
static inline void AL_DetectCollision(ObjectMaster* obj)
{
	__asm
	{
		mov eax, obj
		call AL_DetectCollisionPtr
	}
}

static const void* const AL_SetBehaviorWithTimerPtr = (void*)0x53D890;
static inline void AL_SetBehaviorWithTimer(ObjectMaster* obj, int(__cdecl* Func)(ObjectMaster*), int ReserveTimer)
{
	__asm
	{
		push[ReserveTimer]
		push[Func]
		mov eax, [obj]
		call AL_SetBehaviorWithTimerPtr
		add esp, 8
	}
}

static const void* const dsPlay_iloopPtr = (void*)0x4372E0;
static inline void dsPlay_iloop(int id, NJS_VECTOR* position, ObjectMaster* entity, char bank, char volume)
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
		call dsPlay_iloopPtr
		add esp, 12
	}
}

static const void* const AL_IconResetPosPtr = (void*)0x53CAC0;
static inline void AL_IconResetPos(ObjectMaster* obj)
{
	__asm
	{
		mov eax, [obj]
		call AL_IconResetPosPtr
	}
}

static const void* const SetMotionLinkPtr = (void*)0x00793C40;
static inline void SetMotionLink(MOTION_CTRL* Control, int Mtnnum)
{
	__asm
	{
		mov eax, Control
		mov edx, Mtnnum
		call SetMotionLinkPtr
	}
}

static void AL_SetMotionLink(ObjectMaster* obj, int Mtnnum)
{
	SetMotionLink(&((CHAOWK*)obj->Data1.Chao)->MotionCtrl, Mtnnum);
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
