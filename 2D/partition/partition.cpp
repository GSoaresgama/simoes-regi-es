#include <iostream>
#include <cmath>

#define MAX_Z 19.2085
#define ERROR 0.0005
#define MAX_CORD_X 8.055
#define MAX_CORD_Y 9.664
#define DESIRED_POP_SIZE 500
#define NUMBER_OF_DIMENSION 2

#define START_MUT 5
#define MUT_DECREASE_RATE 0.8
#define MUT_INCREASE_RATE 2

using namespace std;

const float coordsLimits[] = {10, 10};
const float regionSize[] = {20, 20};
const float MAX_MUT = 50 * coordsLimits[0];

int totalNumberRegions;
int numberOfRegions[NUMBER_OF_DIMENSION];
int regionsByDimension[NUMBER_OF_DIMENSION];
int timeToChange[NUMBER_OF_DIMENSION];

int POP_BY_REGION;
int POP_SIZE;
int test;


//fonte: https://en.wikipedia.org/wiki/Test_functions_for_optimization  (Hölder table function)

struct indvData_t;

typedef struct region_t
{
    float *bestCoords;
    float maxFit = -MAX_Z;
    int maxIndex;

    int hasImproved = 5;

    float startCoords[NUMBER_OF_DIMENSION];
    float endCoords[NUMBER_OF_DIMENSION];
    float MUT = START_MUT;

} region_t;

typedef struct indvData_t
{
    region_t *region;
    float coords[NUMBER_OF_DIMENSION];

    float fitness;

} indvData_t;

float func(float x, float y)
{
    float a = abs(1 - sqrt(x * x + y * y) / M_PI);

    return abs(sin(x) * cos(y) * exp(a));
}

void calculateTimeTochange()
{

    timeToChange[NUMBER_OF_DIMENSION - 1] = 1;
    for (int i = NUMBER_OF_DIMENSION - 2; i >= 0; i--)
    {
        timeToChange[i] = regionsByDimension[i + 1] * timeToChange[i + 1];
    }
}

void defineRegions(region_t *regions)
{
    int i;
    int j;

    float **regionLimits;

    regionLimits = new float *[NUMBER_OF_DIMENSION];

    for (i = 0; i < NUMBER_OF_DIMENSION; i++)
    {
        //region limits
        regionLimits[i] = new float[regionsByDimension[i]];

        for (j = 0; j < regionsByDimension[i]; j++)
        {
            regionLimits[i][j] = j * regionSize[i] - regionSize[i] * regionsByDimension[i] / 2;

            cout << regionLimits[i][j] << endl;
        }
    }

    calculateTimeTochange();

    int k = 0;
    for (int coord = 0; coord < NUMBER_OF_DIMENSION; coord++)
    {
        k = 0;
        for (int n_reg = 0; n_reg < totalNumberRegions; n_reg++)
        {
            if (!(n_reg % timeToChange[coord]) && n_reg != 0)
                k++;

            regions[n_reg].startCoords[coord] = regionLimits[coord][k % regionsByDimension[coord]];
            regions[n_reg].endCoords[coord] = regions[n_reg].startCoords[coord] + regionSize[coord];
        }
    }

    /*
    for (i = 0; i < totalNumberRegions; i++)
    {
        cout << "region(" << i << "): " << regions[i].startCoords[0] << " | ";
        cout << regions[i].startCoords[1] << endl;
    }
    //*/
}

void inicializeIndv(indvData_t *indvs, region_t *regions)
{
    int i;
    int n_dim;
    int region = 0;

    for (i = 0; i < POP_SIZE; i++)
    {
        indvs[i].fitness = -MAX_Z;

        if (!(i % POP_BY_REGION) && i != 0)
            region++;

        indvs[i].region = &regions[region];

        for (n_dim = 0; n_dim < NUMBER_OF_DIMENSION; n_dim++)
        {
            indvs[i].coords[n_dim] = (float)(rand() % (int)(regionSize[n_dim] * 100)) / 100;
            indvs[i].coords[n_dim] += regions[region].startCoords[n_dim];

            //cout << "indv" << i << " :" << indvs[i].coords[n_dim] << " | ";
        }
        //cout << endl;
    }
}

