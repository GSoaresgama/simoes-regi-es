#pragma once

#include "structs.hpp"

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
