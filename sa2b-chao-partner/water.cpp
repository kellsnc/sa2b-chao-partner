#include "stdafx.h"
#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "common.h"
#include "water.h"

FunctionHook<BOOL, ObjectMaster*> AL_CheckWater_hook(0x561630);

static float GetWaterHeight(NJS_POINT3* pos)
{
    CharSurfaceInfo surfaceinfo;

    GetActiveCollisions(pos->x, pos->y, pos->z, 200.0f);
    GetCharacterSurfaceInfo(pos, &surfaceinfo);

    if (surfaceinfo.TopSurface & SurfaceFlag_Water)
    {
        return surfaceinfo.TopSurfaceDist;
    }
    else
    {
        return -10000000.0f;
    }
}

BOOL __cdecl AL_CheckWater_r(ObjectMaster* obj)
{
    if (CurrentLevel == LevelIDs_ChaoWorld)
    {
        return AL_CheckWater_hook.Original(obj);
    }
    else
    {
        auto chaowp = (CHAOWK*)obj->Data1.Chao;
        auto movewp = (MOVE_WORK*)obj->EntityData2;

        if (chaowp->entity.Status >= 0)
        {
            movewp->WaterY = GetWaterHeight(&chaowp->entity.Position);

            if (chaowp->entity.Position.y + 2.0f >= movewp->WaterY)
            {
                chaowp->Behavior.Flag &= ~(0x4 | 0x1);
                return FALSE;
            }
            else
            {
                if (!(chaowp->Behavior.Flag & 1))
                {
                    chaowp->Behavior.Flag |= 0x1;
                    dsPlay_iloop(0x1020, &obj->Data1.Entity->Position, 0, 0, 0);
                    AL_SetBehaviorWithTimer(obj, ALBHV_Swim, -1);
                }

                if (movewp->Velo.y < 0.0f)
                {
                    movewp->Velo.y *= 0.1f;
                }

                chaowp->Behavior.Flag |= 0x4;
                return TRUE;
            }
        }
        else
        {
            chaowp->Behavior.Flag &= ~0x1;
            return FALSE;
        }
    }
}

void PatchWaterDetection()
{
    AL_CheckWater_hook.Hook(AL_CheckWater_r);
}
