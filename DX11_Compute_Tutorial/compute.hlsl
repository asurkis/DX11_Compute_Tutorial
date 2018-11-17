#include "SharedConst.h"

RWBuffer<float2> position: register(u0);
RWBuffer<float2> velocity: register(u1);

[numthreads(NUMTHREADS, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    float2 acc = float2(0, 0);
    for (uint i = 0; i < PARTICLE_COUNT; i++)
    {
        float2 diff = position[i] - position[id.x];
        float len = max(1e-10, length(diff));
        float k = 1e-9 * (len - 0.25) / len;
        acc += k * diff;
    }

    position[id.x] += velocity[id.x] + 0.5 * acc;
    velocity[id.x] += acc;
}
