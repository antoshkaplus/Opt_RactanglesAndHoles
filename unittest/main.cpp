//
//  main.cpp
//  Test
//
//  Created by Anton Logunov on 5/23/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include <iostream>

#include <ant>

#include "rects_and_holes.h"
#include "plane.h"
#include "circle_plane.h"
#include "eldidou.h"

void sample(vector<int>& ws, vector<int>& hs, ant::Count n) {
    ws.resize(n);
    hs.resize(n);
    for (auto i = 0; i < n; ++i) {
        ws[i] = rand()%1000+1;
        hs[i] = rand()%1000+1;
    }
}

void meta_test() {
    vector<int> ws, hs;
    auto n = 1000;
    sample(ws, hs, n);
    vector<Size> sizes(n);
    for (auto i = 0; i < n; ++i) {
        sizes[i] = {(size_t)ws[i], (size_t)hs[i]};
    }
    ant::algebra::Mat<Count> mat(n, n); 
    for (auto i = 0; i < 100; ++i) {
        Plane plane;
        plane.place(sizes);
        for (auto k = 0; k < n; ++k) {
//            for (auto k_2 : plane.neighbors()[k]) {
//                if (k_2 == n) continue;
//                mat(k, k_2) = mat(k_2, k) += 1;
//            }
        }
    }
    
    vector<ant::Count> cs(100, 0);
    for (auto k = 0; k < n; ++k) {
        for (auto k_2 = k+1; k_2 < n; ++k_2) {
            ++cs[mat(k, k_2)];
        }
    }
    
    for (auto i = 0; i < 100; ++i) {
        cout << "i : " << i << " count : " << cs[i] << endl; 
    }
}

void solution_test() {
    vector<ant::Int> ws, hs;
    sample(ws, hs, 1000);
    RectanglesAndHoles rah;
    rah.place(ws, hs);
}

void circle_plane_test() {
    size_t n = 100;
    vector<ant::Int> ws, hs;
    sample(ws, hs, n);
    vector<Size> sizes(n);
    for (auto i = 0; i < n; ++i) {
        sizes[i] = {(size_t)ws[i], (size_t)hs[i]};
    }
    CirclePlane cp;
    cp.place(sizes);
}


int main(int argc, const char * argv[])
{
    circle_plane_test();
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}

