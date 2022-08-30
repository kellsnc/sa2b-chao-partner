#include "stdafx.h"
#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "common.h"
#include "utils.h"
#include "chao.h"

FunctionHook<void, ObjectMaster*> Chao_Main_hook(0x54FE20);

Buttons ChaoAttackButton = Buttons_Z;

ChaoData1* CurrentChaoData = nullptr;

static void Chao_MoveToTarget(ChaoData1* data1, NJS_POINT3* targetPos, Float speed)
{
	data1->entity.Position = LerpPosition(&data1->entity.Position, targetPos, speed);
	data1->entity.Rotation.y = GetYawAngleToPoint(&data1->entity.Position, targetPos);
}

static float Chao_GetFlightSpeed(ChaoDataBase* database)
{
	return 0.1f + (min((float)(database->StatLevels[ChaoStat_Fly]), 100.0f) / 500.0f);
}

static float Chao_GetAttackRange(ChaoDataBase* database)
{
	return max(0, max(200.0f, min(500.0f, database->StatLevels[ChaoStat_Stamina] * 7)) - (database->Emotion.Tiredness / 2));
}

static bool Chao_AttackCondition(ChaoDataBase* database, CustomData* custom, int playerid)
{
	if (custom->noAutoAttackTimer == 0 && rand() % 200 < max(20, database->Happiness))
	{
		custom->noAutoAttackTimer = 60 + database->Emotion.ColdLevel + database->Emotion.Hunger + database->Emotion.Tiredness + database->Emotion.Anger;
		return true;
	}

	if (custom->noAttackTimer == 0 && Controllers[playerid].press & ChaoAttackButton)
	{
		if (database->Emotion.Sleepiness > 200 && rand() % 3 == 0)
		{
			return false;
		}

		custom->noAttackTimer = 120 + database->Emotion.ColdLevel + database->Emotion.Hunger + database->Emotion.Tiredness + database->Emotion.Anger;

		return true;
	}

	return false;
}

