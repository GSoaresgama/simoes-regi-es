#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <math.h>

#include "structs.hpp"
#include "individual.hpp"

//g++ *.cpp -o partitionClass.o `pkg-config --cflags --libs opencv4`

#define HEIGHT 600
#define LENGHT 1800

#define START_MUT 50
#define MUT_DECREASE_RATE 0.80
#define MUT_INCREASE_RATE 2

#define MAX_VALUE 4.76224
#define ERROR 0.0005

using namespace std;
using namespace cv;

//globals:
const float X_SCALE = LENGHT / (2.0 * X_LIMITS);
const float Y_SCALE = HEIGHT / (MAX_Y * 2 * 1.2);
const float MAX_MUT = 2 * X_LIMITS * 100 / NUMBER_OF_REGIONS;

float bestXcord = 0;
float best = -MAX_Y;

int test = 0;

Mat functionImg(HEIGHT, LENGHT, CV_8UC3, Scalar(0, 0, 0));

float func(float x)
{
    return (2 * cos(0.39 * x) + 5 * sin(0.5 * x) + 0.5 * cos(0.1 * x) + 10 * sin(0.7 * x) + 5 * sin(1 * x) + 5 * sin(0.35 * x)) / 5;
}

void swap(individual *a, individual *b)
{
    individual t = *a;
    *a = *b;
    *b = t;
}

void quickSortXcord(vector<individual> &array, int low, int high)
{
    int i = low;
    int j = high;
    float pivot = array[(i + j) / 2].x_cord;

    while (i <= j)
    {
        while (array[i].x_cord < pivot)
            i++;

        while (array[j].x_cord > pivot)
            j--;

        if (i <= j)
        {
            swap(&array[i], &array[j]);
            i++;
            j--;
        }
    }

    if (j > low)
        quickSortXcord(array, low, j);
    if (i < high)
        quickSortXcord(array, i, high);
}

void drawGrath()
{
    Point pt1;
    Point pt2;

    pt1.x = LENGHT / 2;
    pt1.y = 0;

    pt2.x = LENGHT / 2;
    pt2.y = HEIGHT;

    line(functionImg, pt1, pt2, Scalar(255, 255, 255));

    pt1.x = 0;
    pt1.y = HEIGHT / 2;

    pt2.x = LENGHT;
    pt2.y = HEIGHT / 2;

    line(functionImg, pt1, pt2, Scalar(255, 255, 255));
}

void drawFunctionValues()
{
    Point pt;

    for (float x = -X_LIMITS; x < X_LIMITS; x += 1 / X_SCALE)
    {
        pt.x = x * X_SCALE + LENGHT / 2;
        pt.y = -func(x) * Y_SCALE + HEIGHT / 2;

        functionImg.at<Vec3b>(pt) = Vec3b(255, 0, 0);
    }
}

void plotPoints(vector<individual> indVec)
{
    Point pt;

    for (int i = 0; i < indVec.size(); i++)
    {
        pt.x = indVec[i].x_cord * X_SCALE + LENGHT / 2;
        pt.y = -func(indVec[i].x_cord) * Y_SCALE + HEIGHT / 2;

        circle(functionImg, pt, 2, Scalar(0, 255, 0), FILLED);
    }

    pt.x = bestXcord * X_SCALE + LENGHT / 2;
    pt.y = -func(bestXcord) * Y_SCALE + HEIGHT / 2;

    circle(functionImg, pt, 2, Scalar(0, 0, 255), FILLED);
}

void decreaseHasImproved(list_t<Region> *regionList)
{
    for (int i = 0; i < NUMBER_OF_REGIONS; i++)
    {
        if (regionList->data->hasImproved > 0)
            regionList->data->hasImproved--;

        regionList = regionList->next;
    }
}

void calculatesFitness(vector<individual> &indVec, list_t<Region> *regionList)
{
    decreaseHasImproved(regionList);

    for (int i = 0; i < indVec.size(); i++)
    {
        if ((indVec[i].fitness = func(indVec[i].x_cord)) > indVec[i].region->best)
        {
            indVec[i].region->best = indVec[i].fitness;
            indVec[i].region->bestX = indVec[i].x_cord;
            indVec[i].region->hasImproved = 5;

            if (indVec[i].region->best > best)
            {
                best = indVec[i].region->best;
                bestXcord = indVec[i].x_cord;
            }
        }

        test++;
    }
}

list_t<Region> *createRegionList()
{
    list_t<Region> *current = new list_t<Region>;
    list_t<Region> *save;
    list_t<Region> *start;

    start = current;

    for (int i = 0; i < NUMBER_OF_REGIONS; i++)
    {
        current->data = new Region;
        current->data->best = -MAX_Y;
        current->data->quantIndvs = 0;
        current->data->startX = regionSize * i;
        current->data->endX = regionSize * (i + 1);
        current->data->hasImproved = 5;

        current->next = new list_t<Region>;

        //cout << current->data->startX << endl;

        save = current;
        current = current->next;
    }

    delete (current);
    save->next = nullptr;

    return start;
}

