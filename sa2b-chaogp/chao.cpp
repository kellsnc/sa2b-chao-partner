#include "stdafx.h"

Buttons ChaoAttackButton = Buttons_Z;

enum CustomChaoActs {
	ChaoAct_FollowPlayer,
	ChaoAct_IdlePlayer,
	ChaoAct_Attack
};

void Chao_MoveToTarget(ChaoData1* data1, NJS_VECTOR* targetPos, float speed) {
	LookAt(&data1->entity.Position, targetPos, &data1->entity.Rotation.x, &data1->entity.Rotation.y);
	MoveForward(&data1->entity, speed);
	data1->entity.Rotation.x = 0;
}

float Chao_GetFlightSpeed(ChaoDataBase* database) {
	return max(0.8f, min(database->StatLevels[ChaoStat_Fly], 99) / 50);
}

float Chao_GetAttackRange(ChaoDataBase* database) {
	return max(0, max(200.0f, min(500.0f, database->StatLevels[ChaoStat_Stamina] * 7)) - (database->Emotion.Tiredness / 2));
}

bool Chao_AttackCondition(ChaoDataBase* database, CustomData* custom, int playerid) {
	if (custom->noAutoAttackTimer == 0 && rand() % 200 < max(20, database->Happiness)) {
		custom->noAutoAttackTimer = 60 + database->Emotion.ColdLevel + database->Emotion.Hunger + database->Emotion.Tiredness + database->Emotion.Anger;
		return true;
	}

	if (custom->noAttackTimer == 0 && Controllers[playerid].press & ChaoAttackButton) {
		if (database->Emotion.Sleepiness > 200 && rand() % 3 == 0) {
			return false;
		}

		custom->noAttackTimer = 120 + database->Emotion.ColdLevel + database->Emotion.Hunger + database->Emotion.Tiredness + database->Emotion.Anger;

		return true;
	}

	return false;
}

void Chao_CheckAttack(ChaoData1* data1, ChaoLeash* leash, EntityData1* player, CharObj2Base* co2) {
	EntityData1* target = GetClosestAttack(&data1->entity.Position, Chao_GetAttackRange(data1->ChaoDataBase_ptr), co2->PlayerNum);

	if (target && Chao_AttackCondition(data1->ChaoDataBase_ptr, &leash->custom, co2->PlayerNum)) {
		data1->entity.NextAction = ChaoAct_Attack;
		leash->custom.targetPos = target->Position;
	}
}

NJS_VECTOR GetPlayerPoint(EntityData1* player, CharObj2Base* co2) {
	NJS_VECTOR dir = { -6, 7, 5 };

	if (co2->CharID == Characters_Eggman) {
		dir = { -10, 14, 7 };
	}
	else if (co2->CharID == Characters_MechEggman || co2->CharID == Characters_MechTails) {
		dir = { -8, 16, 11 };
	}

	return GetPointToFollow(&player->Position, &dir, &player->Rotation);
}

void ChaoAttack(ChaoData1* data1, EntityData1* player, CustomData* custom) {
	float dist = GetDistance(&data1->entity.Position, &custom->targetPos);

	if (dist > 3.0f && GetDistance(&data1->entity.Position, &player->Position) < Chao_GetAttackRange(data1->ChaoDataBase_ptr)) {
		Chao_MoveToTarget(data1, &custom->targetPos, Chao_GetFlightSpeed(data1->ChaoDataBase_ptr) * (dist / 4));
	}
	else {
		data1->entity.NextAction = ChaoAct_FollowPlayer;
	}
}

void FollowPlayer(ChaoData1* data1, EntityData1* player, CharObj2Base* co2) {
	NJS_VECTOR dest = GetPlayerPoint(player, co2);

	float dist = GetDistance(&data1->entity.Position, &dest);

	if (dist > 5.0f) {
		Chao_MoveToTarget(data1, &dest, Chao_GetFlightSpeed(data1->ChaoDataBase_ptr) * (dist / 8));
	}
	else if (co2->Speed.x + co2->Speed.y < 0.2f) {
		data1->entity.NextAction = ChaoAct_IdlePlayer;
	}
}

