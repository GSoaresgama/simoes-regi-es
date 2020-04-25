#include "individual.hpp"
#include <math.h>

individual::individual(/* args */)
{
    fitness = -MAX_Y;
    x_cord = (rand() % (2 * X_LIMITS * 100) - X_LIMITS * 100) / 100.0;

    if (x_cord < -X_LIMITS)
        x_cord = -X_LIMITS;
    else if (x_cord > X_LIMITS)
        x_cord = X_LIMITS;

    region = nullptr;
}

individual::~individual()
{
}
