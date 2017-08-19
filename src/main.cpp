//
//  main.cpp
//  RectanglesAndHoles
//
//  Created by Anton Logunov on 5/21/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include <iostream>

#include "plane.h"
#include "circle_plane.h"
#include "rects_and_holes.h"


#include <string>
#include <cstring>
#include <vector>
#include <cmath>
#include <sstream>
#include <fstream>
#include <limits>
#include <iostream>
#include <algorithm>
#include <sys/time.h>

using namespace std;

#ifdef LOCAL
#    define MAX_TIME 6.9
#    define FREQUENCY 3013616601.
#else
#    define MAX_TIME 9.8
#    define FREQUENCY 3600170865.
#endif

#ifdef LOCAL
#    define BP() callback()
#    define PZ() callback(true)
#else
#    define BP() {}
#    define PZ() {}
#endif

#ifdef __amd64
__inline__ uint64_t rdtsc() { uint64_t a, d; __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d)); return (d<<32) | a; }
#else
__inline__ uint64_t rdtsc() { uint64_t x; __asm__ volatile ("rdtsc" : "=A" (x)); return x; }
#endif

#define CPU_TIME
#if defined CPU_TIME && defined LOCAL
#define TICKS() clock()
#define CLOCK_SCALE CLOCKS_PER_SEC
#else
#define TICKS() rdtsc()
#define CLOCK_SCALE FREQUENCY
#endif

uint32_t seed = 123456789; uint32_t nextInt() { return (seed = 1664525 * seed + 1013904223); } double nextDouble() { return (1./~((uint32_t)0)) * nextInt(); } uint32_t nextInt(int m) { return m * nextDouble(); }
struct Clock { uint64_t startTime, elapsedTime; Clock() { start(); } void start() { startTime = TICKS(); elapsedTime = 0; }; uint64_t update() { uint64_t absoluteTime = TICKS(); elapsedTime += absoluteTime - startTime; startTime = absoluteTime; return elapsedTime; } };
Clock clock_;
struct Timer { uint64_t start; double duration; Timer(double duration) : duration(duration) { start = clock_.update(); } Timer() : duration(MAX_TIME) { clock_.start(); start = clock_.update(); } double left() { return 1. - spent(); }; double spent() { return (clock_.update() - start) / (double)CLOCK_SCALE / duration; }; };
void printFrequency() { uint64_t endClock, startClock = rdtsc(); timeval tv, tv2; gettimeofday(&tv, NULL); do { gettimeofday(&tv2, NULL); } while (tv2.tv_sec - tv.tv_sec + (tv2.tv_usec - tv.tv_usec) * 1e-6 < 1.); endClock = rdtsc(); cerr << "FEQUENCY " << endClock-startClock << endl; }

#define MAXRECT 1000
#define MAXPTF 1000
#define LOCK_RATE .55
#define ADD_RANDOMNESS 200
#define PARALLEL_RUN 15

struct Move
{
    int type;
    int r;
    int p;
};

int N;
int X[MAXRECT];
int Y[MAXRECT];
int SX[MAXRECT];
int SY[MAXRECT];
int D[MAXRECT];
bool placed[MAXRECT];
double OR[MAXRECT];
double ORR[MAXRECT];
int diag[MAXRECT];
int heur[MAXRECT];
int rectIds[2*MAXRECT];
int nextRectIds[2*MAXRECT];
int totalDiag;
int remainingDiag;
int holes;
int minId;
double area;
double holesArea;
Move possibleMove;
vector<Move> currentMoves;
vector<Move> bestMoves[PARALLEL_RUN];
int currentCopy;
vector<int> ret;
int iter0;
int iter1;
int lockIter;
double alphaCoef;
double betaCoef;
double progress;

#define ROTATE 2048
#define ROTATEMASK (ROTATE-1)

