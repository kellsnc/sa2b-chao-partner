#include "stdafx.h"

enum CustomChaoActs {
	ChaoAct_FollowPlayer,
	ChaoAct_IdlePlayer,
	ChaoAct_Attack
};

float Chao_GetFlightSpeed(ChaoData1* data1) {
	return max(0.8f, min(data1->ChaoDataBase_ptr->StatLevels[1], 99) / 50);
}

float Chao_GetAttackRange(ChaoData1* data1) {
	return max(200.0f, min(500.0f, data1->ChaoDataBase_ptr->StatLevels[4] * 7));
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

void FollowPlayer(ChaoData1* data1, EntityData1* player, CharObj2Base* co2) {
	NJS_VECTOR dest = GetPlayerPoint(player, co2);

	float dist = GetDistance(&data1->entity.Position, &dest);

	if (dist > 5.0f) {
		LookAt(&data1->entity.Position, &dest, &data1->entity.Rotation.x, &data1->entity.Rotation.y);
		MoveForward(&data1->entity, Chao_GetFlightSpeed(data1) * (dist / 8));
		data1->entity.Rotation.x = 0;
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
	case ChaoAct_Attack:
		FollowPlayer(data1, player, co2);
		break;
	case ChaoAct_IdlePlayer:
		IdlePlayer(data1, player, co2);
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