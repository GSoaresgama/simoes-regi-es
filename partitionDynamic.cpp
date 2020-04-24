#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <vector>
#include <bits/stdc++.h>

//g++ partitionDynamic.cpp -o partitionDynamic.o `pkg-config --cflags --libs opencv4`

#define HEIGHT 600
#define LENGHT 1800
#define MAX_Y 3.0
#define X_LIMITS 10

using namespace std;
using namespace cv;

//globals:
const float X_SCALE = LENGHT / (2.0 * X_LIMITS);
const float Y_SCALE = HEIGHT / (MAX_Y * 2 * 1.2);

int INDV_COUNT = 10;
int BASE_REGION_POP = 5;

Mat functionImg(HEIGHT, LENGHT, CV_8UC3, Scalar(0, 0, 0));

float func(float x)
{
    return sin(3 * x) + cos(x) + sin(x * cos(x));
}

void swap(float *a, float *b)
{
    float t = *a;
    *a = *b;
    *b = t;
}
void quickSort(vector<float> &arrayToSort, vector<float> &arrayParam, int low, int high)
{
    int i = low;
    int j = high;
    float pivot = arrayParam[(i + j) / 2];
    float temp;

    while (i <= j)
    {
        while (arrayParam[i] < pivot)
            i++;

        while (arrayParam[j] > pivot)
            j--;

        if (i <= j)
        {
            swap(&arrayToSort[i], &arrayToSort[j]);
            swap(&arrayParam[i], &arrayParam[j]);
            i++;
            j--;
        }
    }

    if (j > low)
        quickSort(arrayToSort, arrayParam, low, j);
    if (i < high)
        quickSort(arrayToSort, arrayParam, i, high);
}

