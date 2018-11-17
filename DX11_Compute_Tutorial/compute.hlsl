#include "SharedConst.h"

// Буфер позиций, UAV в слоте 0
RWBuffer<float2> position: register(u0);
// Буфер скоростей, UAV в слоте 1
RWBuffer<float2> velocity: register(u1);

// Количество потоков выполнения
[numthreads(NUMTHREADS, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    float2 acc = float2(0, 0);
    for (uint i = 0; i < PARTICLE_COUNT; i++)
    {
        // Вектор от одной точки до другой
        float2 diff = position[i] - position[id.x];
        // Берем минимальное значение модуля вектора, чтобы не рассматривать случай 0-вектора
        float len = max(1e-10, length(diff));
        float k = 1e-9 * (len - 0.25) / len;
        acc += k * diff;
    }

    position[id.x] += velocity[id.x] + 0.5 * acc;
    velocity[id.x] += acc;
}
