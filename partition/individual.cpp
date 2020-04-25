#include "individual.hpp"
#include <math.h>

individual::individual(/* args */)
{
}

individual::~individual()
{
}

void individual::setInicialValues(Region *region)
{
    fitness = -MAX_Y;
    x_cord = (rand() % (regionSize * 100)) / 100.0 + region->startX;
    x_cord -= X_LIMITS;

    this->region = region;
}
