#pragma once

VoidFunc(LoadChaoPalette, 0x534350);

void LookAt(NJS_VECTOR* from, NJS_VECTOR* to, Angle* outx, Angle* outy);
float GetDistance(NJS_VECTOR* orig, NJS_VECTOR* dest);
NJS_VECTOR GetPointToFollow(NJS_VECTOR* pos, NJS_VECTOR* dir, Rotation* rot);
void MoveForward(EntityData1* entity, float speed);
void PutBehindPlayer(NJS_VECTOR* pos, EntityData1* data, float dist);