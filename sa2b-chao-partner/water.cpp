#include "stdafx.h"

float GetWaterHeight(ChaoData1* chaodata1)
{
    NJS_VECTOR pos = { chaodata1->entity.Position.x, chaodata1->entity.Position.y - 2.0f, chaodata1->entity.Position.z };
    CharSurfaceInfo surfaceinfo;

    GetActiveCollisions(pos.x, pos.y, pos.z, 200.0f);
    GetCharacterSurfaceInfo(&pos, &surfaceinfo);

    if (surfaceinfo.TopSurface & SurfaceFlag_Water)
    {
        return surfaceinfo.TopSurfaceDist;
    }
    else
    {
        return -10000000.0f;
    }
}

BOOL __cdecl Chao_DetectWater_r(ObjectMaster* obj);
Trampoline Chao_DetectWater_t(0x561630, 0x561635, Chao_DetectWater_r);
BOOL __cdecl Chao_DetectWater_r(ObjectMaster* obj)
{
    ChaoData1* data1 = (ChaoData1*)obj->Data1.Chao;

    if (CurrentLevel == LevelIDs_ChaoWorld)
    {
        return ((decltype(Chao_DetectWater_r)*)Chao_DetectWater_t.Target())(obj);
    }
    else
    {
        ChaoData1* data = obj->Data1.Chao;
        ChaoData2* data2 = (ChaoData2*)obj->EntityData2;

        if (data->entity.Status >= 0)
        {
            data2->WaterHeight = GetWaterHeight(data);

            if (data->entity.Position.y + 2.0f >= data2->WaterHeight)
            {
                data->ChaoBehaviourInfo.CurrentActionInfo.field_0 &= 0xFFFAu;
                return FALSE;
            }
            else
            {
                if (!(data->ChaoBehaviourInfo.CurrentActionInfo.field_0 & 1))
                {
                    data->ChaoBehaviourInfo.CurrentActionInfo.field_0 = data->ChaoBehaviourInfo.CurrentActionInfo.field_0 | 1;
                    SE_CallV2(0x1020, &obj->Data1.Entity->Position, 0, 0, 0);
                    RunChaoBehaviour(obj, (void*)0x562330, -1);
                }

                if (data2->float4 < 0.0f)
                {
                    data2->float4 *= 0.1f;
                }

                data->ChaoBehaviourInfo.CurrentActionInfo.field_0 |= 4u;
                return TRUE;
            }
        }
        else
        {
            data->ChaoBehaviourInfo.CurrentActionInfo.field_0 &= 0xFFFEu;
            return FALSE;
        }
    }
}