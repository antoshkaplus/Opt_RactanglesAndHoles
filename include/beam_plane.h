//
//  beam_plane.h
//  RectanglesAndHoles
//
//  Created by Anton Logunov on 5/31/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __RectanglesAndHoles__beam_plane__
#define __RectanglesAndHoles__beam_plane__

#include <iostream>

#include "plane.h"

struct BeamPlane : Plane {
    
    vector<Rectangle> place(const vector<Size>& sizes) {
        init(sizes);
        GlobalPlace place;
        
        auto k = rand()%sizes_left.size()+1;
        auto sz_it = sizes_left.begin();
        while (--k != 0) {
            ++sz_it; 
        } 
        Index size_index = *sz_it;//->second;
        place = GlobalPlace(
                                 *this,
                                 kTop,
                                 Rectangle({0, 0}, sizes[size_index]), 
                                 size_index);
        insert(place);

        for (auto i = 0; i < 3; ++i) {
            place = randomPlace();
            insert(place);
        }
        Plane best_plane;
        HollowPlace h_pc;
        bool h_pc_initialized;
        Index h_side_index;
        vector<Ladder> ls;
        vector<LadderPlace> l_pcs;
        while (!sizes_left.empty()) {
            ls = ladders();
            if (ls.empty()) {
                place = randomPlace();
                insert(place);
                continue;
            }
            shuffle(ls.begin(), ls.end(), rng_);
            ls.resize(min<Int>(100000000/(size_count*size_count*5), ls.size()));
            bool best_initialized = false;
            for (auto s : ls) {
                Plane plane = *this;
                auto& side = plane.sides[s.side_index];
                for (auto it = side.begin(); it != side.end(); ++it) {
                    if (it->segment == s.iterator->segment) {
                        s.iterator = it;
                        break;
                    }
                }
                bool ladder_found;
                tie(ladder_found, static_cast<Place&>(place)) = bestLadderPlace(s);
                if (!ladder_found) continue;
                place.side_index = s.side_index;
                plane.insert(place);
                while(true) {
                    tie(h_pc_initialized, h_pc, h_side_index) = plane.findHollowPlace();
                    if (!h_pc_initialized) break;
                    plane.total_touching += h_pc.touch.width*h_pc.rect.size.height + 
                                            h_pc.touch.height*h_pc.rect.size.width 
                                            - h_pc.touch.width*h_pc.touch.height;
                    ++plane.hole_count;
                    plane.total_hole_area += h_pc.hole_area;
                    static_cast<Place&>(place) = h_pc;
                    place.side_index = h_side_index;
                    plane.insert(place);
                } 
                if (!best_initialized || 
                     sqrt(plane.total_touching)/(sqrt(plane.hole_count+0.001)*plane.total_hole_area)< 
                     sqrt(best_plane.total_touching)/(sqrt(best_plane.hole_count+0.001)*best_plane.total_hole_area)) {
                    
                    best_plane = plane;
                    best_initialized = true;
                }
            }
            if (best_initialized) static_cast<Plane&>(*this) = best_plane;
        }
        vector<Rectangle> rects(sizes.size());
        for (auto& s : solution) {
            rects[get<0>(s)] = get<1>(s);
        }
        return rects;
    }
};


#endif /* defined(__RectanglesAndHoles__beam_plane__) */