int xr;
int yr;
int xl;
int yl;
int sides[4][MAXRECT];
int nbSide[4];
int sidesSize[4];
int P;
int PY[MAXPTF];
int PX[MAXPTF];
int pyminleft;
int pyminright;
int areaRect[MAXRECT];
int nbAreaRect;
double bestScoreEver;
double bestScore[PARALLEL_RUN];
double score;

#define ADDL0  0
#define ADDL1  1
#define ADDR0  2
#define ADDR1  3
#define PLACE0 4
#define PLACE1 5
#define SKIP   10

struct DiagSorter
{
    bool operator()(int l, int r) const
    {
        return diag[l] > diag[r];
    }
};

struct OrientationSorter
{
    bool operator()(int l, int r) const
    {
        return OR[l] < OR[r];
    }
};

struct RevOrientationSorter
{
    bool operator()(int l, int r) const
    {
        return ORR[l] > ORR[r];
    }
};

struct RectIdSorter
{
    bool operator()(int l, int r)
    {
        int hl = heur[l & ROTATEMASK] + .04 * (SY[l & ROTATEMASK] - SX[l & ROTATEMASK]);
        int hr = heur[r & ROTATEMASK] + .04 * (SY[r & ROTATEMASK] - SX[r & ROTATEMASK]);
        return hl > hr;
    }
};

template <bool x, bool inc>
struct CoordinateSorter
{
    bool operator()(int l, int r) const
    {
        if (x)
        {
            if (inc)
                return X[l] > X[r];
            return X[l] < X[r];
        }
        else
        {
            if (inc)
                return Y[l] > Y[r];
            return Y[l] < Y[r];
        }
    }
};


void init0()
{
    currentCopy = -1;
    nextInt();
    lockIter = 0;
    ret.resize(3*N);
    bestScoreEver = 0;
    memset(bestScore, 0, sizeof(bestScore));
    memset(D, 0, sizeof(X));
    totalDiag = 0;
    for (int r = 0; r < N; ++r)
    {
        diag[r] = (int)(sqrt(SX[r]*SX[r] + SY[r]*SY[r]));
        OR[r] = max(SX[r] / (double)SY[r], SY[r] / (double)SX[r]);
        rectIds[r] = r;
        rectIds[r+N] = r + ROTATE;
        totalDiag += diag[r];
    }
    
    for (int r = 0; r < N; ++r)
        if (SX[r] > SY[r])
        {
            swap(SX[r], SY[r]);
            D[r] = 1-D[r];
        }
    for (int r = 0; r < N; ++r)
        heur[r] = -.0018*(.8 * diag[r] + .2 * SY[r])*(.8 * diag[r] + .2 * SY[r]) - .075*(SY[r] - SX[r]);
    stable_sort(rectIds, rectIds+2*N, RectIdSorter());
}

void init1()
{
    for (int r = 0; r < N; ++r)
        if (SX[r] > SY[r])
        {
            swap(SX[r], SY[r]);
            D[r] = 1-D[r];
        }
    currentCopy = (currentCopy + 1) % PARALLEL_RUN;
    lockIter = min((int)bestMoves[currentCopy].size()-1, (int)(LOCK_RATE * progress*progress * N));
    while (lockIter < (int)bestMoves[currentCopy].size()-1 && bestMoves[currentCopy][lockIter].type > ADDR1)
        lockIter++;
    remainingDiag = totalDiag;
    P = 0;
    pyminleft = 0;
    pyminright = 0;
    memset(X, 0, sizeof(X));
    memset(Y, 0, sizeof(X));
    memset(placed, 0, sizeof(placed));
    holes = 1;
    holesArea = 0;
    for (int i = 0; i < 2*N; ++i)
        nextRectIds[i] = i+1;
    minId = 0;
    alphaCoef = nextDouble();
    
    if (progress < .8)
        betaCoef = nextInt(100);
    else
        betaCoef = numeric_limits<int>::min();
}

