#include "stdafx.h"
#include "SA2ModLoader.h"
#include "common.h"

NJS_POINT3 LerpPosition(NJS_POINT3* from, NJS_POINT3* to, Float spd)
{
	NJS_POINT3 out;
	out.x = from->x + (to->x - from->x) * spd;
	out.y = from->y + (to->y - from->y) * spd;
	out.z = from->z + (to->z - from->z) * spd;
	return out;
}

Angle GetYawAngleToPoint(NJS_POINT3* from, NJS_POINT3* to)
{
	NJS_POINT3 unit;
	unit.x = to->x - from->x;
	unit.y = to->y - from->y;
	unit.z = to->z - from->z;
	return njArcTan2(unit.x, unit.z);
}

float GetDistance(NJS_POINT3* orig, NJS_POINT3* dest)
{
	return sqrtf(powf(dest->x - orig->x, 2) + powf(dest->y - orig->y, 2) + powf(dest->z - orig->z, 2));
}

void njGetTranslation(NJS_MATRIX_PTR m, NJS_POINT3* pos)
{
	if (!m)
	{
		m = _nj_current_matrix_ptr_;
	}

	pos->x = m[M03];
	pos->y = m[M13];
	pos->z = m[M23];
}

NJS_POINT3 GetPointToFollow(NJS_POINT3* pos, NJS_POINT3* dir, Rotation* rot)
{
	NJS_POINT3 point;
	njPushUnitMatrix();
	njTranslateEx(pos);
	njRotateZ(_nj_current_matrix_ptr_, rot->z);
	njRotateX(_nj_current_matrix_ptr_, rot->x);
	njRotateY(_nj_current_matrix_ptr_, -rot->y);
	njTranslateEx(dir);
	njGetTranslation(_nj_current_matrix_ptr_, &point);
	njPopMatrix(1u);

	return point;
}

void PutBehindPlayer(NJS_POINT3* pos, EntityData1* data, Float dist)
{
	if (data)
	{
		if (pos)
		{
			pos->x = data->Position.x - njCos(data->Rotation.y) * dist;
			pos->y = data->Position.y;
			pos->z = data->Position.z - njSin(data->Rotation.y) * dist;
		}
	}
}

EntityData1* GetClosestAttack(NJS_POINT3* pos, Float range, int playerid)
{
	EntityData1* entityReturn = nullptr;
	Float distanceMin = range;

	uint16_t count = playerid != 1 ? TargetEntitiesP1_Count : TargetEntitiesP2_Count;
	TargetEntityStruct* entitiesArray = playerid != 1 ? TargetEntitiesP1 : TargetEntitiesP2;

	for (int i = 0; i < count; ++i)
	{
		TargetEntityStruct* current = &entitiesArray[i];

		Float dist = GetDistance(pos, &current->entity->Position);

		if (current->entity && dist < distanceMin)
		{
			distanceMin = dist;
			entityReturn = current->entity;
		}
	}

	return entityReturn;
}

EntityData1* GetClosestRing(NJS_POINT3* pos, Float range, int playerid)
{
	EntityData1* entityReturn = nullptr;
	Float distanceMin = range;

	uint16_t count = playerid != 1 ? TargetRingEntitiesP1_Count : TargetRingEntitiesP2_Count;
	TargetEntityStruct* entitiesArray = playerid != 1 ? TargetRingEntitiesP1 : TargetRingEntitiesP2;

	for (int i = 0; i < count; ++i)
	{
		TargetEntityStruct* current = &entitiesArray[i];

		if (current->entity && current->distance < distanceMin)
		{
			distanceMin = current->distance;
			entityReturn = current->entity;
		}
	}

	return entityReturn;
}