void resetList(list_t<Region> *start)
{
    list_t<Region> *current = start;

    for (int i = 0; i < NUMBER_OF_REGIONS; i++)
    {
        current->data = new Region;
        current->data->best = -MAX_Y;
        current->data->quantIndvs = 0;
        current->data->startX = regionSize * i;
        current->data->endX = regionSize * (i + 1);
        current->data->hasImproved = 5;
        current->data->mutChange = START_MUT;

        //cout << current->data->startX << endl;

        current = current->next;
    }
}

void drawRegions(list_t<Region> *regionList)
{
    Point pt1;
    Point pt2;

    pt1.y = 0;
    pt2.y = HEIGHT - 1;

    for (int i = 0; i < NUMBER_OF_REGIONS; i++)
    {
        pt1.x = regionSize * i * X_SCALE;
        pt2.x = pt1.x;

        line(functionImg, pt1, pt2, Scalar(0, 0, 255));
    }

    pt1.x = LENGHT - 1;
    pt2.x = pt1.x;
    line(functionImg, pt1, pt2, Scalar(0, 0, 255));
}

void mutation(vector<individual> &indvVec, list_t<Region> *regionList)
{
    float dif;

    for (int i = 0; i < NUMBER_OF_REGIONS; i++)
    {
        if (regionList->data->mutChange > MAX_MUT)
            regionList->data->mutChange = MAX_MUT;
        else if (regionList->data->mutChange < START_MUT)
            regionList->data->mutChange = START_MUT;

        if (regionList->data->hasImproved == 0)
            regionList->data->mutChange *= MUT_INCREASE_RATE;
        else
        {
            if (regionList->data->mutChange > START_MUT)
                regionList->data->mutChange = START_MUT;

            regionList->data->mutChange *= MUT_DECREASE_RATE;
        }

        regionList = regionList->next;
    }

    for (int i = 0; i < indvVec.size(); i++)
    {
        if (indvVec[i].x_cord == indvVec[i].region->bestX)
            continue;

        dif = (rand() % (int)(2 * indvVec[i].region->mutChange) - indvVec[i].region->mutChange) / 100.0;
        //dif -= indvVec[i].region->mutChange * 1000.0;
        //dif /= 1000.0;

        indvVec[i].x_cord += dif;

        if (indvVec[i].x_cord < -X_LIMITS || indvVec[i].x_cord > X_LIMITS)
        {
            indvVec[i].x_cord = indvVec[i].region->startX + (rand() % (100 * regionSize)) / 100;
            indvVec[i].x_cord -= X_LIMITS;
        }
    }
}

void eletism(vector<individual> &indVec)
{
    for (int i = 0; i < indVec.size(); i++)
    {
        if (indVec[i].fitness == indVec[i].region->best)
            continue;

        indVec[i].x_cord = (indVec[i].x_cord + indVec[i].region->bestX) / 2;
    }
}

int main()
{
    srand(time(0));

    String funcWindow = "function grath";
    namedWindow(funcWindow);

    int gen = 0;
    int totalgen = 0;
    int totalTests = 0;

    vector<individual> indVec;
    list_t<Region> *regionList;
    list_t<Region> *current;

    regionList = createRegionList();
    current = regionList;

    indVec.resize(NUMBER_OF_REGIONS * INDV_BY_REGION);

    cout.precision(6);

    cout << "number of indvs: " << NUMBER_OF_REGIONS * INDV_BY_REGION
         << " indvs by region: " << INDV_BY_REGION << endl;
    
    for (int i = 0; i < 5000; i++)
    {

        totalgen += gen;
        totalTests += test;

        gen = 0;
        test = 0;

        best = -MAX_Y;

        resetList(regionList);
        current = regionList;

        for (int j = 0; j < (NUMBER_OF_REGIONS * INDV_BY_REGION); j++)
        {
            if (!(j % INDV_BY_REGION) && j != 0)
            {
                current = current->next;
            }

            indVec[j].setInicialValues(current->data);
        }

        while (1)
        {
            functionImg.setTo(Scalar(0, 0, 0));

            drawGrath();
            drawFunctionValues();

            drawRegions(regionList);

            calculatesFitness(indVec, regionList);
            plotPoints(indVec);

            eletism(indVec);

            mutation(indVec, regionList);

            //imshow(funcWindow, functionImg);

            if (best > MAX_VALUE - ERROR)
            {
                //cout << best << "  ";
                cout << gen << "  ";
                cout << test << endl;

                break;
            }

            //waitKey(0);

            gen++;
        }
    }

    cout << "total gen: " << totalgen << " | total tests: " << totalTests << endl;

    return 0;
}