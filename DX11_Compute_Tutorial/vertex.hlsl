#include "common.hlsli"

VertexOut main(float2 pos : POSITION, float3 col: COLOR)
{
    VertexOut vert;
    vert.pos = float4(pos, 0, 1);
    vert.col = col;
    return vert;
}
