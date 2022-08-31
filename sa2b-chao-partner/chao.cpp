#include "stdafx.h"
#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "common.h"
#include "utils.h"
#include "chao.h"

FunctionHook<void, ObjectMaster*> Chao_Main_hook(0x54FE20);

Buttons ChaoAttackButton = Buttons_Z;

CHAOWK* CurrentChaoData = nullptr;

static void Chao_MoveToTarget(CHAOWK* chaowp, NJS_POINT3* targetPos, Float speed)
{
	chaowp->entity.Position = LerpPosition(&chaowp->entity.Position, targetPos, speed);
	chaowp->entity.Rotation.y = GetYawAngleToPoint(&chaowp->entity.Position, targetPos);
}

static float Chao_GetFlightSpeed(CHAOWK* chaowp)
{
	return 0.1f + (min((float)(chaowp->pParamGC->Lev[ChaoStat_Fly]), 100.0f) / 500.0f);
}

static float Chao_GetAttackRange(CHAOWK* chaowp)
{
	return max(0, max(200.0f, min(500.0f, (float)chaowp->pParamGC->Lev[ChaoStat_Stamina] * 7.0f)) - (float)(chaowp->pParamGC->emotion.State[EM_ST_SLEEP_DEPTH] / 2));
}

static bool Chao_AttackCondition(CHAOWK* chaowp, CustomData* custom, int playerid)
{
	auto param = chaowp->pParamGC;

	if (custom->noAutoAttackTimer == 0 && rand() % 200 < max(20, param->emotion.Mood[EM_MD_PLEASURE]))
	{
		custom->noAutoAttackTimer = 60 + std::abs(min(0, param->emotion.IllState[EM_ILL_COLD])) + param->emotion.State[EM_ST_HUNGER] / 50;
		return true;
	}

	if (custom->noAttackTimer == 0 && Controllers[playerid].press & ChaoAttackButton)
	{
		if (param->emotion.State[EM_ST_SLEEP_DEPTH] > 200 && rand() % 3 == 0)
		{
			return false;
		}

		custom->noAutoAttackTimer = 120 + std::abs(min(0, param->emotion.IllState[EM_ILL_COLD])) + param->emotion.State[EM_ST_HUNGER] / 50;

		return true;
	}

	return false;
}

static void Chao_CheckAttack(CHAOWK* chaowp, ChaoLeash* leash, EntityData1* player, CharObj2Base* co2)
{
	if (ChaoAssist)
	{
		auto target = GetClosestAttack(&chaowp->entity.Position, Chao_GetAttackRange(chaowp), co2->PlayerNum);

		if (target && Chao_AttackCondition(chaowp, &leash->custom, co2->PlayerNum))
		{
			chaowp->entity.NextAction = ChaoAct_Attack;
			leash->custom.target = target;
		}
	}
}

static NJS_POINT3 GetFollowPoint(EntityData1* player, CharObj2Base* co2)
{
	NJS_POINT3 dir = { -6, 7, 5 };

	if (co2->CharID == Characters_Eggman)
	{
		dir = { -10, 14, 7 };
	}
	else if (co2->CharID == Characters_MechEggman || co2->CharID == Characters_MechTails)
	{
		dir = { -8, 16, 11 };
	}
	else if (co2->CharID == Characters_Knuckles || co2->CharID == Characters_Rouge)
	{
		if (player->Action == Action_Climb || player->Action == Action_DigOnWall ||
			player->Action == Action_DigFailOnWall || player->Action == Action_DigFinishOnWall)
		{
			dir = { 2.0f, 11.0f, 4.0f };
		}
	}

	return GetPointToFollow(&player->Position, &dir, &player->Rotation);
}

static void ChaoAttack(CHAOWK* chaowp, EntityData1* player, CharObj2Base* co2, CustomData* custom)
{
	auto target = GetClosestAttack(&chaowp->entity.Position, Chao_GetAttackRange(chaowp), co2->PlayerNum);
	auto dist = GetDistance(&chaowp->entity.Position, &player->Position);

	if (target == custom->target && dist < Chao_GetAttackRange(chaowp))
	{
		auto target_pos = GetEntityCenter(target);
		Chao_MoveToTarget(chaowp, &target_pos, Chao_GetFlightSpeed(chaowp));
	}
	else
	{
		chaowp->entity.NextAction = ChaoAct_FollowPlayer;
	}
}

static void FollowPlayer(CHAOWK* chaowp, EntityData1* player, CharObj2Base* co2)
{
	NJS_POINT3 dest = GetFollowPoint(player, co2);

	Chao_MoveToTarget(chaowp, &dest, Chao_GetFlightSpeed(chaowp));

	if (njScalor(&co2->Speed) == 0 && GetDistance(&chaowp->entity.Position, &dest) < 5.0f)
	{
		chaowp->entity.NextAction = ChaoAct_IdlePlayer;
	}
}