void setRegionMutation(region_t *regions)
{
    for (int i = 0; i < totalNumberRegions; i++)
    {
        if (regions[i].MUT > MAX_MUT)
            regions[i].MUT = MAX_MUT;
        else if (regions[i].MUT < START_MUT)
            regions[i].MUT = START_MUT;

        if (regions[i].hasImproved == 0)
            regions[i].MUT *= MUT_INCREASE_RATE;
        else
        {
            if (regions[i].MUT > START_MUT)
                regions[i].MUT = START_MUT;

            regions[i].MUT *= MUT_DECREASE_RATE;
        }
    }
}

void simpleMutation(indvData_t *indv)
{

    for (int n_dim = 0; n_dim < NUMBER_OF_DIMENSION; n_dim++)
    {
        (*indv).coords[n_dim] += (rand() % (int)(2 * indv->region->MUT) - indv->region->MUT) / 100.0;

        if ((*indv).coords[n_dim] > coordsLimits[n_dim])
            (*indv).coords[n_dim] = coordsLimits[n_dim];
        else if ((*indv).coords[n_dim] < -coordsLimits[n_dim])
            (*indv).coords[n_dim] = -coordsLimits[n_dim];
    }
}

void calculateFitness(indvData_t *indvs, region_t *regions, float *best, int *bestIndex)
{
    for (int i = 0; i < totalNumberRegions; i++)
    {
        if (regions[i].hasImproved > 0)
            regions[i].hasImproved--;
    }

    for (int i = 0; i < POP_SIZE; i++)
    {
        indvs[i].fitness = func(indvs[i].coords[0], indvs[i].coords[1]);

        if (indvs[i].fitness > indvs[i].region->maxFit)
        {
            indvs[i].region->maxFit = indvs[i].fitness;
            indvs[i].region->maxIndex = i;
            indvs[i].region->bestCoords = indvs[i].coords;
            indvs[i].region->hasImproved = 5;

            if (indvs[i].region->maxFit > *best)
            {
                *best = indvs[i].region->maxFit;
                *bestIndex = i;
            }
        }
    }
    test += POP_SIZE;
}

void eletism(indvData_t *indvs, float best, int bestIndex)
{
    for (int i = 0; i < POP_SIZE; i++)
    {
        if (i == indvs[i].region->maxIndex)
            continue;

        for (int n_dim = 0; n_dim < NUMBER_OF_DIMENSION; n_dim++)
        {
            indvs[i].coords[n_dim] = (indvs[i].coords[n_dim] + indvs[i].region->bestCoords[n_dim]) / 2;
        }

        simpleMutation(&indvs[i]);
    }
}

void resetRegion(region_t *regions)
{
    for (int i = 0; i < totalNumberRegions; i++)
    {
        regions[i].hasImproved = 5;
        regions[i].maxFit = -MAX_Z;
        regions[i].MUT = START_MUT;
    }
}

int main()
{
    //srand(time(0));
    srand(0);


    indvData_t *indvs;
    region_t *regions;

    float best = -MAX_Z;

    int gen = 0;
    int bestIndex;
    int totalTest = 0;
    test = 0;
    totalNumberRegions = 1;

    for (int i = 0; i < NUMBER_OF_DIMENSION; i++)
    {
        regionsByDimension[i] = 2 * coordsLimits[i] / regionSize[i];
        totalNumberRegions *= regionsByDimension[i];
    }

    POP_BY_REGION = DESIRED_POP_SIZE / totalNumberRegions;
    POP_SIZE = POP_BY_REGION * totalNumberRegions;

    indvs = new indvData_t[POP_SIZE];
    regions = new region_t[totalNumberRegions];

    //cout << POP_BY_REGION << endl;
    defineRegions(regions);
    inicializeIndv(indvs, regions);

    cout.precision(6);
    for (int i = 0; i < 5000; i++)
    {
        gen = 0;
        best = -MAX_Z;
        totalTest += test;
        test = 0;
        resetRegion(regions);
        inicializeIndv(indvs, regions);

        while (true)
        {
            calculateFitness(indvs, regions, &best, &bestIndex);
            setRegionMutation(regions);
            eletism(indvs, best, bestIndex);

            //cout << best << " | ";
            //cout << gen << endl;
            
            if (best >= MAX_Z - ERROR)
            {
                //cout << test << "," << gen << endl;
                break;
            }
           
            gen++;
        }
    }
    cout << "total reg: " << totalNumberRegions << " | pop by reg: " << POP_BY_REGION << endl;
    cout << "region size: " << regionSize[0] << " | " << regionSize[1] << endl;
    cout << "total test: " << totalTest << endl;
}