void init2(bool random)
{
    for (int r = 0; r < N; ++r)
        ORR[r] = max(SX[r] / (double)SY[r], SY[r] / (double)SX[r]) + (random * .05*nextDouble());
    area = holesArea;
    memset(nbSide, 0, sizeof(nbSide));
    memset(sidesSize, 0, sizeof(sidesSize));
    nbAreaRect = 0;
}

void getMoves()
{
    int p = -1;
    int bestPY = 1e6;
    for (int i = 1; i < P-1; ++i)
        if (PY[i] < PY[i-1] && PY[i] < PY[i+1])
        {
            if (PY[i] < bestPY)
            {
                bestPY = PY[i];
                p = i;
            }
        }
    
    int previous = 0;
    if (p != -1)
    {
        if (iter1 == lockIter)
        {
            for (int i = 0; i < 50; ++i)
            {
                int r = nextInt(N);
                if (!placed[r])
                {
                    int f = nextInt(2);
                    if (PY[p-1] < PY[p+1])
                    {
                        if (f)
                        {
                            if (PX[p+1]-SX[r] > PX[p-1] && PX[p+1]-SX[r] < PX[p] && PY[p+1] != PY[p-1]+SY[r])
                            {
                                possibleMove = {PLACE0, r, p};
                                return;
                            }
                        }
                        else
                        {
                            if (PX[p+1]-SY[r] > PX[p-1] && PX[p+1]-SY[r] < PX[p] && PY[p+1] != PY[p-1]+SX[r])
                            {
                                possibleMove = {PLACE1, r, p};
                                return;
                            }
                        }
                    }
                    else
                    {
                        if (f)
                        {
                            if (PX[p]+SX[r] < PX[p+2] && PX[p]+SX[r] > PX[p+1] && PY[p-1] != PY[p+1]+SY[r])
                            {
                                possibleMove = {PLACE0, r, p};
                                return;
                            }
                        }
                        else
                        {
                            if (PX[p]+SY[r] < PX[p+2] && PX[p]+SY[r] > PX[p+1] && PY[p-1] != PY[p+1]+SX[r])
                            {
                                possibleMove = {PLACE1, r, p};
                                return;
                            }
                        }
                    }
                }
                else
                {
                    nextRectIds[previous] = nextRectIds[i];
                }
            }
        }
        
        for (int i = 0; i < 2*N;)
        {
            int r = rectIds[i] & ROTATEMASK;
            if (!placed[r])
            {
                bool f = (rectIds[i] & ~ROTATEMASK) == ROTATE;
                if (PY[p-1] < PY[p+1])
                {
                    if (f)
                    {
                        if (PX[p+1]-SX[r] > PX[p-1] && PX[p+1]-SX[r] < PX[p] && PY[p+1] != PY[p-1]+SY[r])
                        {
                            possibleMove = {PLACE0, r, p};
                            return;
                        }
                    }
                    else
                    {
                        if (PX[p+1]-SY[r] > PX[p-1] && PX[p+1]-SY[r] < PX[p] && PY[p+1] != PY[p-1]+SX[r])
                        {
                            possibleMove = {PLACE1, r, p};
                            return;
                        }
                    }
                }
                else
                {
                    if (f)
                    {
                        if (PX[p]+SX[r] < PX[p+2] && PX[p]+SX[r] > PX[p+1] && PY[p-1] != PY[p+1]+SY[r])
                        {
                            possibleMove = {PLACE0, r, p};
                            return;
                        }
                    }
                    else
                    {
                        if (PX[p]+SY[r] < PX[p+2] && PX[p]+SY[r] > PX[p+1] && PY[p-1] != PY[p+1]+SX[r])
                        {
                            possibleMove = {PLACE1, r, p};
                            return;
                        }
                    }
                }
            }
            else
            {
                nextRectIds[previous] = nextRectIds[i];
            }
            previous = i;
            i = nextRectIds[i];
        }
        
        possibleMove = {SKIP, 0, p};
    }
    else
    {
        int bestEval = numeric_limits<int>::min();
        for (int r = 0; r < N; ++r)
        {
            if (!placed[r])
            {
                int alpha = -alphaCoef * sqrt((100000.*iter1)/N);
                int diffX = abs(SX[r] - SY[r] + alpha);
                int diffY = abs(SY[r] - SX[r] + alpha);
                int desiredSize = 510 + betaCoef + nextInt(20);
                
                if (P < 3) desiredSize = 350;
                int size = abs(SX[r] - desiredSize);
                if (PY[0] < PY[P-1])
                {
                    int eval = -size + nextInt(ADD_RANDOMNESS) + nextInt(ADD_RANDOMNESS);
                    if (bestEval < eval-diffX)
                    {
                        possibleMove = {ADDL0, r, -1};
                        bestEval = eval-diffX;
                    }
                    if (bestEval < eval-diffY)
                    {
                        possibleMove = {ADDL1, r, -1};
                        bestEval = eval-diffY;
                    }
                }
                else
                {
                    int eval = -size + nextInt(ADD_RANDOMNESS) + nextInt(ADD_RANDOMNESS);
                    if (bestEval < eval-diffX)
                    {
                        possibleMove = {ADDR0, r, -1};
                        bestEval = eval-diffX;
                    }
                    if (bestEval < eval-diffY)
                    {
                        possibleMove = {ADDR1, r, -1};
                        bestEval = eval-diffY;
                    }
                }
            }
        }
    }
}

