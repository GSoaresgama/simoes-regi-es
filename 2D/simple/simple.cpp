#include <iostream>
#include <cmath>

#define MAX_Z 19.2085
#define ERROR 0.0005
#define MAX_CORD_X 8.055
#define MAX_CORD_Y 9.664
#define POP_SIZE 500

#define START_MUT 5
#define MUT_DECREASE_RATE 0.8
#define MUT_INCREASE_RATE 2

using namespace std;

const float coordsLimits[] = {10, 10};
const float MAX_MUT = 50 * coordsLimits[0];

int test = 0;
int hasImproved = 5;
float MUT = START_MUT;

//fonte: https://en.wikipedia.org/wiki/Test_functions_for_optimization  (Hölder table function)

typedef struct indvData_t
{
    float x;
    float y;

    float fitness;

} indvData_t;

float func(float x, float y)
{
    float a = abs(1 - sqrt(x * x + y * y) / M_PI);

    return abs(sin(x) * cos(y) * exp(a));
}

void inicializeIndv(indvData_t *indvs)
{
    for (int i = 0; i < POP_SIZE; i++)
    {
        indvs[i].fitness = -MAX_Z;
        indvs[i].x = (float)(rand() % (int)(2 * coordsLimits[0] * 100) - coordsLimits[0] * 100) / 100;
        indvs[i].y = (float)(rand() % (int)(2 * coordsLimits[1] * 100) - coordsLimits[1] * 100) / 100;

        //cout << indvs[i].x << " | " << indvs[i].y << endl;
    }
}

void simpleMutation(indvData_t *indv)
{
    if (MUT > MAX_MUT)
        MUT = MAX_MUT;
    else if (MUT < START_MUT)
        MUT = START_MUT;

    if (hasImproved == 0)
        MUT *= MUT_INCREASE_RATE;
    else
    {
        if (MUT > START_MUT)
            MUT = START_MUT;

        MUT *= MUT_DECREASE_RATE;
    }

    (*indv).x += (rand() % (int)(2 * MUT) - MUT) / 100.0;
    (*indv).y += (rand() % (int)(2 * MUT) - MUT) / 100.0;

    if ((*indv).x > coordsLimits[0])
        (*indv).x = coordsLimits[0];
    else if ((*indv).x < -coordsLimits[0])
        (*indv).x = -coordsLimits[0];

    if ((*indv).y > coordsLimits[1])
        (*indv).y = coordsLimits[1];
    else if ((*indv).y < -coordsLimits[1])
        (*indv).y = -coordsLimits[1];
}

void calculateFitness(indvData_t *indvs, float *best, int *bestIndex)
{
    if (hasImproved > 0)
        hasImproved--;

    for (int i = 0; i < POP_SIZE; i++)
    {
        indvs[i].fitness = func(indvs[i].x, indvs[i].y);

        if (indvs[i].fitness > *best)
        {
            *best = indvs[i].fitness;
            *bestIndex = i;

            hasImproved = 5;
        }
    }

    test += POP_SIZE;
}

void eletism(indvData_t *indvs, float best, int bestIndex)
{
    for (int i = 0; i < POP_SIZE; i++)
    {
        if (i == bestIndex)
            continue;

        indvs[i].x = (indvs[i].x + indvs[bestIndex].x) / 2;
        indvs[i].y = (indvs[i].y + indvs[bestIndex].y) / 2;

        simpleMutation(&indvs[i]);
    }
}

int main()
{
    srand(time(0));

    indvData_t *indvs;

    float best = -MAX_Z;

    int gen = 0;
    int bestIndex;
    int totalTest = 0;
    int totalGen = 0;

    indvs = new indvData_t[POP_SIZE];
    inicializeIndv(indvs);

    cout.precision(6);
    for (int i = 0; i < 5000; i++)
    {
        totalTest += test;
        totalGen += gen;
        gen = 0;
        test = 0;
        inicializeIndv(indvs);
        hasImproved = 5;
        MUT = START_MUT;
        best = -MAX_Z;

        while (true)
        {
            calculateFitness(indvs, &best, &bestIndex);
            eletism(indvs, best, bestIndex);

            //cout << best << endl;

            gen++;
            if (best >= MAX_Z - ERROR)
            {
                cout << test << "," << gen << endl;
                break;
            }
        }
    }

    cout << "total test: " << totalTest << " | totalGen: " << totalGen << endl;
}
