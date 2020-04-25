#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <math.h>

#define INDV_PER_DIV 5
#define MAX_MUT 6
#define SCALE 100
#define SCALE_Y_AXIS 50
#define MUT_CHANGE 1
#define GEN_GAP 10
#define VERY_SMAL_NUMBER 0.001

#define HEIGHT 600
#define LENGHT 1800

//g++ partitio.cpp -o partition.o `pkg-config --cflags --libs opencv4`

using namespace std;
using namespace cv;

const int divisionQuant = 5;
const int divisionSize = LENGHT / divisionQuant;

bool checkAllConverged();
void drawFitGraph();

int maxi[divisionQuant];
int gen = 0;

bool allConverged = false;
bool resetMut = false;
bool maxMut[divisionQuant];

float maxAllFit;
float maxAllFitPosi;
float lastMax[divisionQuant];
float maxFit[divisionQuant];
float maxPosi[divisionQuant];
float _MUT_CHANGE[divisionQuant];
float indvVals[divisionQuant][INDV_PER_DIV];
float maxAllFitHist[10000];

float func(float x)
{
    return (2 * cos(0.39 * x) + 5 * sin(0.5 * x) + 0.5 * cos(0.1 * x) + 10 * sin(0.7 * x) + 5 * sin(1 * x) + 5 * sin(0.35 * x)) / 5;
}

void printVector(Mat image)
{
    int i;
    int j;
    int divisionIndex;
    int pixelX;
    int pixelY;

    float x;

    Point pt;

    image = cv::Mat::zeros(image.size(), image.type());

    String windowName = "create_image";

    //x axis
    for (i = 0; i < LENGHT; i++)
    {
        pt.x = i;
        pt.y = HEIGHT / 2;

        image.at<Vec3b>(pt) = Vec3b(255, 255, 255);
    }

    //y axis
    for (i = 0; i < HEIGHT; i++)
    {
        pt.x = LENGHT / 2;
        pt.y = i;

        image.at<Vec3b>(pt) = Vec3b(255, 255, 255);
    }

    //division axis
    if (!allConverged)
    {

        for (divisionIndex = 0; divisionIndex <= divisionQuant; divisionIndex++)
        {
            pt.x = divisionSize * divisionIndex;

            if (divisionIndex == divisionQuant)
                pt.x--;

            for (i = 0; i < HEIGHT; i++)
            {
                pt.y = i;
                image.at<Vec3b>(pt) = Vec3b(255, 0, 0);
            }
        }
    }

    //function
    for (x = -LENGHT / 2; x < LENGHT / 2; x += 1)
    {
        pt.x = x + LENGHT / 2.0;
        pt.y = HEIGHT / 2 - (func(x / SCALE) * SCALE_Y_AXIS);

        image.at<Vec3b>(pt) = Vec3b(255, 255, 255);

        //pixelX = (x + LENGHT / 2.0);
        //pixelY = (HEIGHT / 2 - (int)(func(x / SCALE) * 50));
    }

    //points
    for (i = 0; i < divisionQuant; i++)
    {
        for (j = 0; j < INDV_PER_DIV; j++)
        {
            if (j == maxi[i])
                continue;

            pt.x = indvVals[i][j] * SCALE + LENGHT / 2;
            pt.y = HEIGHT / 2 - (func(indvVals[i][j]) * SCALE_Y_AXIS);

            circle(image, pt, 2, Scalar(0, 255, 0), -1);
        }

        //cout << "indice: " << i << " valor max(" << indvVals[i][maxi[i]] << "): " << func(indvVals[i][maxi[i]]) << endl;

        pt.x = indvVals[i][maxi[i]] * SCALE + LENGHT / 2;
        pt.y = HEIGHT / 2 - (func(indvVals[i][maxi[i]]) * SCALE_Y_AXIS);

        circle(image, pt, 3, Scalar(0, 0, 255), -1);
    }

    imshow(windowName, image);
    cv::waitKey(1);
}

void changeMut()
{
    int divIndex;
    int histIndex;

    float dif;

    for (divIndex = 0; divIndex < divisionQuant; divIndex++)
    {
        dif = (lastMax[divIndex] > maxFit[divIndex]) ? lastMax[divIndex] - maxFit[divIndex] : maxFit[divIndex] - lastMax[divIndex];

        cout << "dif: " << dif << " | mut: " << _MUT_CHANGE[divIndex] << endl;

        if (!maxMut[divIndex] && dif < VERY_SMAL_NUMBER && _MUT_CHANGE[divIndex] < divisionSize * MAX_MUT / 2)
        {
            if (_MUT_CHANGE[divIndex] < 0.5)
                _MUT_CHANGE[divIndex] = 0.5;

            _MUT_CHANGE[divIndex] *= 1.05;
        }
        else
        {
            if (_MUT_CHANGE[divIndex] > divisionSize * MAX_MUT / 2)
                maxMut[divIndex] = true;

            //cout << "reduzindo: " << divIndex << " | " << _MUT_CHANGE[divIndex] << endl;
            _MUT_CHANGE[divIndex] *= 0.95;

            if (_MUT_CHANGE[divIndex] > 0.5)
                _MUT_CHANGE[divIndex] = 0.5;
        }

        if (_MUT_CHANGE[divIndex] <= 0.001)
            _MUT_CHANGE[divIndex] = 0.1;
    }

    if (checkAllConverged() && resetMut)
    {
        cout << "terminou" << endl;
        drawFitGraph();
        waitKey(0);
        exit(1);
    }
}