void applyMove(Move & move)
{
    if (move.type & 1)
    {
        swap(SX[move.r], SY[move.r]);
        D[move.r] = !D[move.r];
    }
    
    switch (move.type)
    {
        case ADDL0:
        case ADDL1:
        {
            int r = move.r;
            if (P == 0)
            {
                X[r] = 0;
                Y[r] = 0;
                pyminleft = 0;
                pyminright = 0;
                PY[0] = Y[r]+SY[r];
                PX[0] = X[r];
                PX[1] = X[r]+SX[r];
                P = 1;
            }
            else
            {
                X[r] = PX[0] - SX[r];
                Y[r] = min(PY[0], PY[P-1]);
                pyminleft = Y[r];
                for (int p = P+1; p > 0; --p)
                {
                    PX[p] = PX[p-1];
                    PY[p] = PY[p-1];
                }
                PY[0] = Y[r]+SY[r];
                PX[0] = X[r];
                ++P;
            }
            
            remainingDiag -= diag[r];
            placed[r] = true;
            break;
        }
        case ADDR1:
        case ADDR0:
        {
            int r = move.r;
            if (P == 0)
            {
                X[r] = 0;
                Y[r] = 0;
                pyminleft = 0;
                pyminright = 0;
                PY[0] = Y[r]+SY[r];
                PX[0] = X[r];
                PX[1] = X[r]+SX[r];
                P = 1;
            }
            else
            {
                X[r] = PX[P];
                Y[r] = min(PY[0], PY[P-1]);
                pyminright = Y[r];
                PY[P] = Y[r]+SY[r];
                PX[P] = X[r];
                PX[P+1] = X[r] + SX[r];
                ++P;
            }
            
            remainingDiag -= diag[r];
            placed[r] = true;
            break;
        }
        case PLACE1:
        case PLACE0:
        {
            int r = move.r;
            int p = move.p;
            if (PY[p-1] < PY[p+1])
            {
                holesArea += (PY[p-1]-PY[p]) * (PX[p+1] - PX[p]);
                X[r] = PX[p+1] - SX[r];
                Y[r] = PY[p-1];
                PX[p] = X[r];
                PY[p] = Y[r] + SY[r];
            }
            else
            {
                holesArea += (PY[p+1]-PY[p]) * (PX[p+1] - PX[p]);
                X[r] = PX[p];
                Y[r] = PY[p+1];
                PX[p+1] = X[r] + SX[r];
                PX[p] = X[r];
                PY[p] = Y[r] + SY[r];
            }
            remainingDiag -= diag[r];
            placed[r] = true;
            ++holes;
            break;
        }
        case SKIP:
        {
            int p = move.p;
            for (int i = p; i < P; ++i)
            {
                PY[i] = PY[i+1];
                PX[i] = PX[i+1];
            }
            --P;
            break;
        }
    }
}