static void IdlePlayer(CHAOWK* chaowp, EntityData1* player, CharObj2Base* co2, CustomData* custom)
{
	NJS_POINT3 dest = GetFollowPoint(player, co2);

	if (GetDistance(&custom->pre, &dest) > 3.0f)
	{
		chaowp->entity.NextAction = ChaoAct_FollowPlayer;
	}

	chaowp->entity.Rotation.y += 0x100;

	dest.x = custom->pre.x - njCos(-chaowp->entity.Rotation.y) * 1.0f;
	dest.y = custom->pre.y;
	dest.z = custom->pre.z - njSin(-chaowp->entity.Rotation.y) * 1.0f;

	chaowp->entity.Position = LerpPosition(&chaowp->entity.Position, &dest, 0.1f);
}

bool CheckFlyButton(int playerid)
{
	return Controllers[playerid].press & Buttons_L && Controllers[playerid].press & Buttons_R;
}

static void LevelChao_Fly(ObjectMaster* obj, CHAOWK* chaowp, ChaoLeash* leash, EntityData1* player, CharObj2Base* co2)
{
	switch (chaowp->entity.NextAction) {
	case ChaoAct_FollowPlayer:
		FollowPlayer(chaowp, player, co2);
		Chao_CheckAttack(chaowp, leash, player, co2);
		leash->custom.pre = chaowp->entity.Position;
		break;
	case ChaoAct_IdlePlayer:
		IdlePlayer(chaowp, player, co2, &leash->custom);
		Chao_CheckAttack(chaowp, leash, player, co2);

		break;
	case ChaoAct_Attack:
		ChaoAttack(chaowp, player, co2, &leash->custom);
		leash->custom.pre = chaowp->entity.Position;
		break;
	}

	// Flying animation
	if (FrameCountIngame % 30 == 0)
	{
		AL_SetMotionLink(obj, 286); // or 130
	}

	AL_MotionControl(obj);
	AL_IconControl(obj);
	AL_FaceControl(obj);
	AL_ShapeControl(obj);
	AL_DetectCollision(obj);

	if (CheckFlyButton(chaowp->entity.Index))
	{
		chaowp->entity.Status &= ~StatusChao_FlyPlayer;
		leash->mode = ChaoLeashMode_Free;
	}
}

static void LevelChao_Normal(ObjectMaster* obj, CHAOWK* chaowp, ChaoLeash* leash)
{
	AL_BehaviorControl(obj);
	AL_MotionControl(obj);
	AL_IconControl(obj);
	AL_FaceControl(obj);
	AL_ShapeControl(obj);

	// Fly if Y is pressed
	if (!(chaowp->entity.Status & StatusChao_Held))
	{
		AL_MOV_Control(obj);
		leash->mode = ChaoLeashMode_Free;

		if (CheckFlyButton(chaowp->entity.Index))
		{
			chaowp->entity.Status |= StatusChao_FlyPlayer;
			leash->mode = ChaoLeashMode_Fly;
		}
	}
	
	AL_DetectCollision(obj);

	leash->custom.pre = chaowp->entity.Position;
}

static void LevelChao_UpdateStuff(CustomData* custom)
{
	if (custom->noAttackTimer > 0) custom->noAttackTimer -= 1;
	if (custom->noAutoAttackTimer > 0) custom->noAutoAttackTimer -= 1;
}

void __cdecl Chao_Main_r(ObjectMaster* obj)
{
	CHAOWK* chaowp = (CHAOWK*)obj->Data1.Chao;
	CurrentChaoData = chaowp;

	if (CurrentLevel != LevelIDs::LevelIDs_ChaoWorld)
	{
		SoundsPaused = TRUE;

		if (chaowp->Timer <= 2u)
		{
			AL_IconResetPos(obj);
		}

		EntityData1* player = MainCharObj1[chaowp->entity.Index];
		EntityData2* pmotion = (EntityData2*)MainCharacter[chaowp->entity.Index]->EntityData2;
		CharObj2Base* co2 = MainCharObj2[chaowp->entity.Index];
		ChaoLeash* leash = &CarriedChao[chaowp->entity.Index];

		// If the player cannot be found, act as a normal Chao
		if (player == nullptr)
		{
			Chao_Main_hook.Original(obj);
			return;
		}

		LevelChao_UpdateStuff(&leash->custom);

		// Run custom actions
		if (!(chaowp->entity.Status & StatusChao_FlyPlayer))
		{
			chaowp->entity.Status |= 0x40;
			LevelChao_Normal(obj, chaowp, leash);
		}
		else
		{
			chaowp->entity.Status &= ~0x40;
			LevelChao_Fly(obj, chaowp, leash, player, co2);
		}

		++chaowp->Timer;

		if ((chaowp->ChaoFlag & 8) != 0)
		{
			AddToCollisionList(obj);
		}
		else
		{
			Collision_InitThings(obj);
		}

		SoundsPaused = FALSE;
	}
	else
	{
		Chao_Main_hook.Original(obj);
	}
}

void Chao_Init()
{
	Chao_Main_hook.Hook(Chao_Main_r);
}
