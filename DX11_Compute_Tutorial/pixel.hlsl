#include "common.hlsli"

float4 main(VertexOut vert) : SV_TARGET
{
    return float4(vert.col, 1);
}