void flipHoles()
{
    for (int p = 0; p < P; ++p)
        holesArea -= (PX[p+1] - PX[p]) * (PY[p] - min(PY[0], PY[P-1]));
    for (int r = 0; r < N; ++r)
        if (placed[r])
            Y[r] = -Y[r] - SY[r];
    swap(PY[0], pyminleft);
    PY[0] = -PY[0];
    pyminleft = -pyminleft;
    swap(PY[P-1], pyminright);
    PY[P-1] = -PY[P-1];
    pyminright = -pyminright;
    yl = -yl;
    yr = -yr;
}

void makeHoles()
{
    int n = .62 * N;
    int n2 = .56 * N;
    
    currentMoves.clear();
    for (iter1 = 0; iter1 < n; ++iter1)
    {
        if (iter1 >= lockIter)
        {
            getMoves();
            if (possibleMove.type != PLACE0 && possibleMove.type != PLACE1)
            {
                if (iter1 > n2)
                    break;
            }
            else
            {
                int remainingDiag2 = remainingDiag + PX[P]-PX[0];
                int maxDiag = remainingDiag2 - (holes+1)*remainingDiag2/(double)(holes+2);
                if (diag[possibleMove.r] > maxDiag)
                    break;
                if ((holes+2)*(remainingDiag+PX[P]-PX[0]-diag[possibleMove.r]) < (holes+1)*(remainingDiag+PX[P]-PX[0])) break;
            }
            
            currentMoves.push_back(possibleMove);
        }
        else
        {
            if (iter1 >= bestMoves[currentCopy].size())
                break;
            currentMoves.push_back(bestMoves[currentCopy][iter1]);
        }
        
        applyMove(currentMoves[iter1]);
    }
    flipHoles();
}

void getSides()
{
    nbAreaRect = 0;
    int holesWidth = PX[P]-PX[0];
    sidesSize[0] += holesWidth;
    for (int r = 0; r < N; ++r)
        if (!placed[r])
            areaRect[nbAreaRect++] = r;
    
    sort(areaRect, areaRect+nbAreaRect, RevOrientationSorter());
    
    for (int i = 0; i < nbAreaRect; ++i)
    {
        int r = areaRect[i];
        if (!placed[r])
        {
            if (SX[r] < SY[r])
            {
                swap(SX[r], SY[r]);
                D[r] = !D[r];
            }
            int k;
            if (sidesSize[0] < sidesSize[1] && sidesSize[0] < sidesSize[2] && sidesSize[0] < sidesSize[3])
                k = 0;
            else if (sidesSize[1] < sidesSize[2] && sidesSize[1] < sidesSize[3])
                k = 1;
            else if (sidesSize[2] < sidesSize[3])
                k = 2;
            else
                k = 3;
            sides[k][nbSide[k]++] = r;
            sidesSize[k] += SX[r];
        }
    }
}

void computeSide(int k, int dy)
{
    sort(sides[k], sides[k]+nbSide[k], OrientationSorter());
    
    xl = 0;
    yl = 0;
    xr = sidesSize[k];
    yr = dy;
    
    for (int s = 0; s < nbSide[k]; ++s)
    {
        int r = sides[k][s];
        if (yl < yr)
        {
            X[r] = xl;
            Y[r] = yl;
            xl += SX[r];
            yl += SY[r];
        }
        else
        {
            X[r] = xr-SX[r];
            Y[r] = yr;
            xr -= SX[r];
            yr += SY[r];
        }
    }
}