void IdlePlayer(ChaoData1* data1, EntityData1* player, CharObj2Base* co2) {
	NJS_VECTOR dest = GetPlayerPoint(player, co2);

	float dist = GetDistance(&data1->entity.Position, &dest);

	if (dist < 11.0f && co2->Speed.x + co2->Speed.y < 0.2f) {
		data1->entity.Rotation.y += 0x200;
		data1->entity.Rotation.x = 0;
		MoveForward(&data1->entity, 0.1f);
	}
	else {
		data1->entity.NextAction = ChaoAct_FollowPlayer;
	}
}

void LevelChao_Fly(ObjectMaster* obj, ChaoData1* data1, ChaoLeash* leash, EntityData1* player, CharObj2Base* co2) {
	switch (data1->entity.NextAction) {
	case ChaoAct_FollowPlayer:
		FollowPlayer(data1, player, co2);
		Chao_CheckAttack(data1, leash, player, co2);

		break;
	case ChaoAct_IdlePlayer:
		IdlePlayer(data1, player, co2);
		Chao_CheckAttack(data1, leash, player, co2);

		break;
	case ChaoAct_Attack:
		ChaoAttack(data1, player, &leash->custom);
		break;
	}

	// Flying animation
	if (FrameCountIngame % 30 == 0) {
		Chao_Animation(&data1->MotionTable, 286); // or 130, 286
	}

	Chao_PlayAnimation(obj);
	Chao_RunEmotionBall(obj);
	Chao_MoveEmotionBall(obj);
	Chao_RunPhysics(obj);

	if (Controllers[data1->entity.field_2].press & Buttons_L) {
		data1->entity.Status &= ~StatusChao_FlyPlayer;
		leash->mode = ChaoLeashMode_Free;
	}
}

void LevelChao_Normal(ObjectMaster* obj, ChaoData1* data1, ChaoLeash* leash) {
	Chao_RunMovements(obj);
	Chao_PlayAnimation(obj);
	Chao_RunEmotionBall(obj);
	Chao_RunActions(obj);
	Chao_MoveEmotionBall(obj);

	// Fly if Y is pressed
	if (!(data1->entity.Status & StatusChao_Held)) {
		Chao_RunGravity(obj);
		leash->mode = ChaoLeashMode_Free;

		if (Controllers[data1->entity.field_2].press & Buttons_L) {
			data1->entity.Status |= StatusChao_FlyPlayer;
			leash->mode = ChaoLeashMode_Fly;
		}
	}

	Chao_RunPhysics(obj);
}

void LevleChao_UpdateStuff(CustomData* custom) {
	if (custom->noAttackTimer > 0) custom->noAttackTimer -= 1;
	if (custom->noAutoAttackTimer > 0) custom->noAutoAttackTimer -= 1;
}

void __cdecl Chao_Main_r(ObjectMaster* obj);
Trampoline Chao_Main_t((int)Chao_Main, (int)Chao_Main + 0x8, Chao_Main_r);
void __cdecl Chao_Main_r(ObjectMaster* obj) {
	ChaoData1* data1 = obj->Data1.Chao;

	if (CurrentLevel != LevelIDs::LevelIDs_ChaoWorld) {
		EntityData1* player = MainCharObj1[data1->entity.field_2];
		CharObj2Base* co2 = MainCharObj2[data1->entity.field_2];
		ChaoLeash* leash = &CarriedChao[data1->entity.field_2];

		// If the player cannot be found, act as a normal Chao
		if (player == nullptr) {
			TARGET_STATIC(Chao_Main)(obj);
			return;
		}

		LevleChao_UpdateStuff(&leash->custom);

		// Run custom actions
		if (!(data1->entity.Status & StatusChao_FlyPlayer)) {
			LevelChao_Normal(obj, data1, leash);
		}
		else {
			LevelChao_Fly(obj, data1, leash, player, co2);
		}

		if (!(data1->entity.Status & StatusChao_Held)) {
			AddToCollisionList(obj);
		}
	}
	else {
		TARGET_STATIC(Chao_Main)(obj);
	}
}