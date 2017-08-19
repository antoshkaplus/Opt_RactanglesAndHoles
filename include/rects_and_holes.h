//
//  rects_and_holes.h
//  RectanglesAndHoles
//
//  Created by Anton Logunov on 5/23/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __RectanglesAndHoles__rects_and_holes__
#define __RectanglesAndHoles__rects_and_holes__

#include <iostream>
#include <chrono>

#include "plane.h"
#include "beam_plane.h"

double computeScore(const Plane& plane);

struct Solver {
    virtual vector<Rectangle> solve(const vector<Size>& sizes) {
        BeamPlane plane;
        return plane.place(sizes);
    }
};

struct RandomInitialPoint : Solver {
    vector<Rectangle> solve(const vector<Size>& sizes) override {
        Plane plane;
        clock_t start = clock();
        time_t time = 9200;
        vector<Rectangle> best_rects, rects;
        double best_score = 0, score;
        while (1000.*(clock()-start)/CLOCKS_PER_SEC < time) {
             rects = plane.place(sizes);
             if ((score = computeScore(plane)) > best_score) {
                 best_rects = rects;
                 best_score = score;
                 //cerr << "score: " << score << endl;
             }
        }
        clock_t finish = clock();
        cerr << 1000.*(finish-start)/CLOCKS_PER_SEC;
        return best_rects;
    }
};
//
//struct RectanglesAndHoles {
//    vector<int> place(vector<int>& width, vector<int>& height) {
//        vector<Size> sizes(width.size());
//        for (auto i = 0; i < sizes.size(); ++i) {
//            sizes[i].set(width[i], height[i]);
//        }
//        Solver solver;
//        auto rects = solver.solve(sizes);
//        vector<int> result(3*sizes.size());
//        for (auto i = 0; i < sizes.size(); ++i) {
//            auto k = 3*i; 
//            result[k] = rects[i].origin.x;
//            result[k+1] = rects[i].origin.y;
//            result[k+2] = rects[i].size.width != width[i];
//        }
//        return result;
//    }
//};




#endif /* defined(__RectanglesAndHoles__rects_and_holes__) */
