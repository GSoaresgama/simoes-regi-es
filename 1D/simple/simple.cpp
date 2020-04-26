#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <bits/stdc++.h>

//g++ simple.cpp -o simple.o `pkg-config --cflags --libs opencv4`

#define HEIGHT 600
#define LENGHT 1800
#define MAX_Y 6.0
#define X_LIMITS 75
#define INDV_COUNT 100
#define START_MUT 50
#define MUT_DECREASE_RATE 0.80
#define MUT_INCREASE_RATE 2

#define MAX_VALUE 4.76224
#define ERROR 0.01

#define MAX_GEN 10000

using namespace std;
using namespace cv;

//globals:
const float X_SCALE = LENGHT / (2 * X_LIMITS);
const float Y_SCALE = HEIGHT / (MAX_Y * 2 * 1.2);

const float MAX_MUT = 2 * X_LIMITS * 100;

float MUT = START_MUT;

int hasImproved = 5;
int test = 0;

Mat functionImg(HEIGHT, LENGHT, CV_8UC3, Scalar(0, 0, 0));

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

void inicializeIndv(float *indvs, float *fitness)
{
    for (int i = 0; i < INDV_COUNT; i++)
    {
        fitness[i] = -MAX_Y;
        indvs[i] = (float)(rand() % (2 * X_LIMITS) - X_LIMITS);
    }
}

void plotPoints(float *indvs, int bestIndex)
{
    Point pt;

    for (int i = 0; i < INDV_COUNT; i++)
    {
        pt.x = indvs[i] * X_SCALE + LENGHT / 2;
        pt.y = -func(indvs[i]) * Y_SCALE + HEIGHT / 2;

        circle(functionImg, pt, 2, Scalar(0, 255, 0), FILLED);
    }

    pt.x = indvs[bestIndex] * X_SCALE + LENGHT / 2;
    pt.y = -func(indvs[bestIndex]) * Y_SCALE + HEIGHT / 2;

    circle(functionImg, pt, 2, Scalar(0, 0, 255), FILLED);
}

void calculatesFitness(float *indvs, float *fitness, float *best, int *bestIndex, int gen, float *bestFitHist, float *averageFitHist)
{
    float sum = 0;
    float lastBest = *best;

    if (hasImproved > 0)
        hasImproved--;

    for (int i = 0; i < INDV_COUNT; i++)
    {
        if ((fitness[i] = func(indvs[i])) > *best)
        {
            *best = fitness[i];
            *bestIndex = i;
            hasImproved = 5;
        }
        sum += fitness[i];
        test++;
    }

    bestFitHist[gen] = *best;
    averageFitHist[gen] = sum / INDV_COUNT;
}

void simpleMutation(float *indv)
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

    *indv += (rand() % (int)(2 * MUT) - MUT) / 100.0;
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

int main()
{
    srand(time(0));

    String funcWindow = "function grath";
    namedWindow(funcWindow);

    int gen = 0;

    int totalGen = 0;
    int totalTests = 0;

    int bestIndex = 0;
    float best = -MAX_Y;

    float *indvs = new float[INDV_COUNT];
    float *fitness = new float[INDV_COUNT];
    float *bestFitHist = new float[MAX_GEN];
    float *averageFitHist = new float[MAX_GEN];

    cout.precision(6);

    for (int i = 0; i < 500; i++)
    {
        totalGen += gen;
        totalTests += test;
        gen = 0;
        test = 0;
        MUT = START_MUT;
        hasImproved = 5;
        best = -MAX_Y;
        bestIndex = 0;

        inicializeIndv(indvs, fitness);
        //calculatesFitness(indvs, fitness, &best, &bestIndex, gen, bestFitHist, averageFitHist);

        while (gen < MAX_GEN)
        {
            functionImg.setTo(Scalar(0, 0, 0));

            drawGrath();
            drawFunctionValues();

            calculatesFitness(indvs, fitness, &best, &bestIndex, gen, bestFitHist, averageFitHist);
            plotPoints(indvs, bestIndex);

            eletism(indvs, bestIndex);

            //plotInfoGrath(gen, bestFitHist, averageFitHist);
            imshow(funcWindow, functionImg);

            if (best > MAX_VALUE - ERROR)
            {
                //cout << best << " , ";
                cout << gen << " , ";
                cout << test << endl;

                break;
            }

            waitKey(1);

            gen++;
        } /* code */
    }

    cout << "total gen: " << totalGen << " | total tests: " << totalTests << endl;

    return 0;
}