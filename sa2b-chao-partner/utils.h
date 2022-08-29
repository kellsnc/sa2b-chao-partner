#pragma once

NJS_POINT3 LerpPosition(NJS_POINT3* from, NJS_POINT3* to, Float spd);
Angle GetYawAngleToPoint(NJS_POINT3* from, NJS_POINT3* to);
float GetDistance(NJS_POINT3* orig, NJS_POINT3* dest);
NJS_POINT3 GetPointToFollow(NJS_POINT3* pos, NJS_POINT3* dir, Rotation* rot);
void PutBehindPlayer(NJS_POINT3* pos, EntityData1* data, Float dist);

EntityData1* GetClosestAttack(NJS_POINT3* pos, Float range, int playerid);
EntityData1* GetClosestRing(NJS_POINT3* pos, Float range, int playerid);
NJS_POINT3 GetEntityCenter(EntityData1* data);
