#pragma once

#define TARGET_DYNAMIC(name) ((decltype(name##_r)*)name##_t->Target())
#define TARGET_STATIC(name) ((decltype(name##_r)*)name##_t.Target())

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

struct CustomData {
	NJS_VECTOR targetPos;
	uint16_t noAutoAttackTimer;
	uint16_t noAttackTimer;
};

struct ChaoLeash {
	ChaoLeashModes mode;
	ChaoData* data;
	CustomData custom;
};

extern bool ChaoAssist;
extern bool ChaoLuck;

extern ChaoLeash CarriedChao[2];

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