#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <bits/stdc++.h>

//g++ simple.cpp -o simple.o `pkg-config --cflags --libs opencv4`

#define HEIGHT 600
#define LENGHT 1800
#define MAX_Y 3.0
#define X_LIMITS 10
#define INDV_COUNT 10
#define GENOCIDE_PER 0.3
#define GENOCIDE_INTERVAL 20

using namespace std;
using namespace cv;

//globals:
const float X_SCALE = LENGHT / (2 * X_LIMITS);
const float Y_SCALE = HEIGHT / (MAX_Y * 2 * 1.2);

float MAX_MUT = 30;
int hasImproved = 5;

Mat functionImg(HEIGHT, LENGHT, CV_8UC3, Scalar(0, 0, 0));

float func(float x)
{
    return sin(3 * x) + cos(x) + sin(x * cos(x));
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

void inicializeIndv(float *indvs, float *fitness)
{
    for (int i = 0; i < INDV_COUNT; i++)
    {
        fitness[i] = -MAX_Y;
        indvs[i] = (float)(rand() % (2 * X_LIMITS) - X_LIMITS);
    }
}

void plotPoints(float *indvs)
{
    Point pt;

    for (int i = 0; i < INDV_COUNT; i++)
    {
        pt.x = indvs[i] * X_SCALE + LENGHT / 2;
        pt.y = -func(indvs[i]) * Y_SCALE + HEIGHT / 2;

        circle(functionImg, pt, 2, Scalar(0, 255, 0), FILLED);
    }
}

void calculatesFitness(float *indvs, float *fitness, float *best, int *bestIndex, int gen, float *bestFitHist, float *averageFitHist)
{
    float sum = 0;
    float lastBest = *best;

    for (int i = 0; i < INDV_COUNT; i++)
    {
        if ((fitness[i] = func(indvs[i])) > *best)
        {
            *best = fitness[i];
            *bestIndex = i;
        }
        sum += fitness[i];
    }

    bestFitHist[gen] = *best;
    averageFitHist[gen] = sum / INDV_COUNT;

    if (*best - lastBest < 0.01)
    {
        if (hasImproved > 0)
            hasImproved--;
    }
    else
        hasImproved = 5;
}

void simpleMutation(float *indv)
{

    if (MAX_MUT < 1)
        MAX_MUT = 1;
    else if (MAX_MUT > X_LIMITS * 50)
        MAX_MUT = X_LIMITS * 50;

    if (hasImproved == 0)
        MAX_MUT *= 1.1;
    else
    {
        if (MAX_MUT > 10)
            MAX_MUT = 10;

        MAX_MUT *= 0.95;
    }

    *indv += (rand() % (int)(2 * MAX_MUT) - MAX_MUT) / 100.0;
    if (*indv > X_LIMITS)
        *indv = X_LIMITS;
    else if (*indv < -X_LIMITS)
        *indv = -X_LIMITS;
}

void eletism(float *indvs, int bestIndex)
{

    for (int i = 0; i < INDV_COUNT; i++)
    {
        if (i == bestIndex)
            continue;

        indvs[i] = (indvs[i] + indvs[bestIndex]) / 2.0;
        simpleMutation(&indvs[i]);
    }
}

void plotInfoGrath(int gen, float *bestFitHist, float *averageFitHist)
{
    int maxLenght = 600;
    int maxHeight = HEIGHT / 2;
    int gap;

    static Mat infoGraph(maxHeight, maxLenght, CV_8UC1);
    infoGraph = Mat::zeros(infoGraph.size(), infoGraph.type());

    String windowName = "infoGraph";

    Point pt;
    Point ptAnt;

    for (int i = 1; i < maxLenght; i++)
    {
        pt.x = i;
        pt.y = maxHeight - bestFitHist[i * gen / maxLenght] * Y_SCALE;
        infoGraph.at<uchar>(pt) = 255;

        ptAnt.x = i - 1;
        ptAnt.y = maxHeight - averageFitHist[(i - 1) * gen / maxLenght] * Y_SCALE;
        pt.x = i;
        pt.y = maxHeight - averageFitHist[i * gen / maxLenght] * Y_SCALE;

        line(infoGraph, ptAnt, pt, 127);
    }

    namedWindow(windowName);

    imshow(windowName, infoGraph);
    waitKey(1);
}

void swap(float *a, float *b)
{
    float t = *a;
    *a = *b;
    *b = t;
}

void quickSort(float *arrayToSort, float *array, int low, int high)
{
    int i = low;
    int j = high;
    float pivot = array[(i + j) / 2];
    float temp;

    while (i <= j)
    {
        while (array[i] < pivot)
            i++;

        while (array[j] > pivot)
            j--;

        if (i <= j)
        {
            swap(&arrayToSort[i], &arrayToSort[j]);
            swap(&array[i], &array[j]);
            i++;
            j--;
        }
    }

    if (j > low)
        quickSort(arrayToSort, array, low, j);
    if (i < high)
        quickSort(arrayToSort, array, i, high);
}

void printIndvs(float *indvs, float *fitness)
{

    for (int i = 0; i < INDV_COUNT; i++)
    {
        cout << "indv( " << i << "): " << indvs[i] << " | " << fitness[i] << endl;
    }

    cout << "---------------------------------------" << endl;
}

void genocide(float *indvs, float *fitness, int *bestIndex)
{
    quickSort(indvs, fitness, 0, INDV_COUNT - 1);

    int quant = (int)INDV_COUNT * GENOCIDE_PER;

    cout << quant << endl;

    for (int i = 0; i < quant; i++)
    {
        fitness[i] = func(indvs[i]);
        indvs[i] = (float)(rand() % (2 * X_LIMITS) - X_LIMITS);
    }

    *bestIndex = INDV_COUNT - 1;
}

int main()
{
    srand(time(0));

    String funcWindow = "function grath";
    namedWindow(funcWindow);

    int gen = 0;
    int bestIndex = 0;
    int genocideInterval = GENOCIDE_INTERVAL;

    float best = -MAX_Y;
    float minAverage;

    float *indvs = new float[INDV_COUNT];
    float *fitness = new float[INDV_COUNT];
    float *bestFitHist = new float[10000];
    float *averageFitHist = new float[10000];

    inicializeIndv(indvs, fitness);
    calculatesFitness(indvs, fitness, &best, &bestIndex, gen, bestFitHist, averageFitHist);

    minAverage = averageFitHist[0];

    cout << "min: " << minAverage << endl;

    while (1)
    {
        functionImg.setTo(Scalar(0, 0, 0));

        drawGrath();
        drawFunctionValues();
        plotPoints(indvs);

        calculatesFitness(indvs, fitness, &best, &bestIndex, gen, bestFitHist, averageFitHist);
        eletism(indvs, bestIndex);

        plotInfoGrath(gen, bestFitHist, averageFitHist);
        imshow(funcWindow, functionImg);
        waitKey(100);

        if (averageFitHist[gen] < minAverage && genocideInterval < 0)
        {

            genocideInterval = GENOCIDE_INTERVAL;
            genocide(indvs, fitness, &bestIndex);
            cout << "genocide" << endl;

            MAX_MUT /= 4;

            waitKey(0);
        }

        gen++;
        genocideInterval--;
    }

    delete (indvs);
    delete (fitness);
    delete (bestFitHist);
    delete (averageFitHist);

    return 0;
}