void evaluate()
{
    int i;
    int j;

    for (i = 0; i < divisionQuant; i++)
    {
        for (j = 0; j < INDV_PER_DIV; j++)
        {
            if (func(indvVals[i][j]) >= maxFit[i])
            {
                lastMax[i] = maxFit[i];
                maxFit[i] = func(indvVals[i][j]);
                maxPosi[i] = indvVals[i][j];
                maxi[i] = j;
                //cout << "new max: " << i << endl;
            }
        }

        if (maxFit[i] > maxAllFit)
        {
            maxAllFitPosi = maxPosi[i];
            maxAllFit = maxFit[i];
        }
        maxAllFitHist[gen] = maxAllFit;
    }
}

void eletismPart()
{
    int i;
    int j;

    for (i = 0; i < divisionQuant; i++)
    {
        for (j = 0; j < INDV_PER_DIV; j++)
        {
            if (j == maxi[i])
                continue;

            indvVals[i][j] = (indvVals[i][j] + maxPosi[i]) / 2;
            indvVals[i][j] += _MUT_CHANGE[i] * (float)(rand() % MAX_MUT - MAX_MUT / 2) / 100;

            while (indvVals[i][j] > (float)(divisionSize * (i + 1) - LENGHT / 2) / SCALE || indvVals[i][j] < (float)(divisionSize * i - LENGHT / 2) / SCALE)
            {
                indvVals[i][j] = (rand() % divisionSize + divisionSize * i - LENGHT / 2) / SCALE;
            }
        }
    }
}

void eletism()
{
    int i;
    int j;

    for (i = 0; i < divisionQuant; i++)
    {
        for (j = 0; j < INDV_PER_DIV; j++)
        {
            if (j == maxi[i])
                continue;

            indvVals[i][j] = (indvVals[i][j] + maxAllFitPosi) / 2;
            indvVals[i][j] += _MUT_CHANGE[i] * (float)(rand() % MAX_MUT - MAX_MUT / 2) / 100;

            while (indvVals[i][j] > LENGHT / (2 * SCALE) || indvVals[i][j] < -LENGHT / (2 * SCALE))
            {
                indvVals[i][j] = (rand() % LENGHT - LENGHT / 2) / SCALE;
            }
        }
    }
}

bool checkAllConverged()
{
    for (int i = 0; i < divisionQuant; i++)
    {
        if (!maxMut[i])
        {
            cout << "nao convergiu: " << i << " | " << _MUT_CHANGE[i] << " | " << maxMut[i] << endl;
            return false;
        }
    }

    return true;
}

void drawFitGraph()
{
    int maxLenght = 600;
    int maxHeight = HEIGHT / 2;
    int gap;

    static Mat fitGraph(maxHeight, maxLenght, CV_8UC1);
    fitGraph = Mat::zeros(fitGraph.size(), fitGraph.type());

    String windowName = "fitGraph";

    Point pt;

    for (int i = 0; i < maxLenght; i++)
    {
        pt.x = i;
        pt.y = maxHeight - maxAllFitHist[i * gen / maxLenght] * SCALE_Y_AXIS;
        fitGraph.at<uchar>(pt) = 255;
    }

    namedWindow(windowName);

    imshow(windowName, fitGraph);
    waitKey(1);
}

int main()
{
    std::cout << std::fixed;
    std::cout << std::setprecision(6);

    int i;
    int j;

    //const int BEGIN_VAL_LIMIT = LENGHT / (divisionQuant * (2 * SCALE)) + 1;

    Mat image(HEIGHT, LENGHT, CV_8UC3);

    String windowName = "create_image";

    srand(time(0));

    namedWindow(windowName);

    //inicialize values
    for (i = 0; i < divisionQuant; i++)
    {
        maxMut[i] = false;
        maxi[i] = 0;
        maxPosi[i] = -100;
        maxFit[i] = -100;
        maxAllFit = -100;

        for (j = 0; j < INDV_PER_DIV; j++)
            indvVals[i][j] = (float)(rand() % divisionSize + divisionSize * i - LENGHT / 2) / SCALE;
    }

    while (1)
    {
        printVector(image);
        //avalia

        evaluate();

        if (!allConverged)
            eletismPart();
        else
            eletism();

        gen++;

        if (!(gen % GEN_GAP))
        {
            changeMut();
            if (!allConverged)
            {
                allConverged = checkAllConverged();
            }
            drawFitGraph();
        }
        if (!resetMut && allConverged)
        {
            resetMut = true;
        }
    }
}