void printIndvs(vector<float> indvs, vector<float> fitness)
{

    for (int i = 0; i < indvs.size(); i++)
    {
        cout << "indv( " << i << "): " << indvs[i] << " | " << fitness[i] << endl;
    }

    cout << "---------------------------------------" << endl;
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

void inicializeIndv(vector<float> &indvs, vector<float> &fitness)
{

    const float dist = 2.0 * X_LIMITS / INDV_COUNT;

    for (int i = 0; i < INDV_COUNT; i++)
    {
        fitness.push_back(-MAX_Y);
        indvs.push_back((rand() % (2 * X_LIMITS * 100) - X_LIMITS * 100) / 100.0);
    }
}

void drawSpeciesRegionLines(vector<int> speciesRegionLines)
{
    Point pt1;
    Point pt2;

    for (int i = 0; i < speciesRegionLines.size(); i++)
    {
        pt1.x = speciesRegionLines[i];
        pt1.y = 0;

        pt2.x = pt1.x;
        pt2.y = HEIGHT - 1;

        line(functionImg, pt1, pt2, Scalar(0, 0, 255));
    }
}

void defineSpeciesRegions(vector<float> &indvs, vector<float> &fitness, vector<int> &speciesIndex, vector<int> &speciesRegionLines)
{
    bool concentredFlag = false;

    int begin;

    const float dist = 2.0 * X_LIMITS / indvs.size();

    Point pt1;
    Point pt2;

    //sort by position on X-AXIS
    quickSort(fitness, indvs, 0, indvs.size() - 1);
    //printIndvs(indvs, fitness);

    speciesIndex.clear();
    speciesRegionLines.clear();

    speciesIndex.push_back(0);
    speciesRegionLines.push_back(0);

    begin = 0;
    for (int i = 1; i < indvs.size(); i++)
    {
        if ((indvs[i] - indvs[begin]) /*/ (i - begin)*/ > dist || (concentredFlag && indvs[i] - indvs[begin] > dist / 1.5))
        {
            //cout << "nova especie entre: " << i - 1 << " e " << i << endl;
            speciesIndex.push_back(i - 1);
            begin = i;

            pt1.x = (indvs[i] + indvs[i - 1]) / 2 * X_SCALE + LENGHT / 2;

            speciesRegionLines.push_back(pt1.x);
        }
        else if (indvs[i] - indvs[i - 1] < 0.5 * dist / (i - begin))
            concentredFlag = true;
        else
            concentredFlag = false;
    }

    speciesIndex.push_back(indvs.size() - 1);
    speciesRegionLines.push_back(LENGHT - 1);

    drawSpeciesRegionLines(speciesRegionLines);
}

void plotPoints(vector<float> indvs)
{
    Point pt;

    for (int i = 0; i < indvs.size(); i++)
    {
        pt.x = indvs[i] * X_SCALE + LENGHT / 2;
        pt.y = -func(indvs[i]) * Y_SCALE + HEIGHT / 2;

        circle(functionImg, pt, 2, Scalar(0, 255, 0), FILLED);

        if (i >= INDV_COUNT)
            circle(functionImg, pt, 3, Scalar(255, 255, 0), FILLED);
    }
}

void calculatesFitness(vector<float> indvs, vector<float> &fitness, vector<float> &best, vector<int> &bestIndex, int gen, vector<int> &speciesIndex, vector<int> &hasImproved)
{
    int interval = BASE_REGION_POP;

    bool hasImprovFlag;

    for (int i = 0, j = 0; i < indvs.size(); i++)
    {

        if (!(i % interval) && i != 0)
            j++;

        if (!hasImprovFlag && hasImproved[j] > 0)
            hasImproved[j]--;

        hasImprovFlag = false;

        if ((fitness[i] = func(indvs[i])) > best[j])
        {
            //cout << j << " melhorou"
            //     << "has improved: " << hasImproved[j] << endl;
            best[j] = fitness[i];
            bestIndex[j] = i;
            hasImproved[j] = 5;

            hasImprovFlag = true;
        }
        //cout << j << " has improved: " << hasImproved[j] << endl;
    }
}

void simpleMutation(float &indv, int hasImproved, float &MAX_MUT, int size)
{
    const float dist = 2.0 * X_LIMITS / BASE_REGION_POP;

    float dif;

    MAX_MUT = 5;
    dif = (rand() % (int)(2 * MAX_MUT) - MAX_MUT) / 100.0;

    indv += dif;

    if (indv > X_LIMITS)
        indv = X_LIMITS;
    else if (indv < -X_LIMITS)
        indv = -X_LIMITS;
}

void eletism(vector<float> &indvs, vector<int> &bestIndex, vector<int> &speciesIndex, vector<int> hasImproved, vector<float> &MAX_MUT)
{
    int interval = BASE_REGION_POP;

    for (int i = 0, j = 0; i < indvs.size(); i++)
    {
        if (!(i % interval) && i != 0)
            j++;

        //cout << "index of the best(" << j << "): " << bestIndex[j] << " | posiX: " << indvs[bestIndex[j]] << endl;

        if (i == bestIndex[j])
            continue;

        indvs[i] = (indvs[i] + indvs[bestIndex[j]]) / 2.0;

        simpleMutation(indvs[i], hasImproved[j], MAX_MUT[j], indvs.size());
    }
}

void equalizeRegionsPopulation(vector<int> &speciesIndex, vector<int> &speciesRegionsLines, vector<float> &indvs, vector<float> &fitness)
{
    int indvBySpecies;
    int regionSizeBySpecies;
    int initialQuant = indvs.size();
    float indv;

    for (int i = 1; i < speciesIndex.size(); i++)
    {
        indvBySpecies = speciesIndex[i] - speciesIndex[i - 1];
        if (i == 1)
            indvBySpecies++;

        // cout << "especie(" << i - 1 << ")"
        //      << "quant indv: " << indvBySpecies << endl;

        regionSizeBySpecies = speciesRegionsLines[i] - speciesRegionsLines[i - 1];
        //cout << "reg size: " << regionSizeBySpecies << "anti: " << speciesRegionsLines[i - 1] << endl;
        while (indvBySpecies < BASE_REGION_POP)
        {
            indv = rand() % regionSizeBySpecies - regionSizeBySpecies / 2;
            indv += (speciesRegionsLines[i] + speciesRegionsLines[i - 1]) / 2;
            indv /= X_SCALE;
            indv -= X_LIMITS;

            //cout << indv << endl;

            indvs.push_back(indv);
            fitness.push_back(func(indv));
            indvBySpecies++;
        }
    }

    quickSort(fitness, indvs, 0, indvs.size() - 1);
    printIndvs(indvs, fitness);
}

int main()
{
    srand(time(0));

    String funcWindow = "function grath";
    namedWindow(funcWindow);

    int gen = 0;

    vector<float> best;

    vector<float> indvs;
    vector<float> fitness;
    vector<float> MAX_MUT;
    float *bestFitHist = new float[10000];

    vector<int> bestIndex;
    vector<int> speciesIndex;
    vector<int> speciesRegionsLines;
    vector<int> hasImproved;

    inicializeIndv(indvs, fitness);

    defineSpeciesRegions(indvs, fitness, speciesIndex, speciesRegionsLines);
    equalizeRegionsPopulation(speciesIndex, speciesRegionsLines, indvs, fitness);

    best.resize(speciesIndex.size());
    bestIndex.resize(speciesIndex.size());
    MAX_MUT.resize(speciesIndex.size());
    //printIndvs(indvs, fitness);

    best.assign(best.size(), -MAX_Y);
    hasImproved.assign(speciesIndex.size(), BASE_REGION_POP);
    MAX_MUT.assign(speciesIndex.size(), 1);

    calculatesFitness(indvs, fitness, best, bestIndex, gen, speciesIndex, hasImproved);

    while (1)
    {
        functionImg.setTo(Scalar(0, 0, 0));

        drawGrath();
        drawFunctionValues();
        drawSpeciesRegionLines(speciesRegionsLines);

        plotPoints(indvs);
        calculatesFitness(indvs, fitness, best, bestIndex, gen, speciesIndex, hasImproved);
        eletism(indvs, bestIndex, speciesIndex, hasImproved, MAX_MUT);

        imshow(funcWindow, functionImg);
        waitKey(0);
        gen++;
    }

    delete (bestFitHist);

    return 0;
}