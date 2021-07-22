#include "stdafx.h"

void LookAt(NJS_VECTOR* from, NJS_VECTOR* to, Angle* outx, Angle* outy)
{
	NJS_VECTOR unit = *to;

	unit.x -= from->x;
	unit.y -= from->y;
	unit.z -= from->z;

	if (outy)
	{
		*outy = static_cast<Angle>(atan2f(unit.x, unit.z) * 65536.0f * 0.1591549762031479f);
	}

	if (outx)
	{
		if (from->y == to->y)
		{
			*outx = 0;
		}
		else
		{
			Float len = 1.0f / sqrtf(unit.z * unit.z + unit.x * unit.x + unit.y * unit.y);

			*outx = static_cast<Angle>((acos(len * 3.3499999f) * 65536.0f * 0.1591549762031479f)
				- (acos(-(len * unit.y)) * 65536.0f * 0.1591549762031479f));
		}
	}
}

float GetDistance(NJS_VECTOR* orig, NJS_VECTOR* dest)
{
	return sqrtf(powf(dest->x - orig->x, 2) + powf(dest->y - orig->y, 2) + powf(dest->z - orig->z, 2));
}

void njGetTranslation(NJS_MATRIX_PTR m, NJS_VECTOR* pos)
{
	if (!m)
	{
		m = _nj_current_matrix_ptr_;
	}

	pos->x = m[M03];
	pos->y = m[M13];
	pos->z = m[M23];
}

NJS_VECTOR GetPointToFollow(NJS_VECTOR* pos, NJS_VECTOR* dir, Rotation* rot)
{
	NJS_VECTOR point;
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

void MoveForward(EntityData1* entity, float speed)
{
	njPushUnitMatrix();
	njTranslateEx(&entity->Position);
	njRotateY(_nj_current_matrix_ptr_, entity->Rotation.y);
	njRotateX(_nj_current_matrix_ptr_, entity->Rotation.x);
	njTranslate(_nj_current_matrix_ptr_, 0.0f, 0.0f, speed);
	njGetTranslation(_nj_current_matrix_ptr_, &entity->Position);
	njPopMatrix(1u);
}

void PutBehindPlayer(NJS_VECTOR* pos, EntityData1* data, float dist)
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

EntityData1* GetClosestAttack(NJS_VECTOR* pos, Float range, int playerid)
{
	EntityData1* entityReturn = nullptr;
	Float distanceMin = 10000000;

	uint16_t count = playerid != 1 ? TargetEntitiesP1_Count : TargetEntitiesP2_Count;
	TargetEntityStruct* entitiesArray = playerid != 1 ? TargetEntitiesP1 : TargetEntitiesP2;

	for (int i = 0; i < count; ++i)
	{
		TargetEntityStruct* current = &entitiesArray[i];

		if (current->entity && current->distance < distanceMin)
		{
			distanceMin = current->distance;
			entityReturn = current->entity;
		}
	}

	if (distanceMin < range)
	{
		return entityReturn;
	}
	else {
		return nullptr;
	}
}

EntityData1* GetClosestRing(NJS_VECTOR* pos, Float range, int playerid)
{
	EntityData1* entityReturn = nullptr;
	Float distanceMin = 10000000;

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

	if (distanceMin < range)
	{
		return entityReturn;
	}
	else {
		return nullptr;
	}
}