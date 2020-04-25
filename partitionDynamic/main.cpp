#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <math.h>

#include "structs.hpp"
#include "individual.hpp"

//g++ *.cpp -o partitionClass.o `pkg-config --cflags --libs opencv4`

#define HEIGHT 600
#define LENGHT 1800

#define START_MUT 1
#define MUT_DECREASE_RATE 0.95
#define MUT_INCREASE_RATE 1.1

using namespace std;
using namespace cv;

//globals:
const float X_SCALE = LENGHT / (2.0 * X_LIMITS);
const float Y_SCALE = HEIGHT / (MAX_Y * 2 * 1.2);

float MUT_CHANGE = 1;
float bestXcord = 0;
float best = -MAX_Y;

int INDV_COUNT = 10;
int regionListSize;

Mat functionImg(HEIGHT, LENGHT, CV_8UC3, Scalar(0, 0, 0));

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
    float temp;

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

float func(float x)
{
    return (2 * cos(0.39 * x) + 5 * sin(0.5 * x) + 0.5 * cos(0.1 * x) + 10 * sin(0.7 * x) + 5 * sin(1 * x) + 5 * sin(0.35 * x)) / 5;
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
    for (int i = 0; i < regionListSize; i++)
    {
        if (regionList->data->hasImproved > 0)
        {
            cout << "reg posi: " << regionList->data->startX << endl;
            regionList->data->hasImproved--;
        }

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
    }
}

list_t<Region> *createRegionList()
{
    list_t<Region> *currnet = new list_t<Region>;
    list_t<Region> *save;
    list_t<Region> *start;

    start = currnet;

    for (int i = 0; i < INDV_COUNT; i++)
    {
        currnet->data = new Region;
        currnet->data->best = -MAX_Y;
        currnet->data->quantIndvs = 0;
        currnet->next = new list_t<Region>;
        currnet->data->hasImproved = 5;

        save = currnet;
        currnet = currnet->next;
    }

    delete (currnet);
    save->next = nullptr;

    return start;
}

void defineRegions(vector<individual> &indVec, list_t<Region> *regionList)
{
    quickSortXcord(indVec, 0, indVec.size() - 1);

    float cordDif;
    const float dist = 2.0 * X_LIMITS / INDV_COUNT;

    int begin = 0;
    bool concentredFlag = false;

    regionList->data->startX = 0;
    regionList->data->quantIndvs = 1;
    regionList->data->hasImproved = 5;

    regionListSize = 1;

    for (int i = 1; i < indVec.size(); i++)
    {
        cordDif = indVec[i].x_cord - indVec[i - 1].x_cord;
        indVec[i - 1].region = regionList->data;

        if (cordDif > dist || (concentredFlag && indVec[i].x_cord - indVec[i - 1].x_cord > dist / 1.5))
        {
            regionListSize++;
            regionList->data->endX = X_SCALE * (indVec[i - 1].x_cord + indVec[i].x_cord) / 2 + LENGHT / 2;
            regionList->data->mutChange = BEGIN_MUT_VAL;
            regionList->data->best = -MAX_Y;
            regionList->data->hasImproved = 5;

            if (regionList->next == nullptr)
            {
                regionList->next = new list_t<Region>;
                regionList->next->next = nullptr;
                regionList->next->data = new Region;
            }

            regionList->next->data->best = -MAX_Y;
            regionList->next->data->startX = regionList->data->endX;
            regionList = regionList->next;
            regionList->data->quantIndvs = 1;
            regionList->data->hasImproved = 5;

            begin = i;
        }
        else
            regionList->data->quantIndvs++;
    }

    regionList->data->endX = LENGHT - 1;

    indVec[indVec.size() - 1].region = regionList->data;
}

void drawRegions(list_t<Region> *regionList)
{
    Point pt1;
    Point pt2;

    pt1.y = 0;
    pt2.y = HEIGHT - 1;

    for (int i = 0; i < regionListSize; i++)
    {
        pt1.x = regionList->data->startX;
        pt2.x = pt1.x;
        line(functionImg, pt1, pt2, Scalar(0, 0, 255));

        regionList = regionList->next;
    }

    pt1.x = LENGHT - 1;
    pt2.x = pt1.x;
    line(functionImg, pt1, pt2, Scalar(0, 0, 255));
}

void mutation(vector<individual> &indvVec, list_t<Region> *regionList)
{
    int divIndex;
    int histIndex;

    float dif;
    float maxMut;

    for (int i = 0; i < regionListSize; i++)
    {
        if (regionList->data->mutChange > (maxMut = (regionList->data->endX - regionList->data->startX) / (X_SCALE * 2)))
            regionList->data->mutChange = START_MUT;
        else if (regionList->data->mutChange < START_MUT)
            regionList->data->mutChange = START_MUT;

        if (regionList->data->hasImproved == 0)
            regionList->data->mutChange *= MUT_INCREASE_RATE;
        else
        {
            regionList->data->mutChange = START_MUT;
            regionList->data->mutChange *= MUT_DECREASE_RATE;
        }

        if (i == regionListSize - 1)
        {
            cout << "reg: " << i << " | mut: " << regionList->data->mutChange << " | start: " << regionList->data->startX
                 << " |  end: " << regionList->data->endX << " | hasImproved: " << regionList->data->hasImproved << endl;
        }

        regionList = regionList->next;
    }

    for (int i = 0; i < indvVec.size(); i++)
    {
        if (indvVec[i].x_cord == indvVec[i].region->bestX)
            continue;

        dif = rand() % (int)(2 * indvVec[i].region->mutChange * 1000.0);
        dif -= indvVec[i].region->mutChange * 1000.0;
        dif /= 1000.0;

        indvVec[i].x_cord += dif;

        if (indvVec[i].x_cord < -X_LIMITS || indvVec[i].x_cord > X_LIMITS)
        {
            indvVec[i].x_cord = indvVec[i].region->startX + rand() % (indvVec[i].region->endX - indvVec[i].region->startX);
            indvVec[i].x_cord /= X_SCALE;
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

void equalizeRegionIndv(vector<individual> &indVec, list_t<Region> *regionList)
{
    list_t<Region> *current = regionList;

    for (int i = 0; i < regionListSize; i++)
    {
        while (current->data->quantIndvs < MIN_REGION_POP)
        {
            individual temp;
            float posiX;

            temp.fitness = -MAX_Y;
            temp.region = current->data;

            posiX = current->data->startX + rand() % (current->data->endX - current->data->startX);
            posiX /= X_SCALE;
            posiX -= X_LIMITS;

            temp.x_cord = posiX;

            indVec.push_back(temp);
            current->data->quantIndvs++;
        }
        current = current->next;
    }
}

int main()
{
    srand(time(0));

    String funcWindow = "function grath";
    namedWindow(funcWindow);

    int gen = 0;

    vector<individual> indVec;
    list_t<Region> *regionList;

    indVec.resize(INDV_COUNT);

    regionList = createRegionList();

    cout.precision(4);
    while (1)
    {
        functionImg.setTo(Scalar(0, 0, 0));

        drawGrath();
        drawFunctionValues();

        if (!(gen % 150))
        {
            defineRegions(indVec, regionList);
            equalizeRegionIndv(indVec, regionList);
        }

        drawRegions(regionList);

        calculatesFitness(indVec, regionList);
        plotPoints(indVec);

        eletism(indVec);

        mutation(indVec, regionList);

        imshow(funcWindow, functionImg);
        waitKey(0);
        gen++;
        cout << "gen: " << gen << endl;
    }

    return 0;
}