void moveSide(int k, int ox, int oy)
{
    static const int ROTMAT[4][2][2] = { { { 1,  0 },
        { 0,  1 } },
        { { 0,  1 },
            { -1, 0 } },
        { { -1, 0 },
            { 0, -1 } },
        { { 0, -1 },
            { 1,  0 } } };
    static const int OFFSET[4][2] = { { 0,   0 },
        { 0,  -1 },
        { -1, -1 },
        { -1, 0 } };
    
    for (int s = 0; s < nbSide[k]; ++s)
    {
        int r = sides[k][s];
        if (k % 2 == 1)
        {
            swap(SX[r], SY[r]);
            D[r] = !D[r];
        }
        int x = ROTMAT[k][0][0] * X[r] + ROTMAT[k][0][1] * Y[r] + OFFSET[k][0] * SX[r] + ox;
        int y = ROTMAT[k][1][0] * X[r] + ROTMAT[k][1][1] * Y[r] + OFFSET[k][1] * SY[r] + oy;
        X[r] = x;
        Y[r] = y;
    }
}

void makeArea()
{
    getSides();
    computeSide(0, 0);
    int offsetX = -xl+PX[0];
    int offsetY = max(pyminleft-yl, pyminright-yr);
    computeSide(1, 0);
    computeSide(2, sidesSize[3] - sidesSize[1]);
    computeSide(3, sidesSize[0] - sidesSize[2]);
    moveSide(0, offsetX,                           offsetY);
    moveSide(1, offsetX+sidesSize[0],              offsetY);
    moveSide(2, offsetX+sidesSize[0],              offsetY-sidesSize[1]);
    moveSide(3, offsetX+sidesSize[0]-sidesSize[2], offsetY-sidesSize[3]);
}

void prePlace()
{
    int ids[MAXRECT];
    for (int r = 0; r < N; ++r)
        ids[r] = r;
    sort(ids, ids+N, DiagSorter());
    
    int x = -500*sqrt(N);
    int y = 1000*sqrt(N);
    for (int r = 0; r < N; ++r)
    {
        X[ids[r]] = x;
        Y[ids[r]] = y;
        x += 1000;
        if (r % (int)sqrt(N) == 0)
        {
            x = -500*sqrt(N);
            y -= 1000;
        }
    }
}

void evalSolution()
{
    sort(sides[0], sides[0]+nbSide[0], CoordinateSorter<true, false>());
    sort(sides[1], sides[1]+nbSide[1], CoordinateSorter<false, true>());
    sort(sides[2], sides[2]+nbSide[2], CoordinateSorter<true, true>());
    sort(sides[3], sides[3]+nbSide[3], CoordinateSorter<false, false>());
    
    int by = 1e6;
    int bi = -1;
    for (int i = 0; i < nbSide[2]; ++i)
        if (by > Y[sides[2][i]] + SY[sides[2][i]])
        {
            by = Y[sides[2][i]] + SY[sides[2][i]];
            bi = i;
        }
    
    int y = Y[sides[2][bi]] + SY[sides[2][bi]];
    int xl = X[sides[2][bi]] + SX[sides[2][bi]];
    int xr = X[sides[2][bi]];
    int sl = 2;
    int sr = 2;
    int il = bi;
    int ir = bi;
    
    do
    {
        int rl = sides[sl][il];
        int rr = sides[sr][ir];
        if (Y[rl] + SY[rl] > Y[rr] + SY[rr])
        {
            --ir;
            if (ir < 0)
            {
                --sr;
                ir = nbSide[sr]-1;
                if (sl == 0 && nbSide[0] == 0) break;
            }
            area += ((Y[rr] + SY[rr]) - y) * (xr - xl);
            xr = X[sides[sr][ir]];
            y = Y[rr] + SY[rr];
        }
        else
        {
            ++il;
            if (il == nbSide[sl])
            {
                sl = (sl + 1) & 3;
                il = 0;
                if (sl == 0 && nbSide[0] == 0) break;
            }
            area += ((Y[rl] + SY[rl]) - y) * (xr - xl);
            xl = X[sides[sl][il]] + SX[sides[sl][il]];
            y = Y[rl] + SY[rl];
        }
        
    }
    while (sl != sr || il != ir);
    
    if (nbSide[0] != 0)
    {
        int rl = sides[sl][il];
        int rr = sides[sr][ir];
        if (Y[rl] + SY[rl] > Y[rr] + SY[rr])
        {
            --ir;
            if (ir < 0)
            {
                --sr;
                ir = nbSide[sr]-1;
            }
            area += ((Y[rr] + SY[rr]) - y) * (xr - xl);
            xr = X[sides[sr][ir]];
            y = Y[rr] + SY[rr];
        }
        else
        {
            ++il;
            if (il == nbSide[sl])
            {
                sl = (sl + 1) & 3;
                il = 0;
            }
            area += ((Y[rl] + SY[rl]) - y) * (xr - xl);
            xl = X[sides[sl][il]] + SX[sides[sl][il]];
            y = Y[rl] + SY[rl];
        }
    }
    
    score = area*(double)holes*holes;
}