static void Chao_CheckAttack(ChaoData1* data1, ChaoLeash* leash, EntityData1* player, CharObj2Base* co2)
{
	if (ChaoAssist)
	{
		auto target = GetClosestAttack(&data1->entity.Position, Chao_GetAttackRange(data1->ChaoDataBase_ptr), co2->PlayerNum);

		if (target && Chao_AttackCondition(data1->ChaoDataBase_ptr, &leash->custom, co2->PlayerNum))
		{
			data1->entity.NextAction = ChaoAct_Attack;
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

static void ChaoAttack(ChaoData1* data1, EntityData1* player, CharObj2Base* co2, CustomData* custom)
{
	auto target = GetClosestAttack(&data1->entity.Position, Chao_GetAttackRange(data1->ChaoDataBase_ptr), co2->PlayerNum);
	auto dist = GetDistance(&data1->entity.Position, &player->Position);

	if (target == custom->target && dist < Chao_GetAttackRange(data1->ChaoDataBase_ptr))
	{
		auto target_pos = GetEntityCenter(target);
		Chao_MoveToTarget(data1, &target_pos, Chao_GetFlightSpeed(data1->ChaoDataBase_ptr));
	}
	else
	{
		data1->entity.NextAction = ChaoAct_FollowPlayer;
	}
}

static void FollowPlayer(ChaoData1* data1, EntityData1* player, CharObj2Base* co2)
{
	NJS_POINT3 dest = GetFollowPoint(player, co2);

	Chao_MoveToTarget(data1, &dest, Chao_GetFlightSpeed(data1->ChaoDataBase_ptr));

	if (njScalor(&co2->Speed) == 0 && GetDistance(&data1->entity.Position, &dest) < 5.0f)
	{
		data1->entity.NextAction = ChaoAct_IdlePlayer;
	}
}

static void IdlePlayer(ChaoData1* data1, EntityData1* player, CharObj2Base* co2, CustomData* custom)
{
	NJS_POINT3 dest = GetFollowPoint(player, co2);

	if (GetDistance(&custom->pre, &dest) > 3.0f)
	{
		data1->entity.NextAction = ChaoAct_FollowPlayer;
	}

	data1->entity.Rotation.y += 0x100;

	dest.x = custom->pre.x - njCos(-data1->entity.Rotation.y) * 1.0f;
	dest.y = custom->pre.y;
	dest.z = custom->pre.z - njSin(-data1->entity.Rotation.y) * 1.0f;

	data1->entity.Position = LerpPosition(&data1->entity.Position, &dest, 0.1f);
}

bool CheckFlyButton(int playerid)
{
	return Controllers[playerid].press & Buttons_L && Controllers[playerid].press & Buttons_R;
}

static void LevelChao_Fly(ObjectMaster* obj, ChaoData1* data1, ChaoLeash* leash, EntityData1* player, CharObj2Base* co2)
{
	switch (data1->entity.NextAction) {
	case ChaoAct_FollowPlayer:
		FollowPlayer(data1, player, co2);
		Chao_CheckAttack(data1, leash, player, co2);
		leash->custom.pre = data1->entity.Position;
		break;
	case ChaoAct_IdlePlayer:
		IdlePlayer(data1, player, co2, &leash->custom);
		Chao_CheckAttack(data1, leash, player, co2);

		break;
	case ChaoAct_Attack:
		ChaoAttack(data1, player, co2, &leash->custom);
		leash->custom.pre = data1->entity.Position;
		break;
	}

	// Flying animation
	if (FrameCountIngame % 30 == 0)
	{
		Chao_Animation(&data1->MotionTable, 286); // or 130
	}

	Chao_PlayAnimation(obj);
	Chao_RunEmotionBall(obj);
	Chao_MoveEmotionBall(obj);
	Chao_RunPhysics(obj);

	if (CheckFlyButton(data1->entity.Index))
	{
		data1->entity.Status &= ~StatusChao_FlyPlayer;
		leash->mode = ChaoLeashMode_Free;
	}
}

static void LevelChao_Normal(ObjectMaster* obj, ChaoData1* data1, ChaoLeash* leash)
{
	Chao_RunMovements(obj);
	Chao_PlayAnimation(obj);
	Chao_RunEmotionBall(obj);
	Chao_RunActions(obj);
	Chao_MoveEmotionBall(obj);

	// Fly if Y is pressed
	if (!(data1->entity.Status & StatusChao_Held))
	{
		Chao_RunGravity(obj);
		leash->mode = ChaoLeashMode_Free;

		if (CheckFlyButton(data1->entity.Index))
		{
			data1->entity.Status |= StatusChao_FlyPlayer;
			leash->mode = ChaoLeashMode_Fly;
		}
	}

	Chao_RunPhysics(obj);

	leash->custom.pre = data1->entity.Position;
}

static void LevelChao_UpdateStuff(CustomData* custom)
{
	if (custom->noAttackTimer > 0) custom->noAttackTimer -= 1;
	if (custom->noAutoAttackTimer > 0) custom->noAutoAttackTimer -= 1;
}

void __cdecl Chao_Main_r(ObjectMaster* obj)
{
	ChaoData1* data1 = obj->Data1.Chao;
	CurrentChaoData = data1;

	if (CurrentLevel != LevelIDs::LevelIDs_ChaoWorld)
	{
		SoundsPaused = TRUE;

		if (data1->gap_30 <= 2u)
		{
			sub_53CAC0(obj);
		}

		EntityData1* player = MainCharObj1[data1->entity.Index];
		EntityData2* pmotion = (EntityData2*)MainCharacter[data1->entity.Index]->EntityData2;
		CharObj2Base* co2 = MainCharObj2[data1->entity.Index];
		ChaoLeash* leash = &CarriedChao[data1->entity.Index];

		// If the player cannot be found, act as a normal Chao
		if (player == nullptr)
		{
			Chao_Main_hook.Original(obj);
			return;
		}

		LevelChao_UpdateStuff(&leash->custom);

		// Run custom actions
		if (!(data1->entity.Status & StatusChao_FlyPlayer))
		{
			data1->entity.Status |= 0x40;
			LevelChao_Normal(obj, data1, leash);
		}
		else
		{
			data1->entity.Status &= ~0x40;
			LevelChao_Fly(obj, data1, leash, player, co2);
		}

		++data1->gap_30;

		if ((data1->Flags & 8) != 0)
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
