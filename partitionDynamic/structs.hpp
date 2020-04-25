#pragma once

#define MAX_Y 6.0
#define X_LIMITS 75
#define MIN_REGION_POP 5
#define BEGIN_MUT_VAL 0.05

template <typename T>
struct list_t
{
    T *data;
    struct list_t *next;
};