void makeReturn()
{
    for (int i = 0; i < N; ++i)
    {
        ret[3*i] = X[i];
        ret[3*i+1] = Y[i];
        ret[3*i+2] = D[i];
    }
}

class RectanglesAndHoles
{
public:
    Timer timer;
    
    vector<int> place(vector<int> A, vector<int> B)
    {
        N = A.size();
        
        for (int i = 0; i < N; ++i)
        {
            SX[i] = A[i];
            SY[i] = B[i];
        }
        
        init0();
        iter0 = 0;
        while (timer.left() > 0.)
        {
            progress = timer.spent();
            init1();
#ifdef LOCAL
            prePlace();
#endif
            makeHoles();
            
            BP();
            init2(false);
            
            makeArea();
            evalSolution();
            
            int worstCopy = 0;
            double worstBestScore = 1.e50;
            bool found = false;
            for (int i = 0; i < PARALLEL_RUN; ++i)
            {
                if (currentMoves.size() == bestMoves[worstCopy].size() && memcmp(&currentMoves[0], &bestMoves[worstCopy][0], currentMoves.size()*sizeof(Move)) == 0)
                    found = true;
                if (worstBestScore > bestScore[i])
                {
                    worstBestScore = bestScore[i];
                    worstCopy = i;
                }
            }
            
            if (bestScore[worstCopy] < score && found == false)
            {
                bestScore[worstCopy] = score;
                bestMoves[worstCopy] = currentMoves;
                
                if (bestScoreEver < score)
                {
                    bestScoreEver = score;
                    makeReturn();
                }
                
                for (int i = 0; i < 50; ++i)
                {
                    init2(true);
                    makeArea();
                    evalSolution();
                    if (bestScoreEver < score)
                    {
                        makeReturn();
                        bestScoreEver = score;
                    }
                }
            }
            
            iter0++;
        }
#ifndef LOCAL
        cout << iter0 << endl;
#endif
        
        PZ();
        return ret;
    }
};


using namespace ant;

int main(int argc, const char * argv[])
{
    Count count;
    cin >> count;
    vector<int> width(count);
    vector<int> height(count);
    for (Index i = 0; i < count; ++i) {
        cin >> width[i];
    }
        
    for (Index i = 0; i < count; ++i) {
        cin >> height[i];
    }
    RectanglesAndHoles rah;
    auto ret = rah.place(width, height);
    
    for (auto r : ret) {
        cout << r << endl; 
    }
    cout.flush();
    return 0;
}

