#include "daytime.h"
#include <cppgl.h>

/**
 * @return in range [0,1). 0.0 == 0:00h, 0.5 == 12:00h, 1.0 == 24:00h
 */
float daytime::dayFract(double ms) {
    return (float) glm::fract(ms/daytime::ONE_DAY);
}

int daytime::hour(float fract) {
    return (int) (24.f * fract);
}

float daytime::sunAmbient(float fract) {
    float min = 0.1f;
    float max = 0.7f;

    // increase amount
    float morningStart = 0.25f; // 6:00h
    float morningEnd = 0.333333f; // 8:00h

    // decrease amount
    float eveningStart = 0.791667f; // 19:00h
    float eveningEnd = 0.875f; // 21:00h

    if(fract<morningStart){
        return min;
    }
    if(fract<morningEnd){
        float alpha = glm::smoothstep(morningStart, morningEnd, fract);
        return min + (max-min) * alpha;
    }

    if(fract<eveningStart){
        return max;
    }
    if(fract<eveningEnd){
        float alpha = glm::smoothstep(eveningEnd, eveningStart, fract);
        return min + (max-min) * alpha;
    }
    return 0;
}

int daytime::numAnimals(float fract) {
    static float max = (int) globals::NUM_ANIMALS;

    // increase amount
    float morningStart = 0.25f; // 6:00h
    float morningEnd = 0.333333f; // 8:00h

    // decrease amount
    float eveningStart = 0.791667f; // 19:00h
    float eveningEnd = 0.875f; // 21:00h

    if(fract<morningStart){
        return 0;
    }
    if(fract<morningEnd){
        float alpha = glm::smoothstep(morningStart, morningEnd, fract);
        return (int) (max * alpha);
    }

    if(fract<eveningStart){
        return (int) max;
    }
    if(fract<eveningEnd){
        float alpha = glm::smoothstep(eveningEnd, eveningStart, fract);
        return (int) (max * alpha);
    }
    return 0;
}

int daytime::numEnemies(float fract) {
    static float max = (int) globals::NUM_ENEMIES;

    // increase amount
    float eveningStart = 0.86458333f; // 20:45h
    float eveningEnd = 0.8854166667f; // 21:15h

    // decrease amount
    float morningStart = 0.2395833f; // 5:45h
    float morningEnd = 0.26041667f; // 6:15h

    if(fract<morningStart){
        return (int) max;
    }
    if(fract<morningEnd){
        float alpha = glm::smoothstep(morningEnd, morningStart, fract);
        return (int) (max * alpha);
    }

    if(fract<eveningStart){
        return 0;
    }
    if(fract<eveningEnd){
        float alpha = glm::smoothstep(eveningStart, eveningEnd, fract);
        return (int) (max * alpha);
    }
    return (int) max;
}
