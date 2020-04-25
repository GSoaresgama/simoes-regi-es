#pragma once

#include "structs.hpp"
#include "Region.hpp"

class individual
{
private:
public:
    float x_cord;
    float fitness;

    Region *region;

    individual(/* args */);
    ~individual();

    void reset();
};
