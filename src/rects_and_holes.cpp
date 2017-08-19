//
//  rects_and_holes.cpp
//  RectanglesAndHoles
//
//  Created by Anton Logunov on 5/23/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include "rects_and_holes.h"

double computeScore(const Plane& plane) {
    return plane.hole_count*plane.hole_count*plane.total_hole_area;
}