#pragma once

#include "structs.hpp"

#define NUMBER_OF_REGIONS 5
const int regionSize = 2 * X_LIMITS / NUMBER_OF_REGIONS;

class Region
{
private:
    /* data */
public:
    int hasImproved;
    int startX;
    int endX;
    int quantIndvs;

    float bestX;
    float best;
    float mutChange;

    Region(/* args */);
    ~Region();
};
