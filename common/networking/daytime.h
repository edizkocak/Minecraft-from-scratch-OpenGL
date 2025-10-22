#ifndef GRAPA_DAYTIME_H
#define GRAPA_DAYTIME_H

namespace daytime {
    const double ONE_DAY = 240 * 1000;

    float dayFract(double ms);
    int hour(float fract);

    float sunAmbient(float fract);

    int numAnimals(float fract);
    int numEnemies(float fract);
}

#endif //GRAPA_DAYTIME_H
