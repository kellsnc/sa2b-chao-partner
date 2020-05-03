#include "stdafx.h"

Rotation fPositionToRotation(NJS_VECTOR* orig, NJS_VECTOR* point) {
	NJS_VECTOR dist;
	Rotation result;

	dist.x = point->x - orig->x;
	dist.y = point->y - orig->y;
	dist.z = point->z - orig->z;

	result.x = atan2(dist.y, dist.z) * 65536.0 * -0.1591549762031479;
	result.y = atan2(dist.x, dist.z) * 65536.0 * 0.1591549762031479;

	result.y = -result.y - 0x4000;
	return result;
}

NJS_VECTOR TransformSpline(NJS_VECTOR* orig, NJS_VECTOR* dest, float state) {
	return { (dest->x - orig->x) * state + orig->x,
			(dest->y - orig->y) * state + orig->y,
			(dest->z - orig->z) * state + orig->z };
}

float GetDistance(NJS_VECTOR* orig, NJS_VECTOR* dest) {
	return sqrtf(powf(dest->x - orig->x, 2) + powf(dest->y - orig->y, 2) + powf(dest->z - orig->z, 2));
}

float* njPushUnitMatrix() {
	float* v8 = _nj_current_matrix_ptr_;
	float* v9 = _nj_current_matrix_ptr_ + 12;
	v8 = _nj_current_matrix_ptr_ + 12;
	memcpy(v9, _nj_current_matrix_ptr_, 0x30u);
	_nj_current_matrix_ptr_ = v9;
	memset(v8, 0, 0x30u);
	*v8 = 1.0;
	v8[5] = 1.0;
	v8[10] = 1.0;
	return v8;
}

void njTranslateV(float* matrix, NJS_VECTOR* pos) {
	njTranslate(matrix, pos->x, pos->y, pos->z);
}

NJS_VECTOR njCalcPoint(float* matrix, float x, float y, float z) {
	return { matrix[1] * y + *matrix * x + matrix[2] * z,
			matrix[4] * x + matrix[5] * y + matrix[6] * z,
			matrix[8] * x + matrix[9] * y + matrix[10] * z };
}