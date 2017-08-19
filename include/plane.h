//
//  plane.h
//  RectanglesAndHoles
//
//  Created by Anton Logunov on 5/21/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __RectanglesAndHoles__plane__
#define __RectanglesAndHoles__plane__

#include <iostream>
#include <fstream>
#include <list>
#include <cmath>
#include <cstdlib>
#include <set>
#include <array>
#include <queue>
#include <valarray>

#include <ant>

using namespace std;
using ant::Int;
using ant::Index;
using ant::Count;
using ant::d2::i::Size;
using namespace ant::d2::i;
using namespace ant::d1;
using namespace ant::algebra;

struct Plane {

    constexpr static Index kTop     = 0;
    constexpr static Index kRight   = 1;
    constexpr static Index kBottom  = 2;
    constexpr static Index kLeft    = 3;
    
    constexpr static Count kSideCount = 4;
    constexpr static Index kSideIndices[] = {kTop, kRight, kBottom, kLeft};
    constexpr static Index kOppositeSideIndex[] = {kBottom, kLeft, kTop, kRight};
    
    struct Value;

    using Neighbors = array<Index, kSideCount>; 
    using Side = ant::d1::ValuedSegmentList<Value>;
        
    struct Value {
        Value(Index size_index, Int top)
        : size_index(size_index), top(top) {}
        
        Index size_index;
        Int top;
    };
    
    struct Hollow {
        Hollow() {}
        Hollow(Index side_index, Side::const_iterator iterator)
        : side_index(side_index), iterator(iterator) {}
        
        Index side_index;
        Side::const_iterator iterator;
    };
    
    struct Ladder {
        Ladder() {}
        Ladder(Index side_index, Side::const_iterator iterator, bool goes_up) 
        : side_index(side_index), iterator(iterator), goes_up(goes_up) {}
        
        Index side_index;
        Side::const_iterator iterator;
        bool goes_up;
    };
    
    struct Place {
        Place() {}
        Place(const Plane& plane) : plane_(&plane) {}
        Place(const Plane& plane, const Rectangle& rect, Index size_index) 
        : plane_(&plane), rect(rect), size_index(size_index) {}
        
        Rectangle rect;
        Index size_index;
    protected:
        Neighbors neighbors_;
        const Plane* plane_; 
    };
    
    struct GlobalPlace : Place {
        GlobalPlace() {}
        GlobalPlace(const Plane& plane, Index side_index, const Rectangle& rect, Index size_index) 
        : Place(plane, rect, size_index), side_index(side_index) {}
        
        Index side_index;
    };
    
    // heuristic aid to minimum 
    struct SmartPlace : Place {
        SmartPlace() {}
        SmartPlace(const Plane& plane) : Place(plane) {}
        
        virtual double heuristic() const = 0;
        
        // consider that alignment should be valued more with quantity ..?
        bool isBetterThan(const Place& pc) const {
            return heuristic() < heuristic();
        }
        
        double alignment;
    };
    
    struct HollowPlace : SmartPlace {
        HollowPlace() {}
        HollowPlace(const Plane& plane) : SmartPlace(plane) {}
        
        double heuristic() const override {
            return // touch.width + touch.height + alignment;
                    (touch.width*rect.size.height + 
                     touch.height*rect.size.width + 
                    - touch.width*touch.height + alignment*alignment) ;
        }
        
        Size touch;
        size_t hole_area;
    };
    
    struct LadderPlace : SmartPlace {
        LadderPlace() {}
        LadderPlace(const Plane& plane) : SmartPlace(plane) {}
        
        double heuristic() const override {
            return alignment;
        }
    };

    void init(const vector<Size>& sizes) {
        this->sizes = &sizes;
        hole_count = 0;
        total_hole_area = 0;
        total_touching = 0;
        ladder_place_count = 0;
        hollow_place_count = 0;
        // 0-ing every side
        sides = decltype(sides)();
        size_count = sizes.size();
        for (auto i = 0; i < size_count; ++i) {
            sizes_left.emplace( i);
        }        
    }
    
    // index is about side_index
    tuple<bool, HollowPlace, Index> findHollowPlace() {
        vector<Hollow> hs = hollows();
        HollowPlace pc;
        bool pc_initialized = false;
        Index pc_side_index;
        vector<Index> inds(hs.size());
        iota(inds.begin(), inds.end(), 0);
        shuffle(inds.begin(), inds.end(), rng_);
        for (auto i : inds) {
            auto h_pcs = hollowPlaces(hs[i]);
            if (!h_pcs.empty()) {                        
                pc = *min_element(h_pcs.begin(), h_pcs.end(), 
                                 [](const HollowPlace& h_0, const HollowPlace& h_1) {
                                     return h_0.heuristic() < h_1.heuristic();
                                 });
                pc_side_index = hs[i].side_index;
                pc_initialized = true;
                break;
            }
        }
        return make_tuple(pc_initialized, pc, pc_side_index);
    } 
    
    tuple<bool, LadderPlace> bestLadderPlace(const Ladder& g) {
        auto l_pcs = ladderPlaces(g);
        if (l_pcs.empty()) return make_tuple(false, LadderPlace());
        return make_tuple(true, *min_element(l_pcs.begin(), l_pcs.end(), 
                            [](const LadderPlace& h_0, const LadderPlace& h_1) {
                                return h_0.heuristic() < h_1.heuristic();
                            }));
    }
    
    // greedy 
    // most simple algorithm
    vector<Rectangle> place(const vector<Size>& sizes) {
//        ofstream out_rects("out_rects.txt");
//        ofstream out_bound("out_bound.txt");
        init(sizes);
        
        GlobalPlace best_place;
        bool best_place_initialized; 
        vector<Hollow> hs;
        vector<HollowPlace> h_pcs;
        vector<Ladder> ls;
        vector<LadderPlace> l_pcs;
        bool first = true;
        HollowPlace h_pc;
        Index h_side_index;
        while (sizes_left.size() > 0) {
            best_place_initialized = false;
            
            if (first) {
                first = false;
                auto k = rand()%sizes_left.size()+1;
                auto sz_it = sizes_left.begin();
                while (--k != 0) {
                    ++sz_it; 
                } 
                Index size_index = *sz_it;//->second;
                best_place = GlobalPlace(
                    *this,
                    kTop,
                    Rectangle({0, 0}, sizes[size_index]), 
                    size_index);
                best_place_initialized = true;
            }
            
            if (!best_place_initialized) {
                tie(best_place_initialized, h_pc, h_side_index) = findHollowPlace();
                total_hole_area += h_pc.hole_area;
                total_touching += h_pc.touch.width*h_pc.rect.size.height + 
                h_pc.touch.height*h_pc.rect.size.width 
                - h_pc.touch.width*h_pc.touch.height;
                ++hole_count;
                static_cast<Place&>(best_place) = h_pc;
                best_place.side_index = h_side_index;
                best_place_initialized = true;
                ++hollow_place_count; 
            }
            
            if (!best_place_initialized) {
                ls = ladders();
                shuffle(ls.begin(), ls.end(), rng_);
                for (auto& g : ls) {
                    best_place.side_index = g.side_index;
                    best_place_initialized = true;
                    ++ladder_place_count;

                }
            }
            
            if (!best_place_initialized) {
                best_place = randomPlace();
            }
            
            insert(best_place);
        }
        vector<Rectangle> rects(sizes.size());
        for (auto& s : solution) {
            rects[get<0>(s)] = get<1>(s);
        }
        return rects;
    }
    
//    tuple<bool, HollowPlace> bestHollowPlace() {
//        
//        
//        
//        
//    }
//    
    GlobalPlace randomPlace() {
        Index k = rand()%sizes_left.size()+1;
        auto sz_it = sizes_left.begin();
        while (--k != 0) {
            ++sz_it; 
        } 
        Index size_index = *sz_it;//->second; 
        Index side_index = rand()%4;
        auto& side = sides[side_index];
        auto sz = (*sizes)[size_index];
        
        Int s_0 = side.front().segment.first() - (Int)sz.width+1;
        Int s_1 = side.back().segment.last()-1;
        ant::d1::Segment s(rand()%2 == 0 ? s_0 : s_1, (Int)sz.width);
        return GlobalPlace(*this, side_index, Rectangle({s.origin, side.max_value(s).top}, sz), size_index);
    }
        
    void insert(const GlobalPlace& pc) {
        Rectangle top_rect = toTopRect(pc.rect, pc.side_index); 
        for (auto i : kSideIndices) {
            Rectangle r = fromTopRect(top_rect, i);
            sides[i].insert(
                {r.origin.x, static_cast<Int>(r.size.width)}, 
                {pc.size_index, static_cast<Int>(r.origin.y + r.size.height)}
            );
            
        }
        sizes_left.erase(pc.size_index);
        solution.emplace_back(pc.size_index, top_rect);
    }
    
    
    vector<Hollow> hollows();    
    vector<Ladder> ladders();    
    vector<HollowPlace> hollowPlaces(const Hollow& h);
    vector<LadderPlace> ladderPlaces(const Ladder& g);
    
    Neighbors toTopNeighbors(const Neighbors& neighs, Index side_index) {
        Neighbors top_neighs;
        switch (side_index) {
        case kTop:
            top_neighs = neighs;
            break;
        case kBottom:
            top_neighs = neighs;
            swap(top_neighs[kTop], top_neighs[kBottom]);
            break;
        case kLeft: 
            top_neighs[kTop] = neighs[kRight];
            top_neighs[kRight] = neighs[kBottom];
            top_neighs[kBottom] = neighs[kLeft];
            top_neighs[kLeft] = neighs[kTop]; 
        case kRight:
            top_neighs = neighs;
            swap(top_neighs[kTop], top_neighs[kRight]);
            swap(top_neighs[kLeft], top_neighs[kBottom]);
        }
        return top_neighs;
    }
    
    Rectangle toTopRect(const Rectangle& rect, Index side_index) {
        Rectangle top_rect;
        switch (side_index) {
        case kTop: 
            top_rect = rect;
            break;
        case kBottom: 
            top_rect.set(
                rect.origin.x, 
                -rect.origin.y-rect.size.height, 
                rect.size.width, 
                rect.size.height
            );
            break;
        case kLeft:
            top_rect.set(
                -rect.origin.y-rect.size.height,
                rect.origin.x,
                rect.size.height,
                rect.size.width
            );
            break;
        case kRight:
            top_rect.set(
                rect.origin.y,
                rect.origin.x,
                rect.size.height,
                rect.size.width
            );
            break;
        }
        return top_rect;
    }
    
    Rectangle fromTopRect(const Rectangle& top_rect, Index side_index) {
        Rectangle rect;
        switch (side_index) {
        case kTop:
            rect = top_rect;
            break;
        case kBottom:
            rect.set(
                top_rect.origin.x, 
                -top_rect.origin.y-top_rect.size.height, 
                top_rect.size.width, 
                top_rect.size.height
            );
            break;
        case kLeft:
            rect.set(
                top_rect.origin.y, 
                -top_rect.origin.x-top_rect.size.width, 
                top_rect.size.height, 
                top_rect.size.width
            );
            break;
        case kRight:
            rect.set(
                top_rect.origin.y, 
                top_rect.origin.x, 
                top_rect.size.height, 
                top_rect.size.width
            );
        }
        return rect;
    }
    
    void snapshot(ostream& out_rects, ostream& out_bound, Index side_index) {
        for (auto i = 0; i < solution.size(); ++i) {
            auto r = fromTopRect(get<1>(solution[i]), side_index); 
            out_rects << r.origin.x << " " << r.origin.y << " "
                      << r.size.width << " " << r.size.height << endl;
        }
        for (auto p : sides[side_index]) {
            //out_bound << p.segment.origin << " " << p.segment.length << " " << p.value.value << endl;
        }
    }
    
    size_t key(size_t width, size_t height) const {
        return (width << 1) + (height << 1);
    }
    
    size_t key(Index size_index) const {
        const Size& s = (*sizes)[size_index];
        return key(s.width, s.height);
    }
    
    
    const vector<Size>* sizes;
    // some statics to watch out
    Count hole_count;
    size_t total_hole_area;
    size_t total_touching;
    Count ladder_place_count;
    Count hollow_place_count;
    // all rectangles aligned by top
    vector<tuple<Index, Rectangle>> solution;
    array<Side, 4> sides;
    Count size_count;
private:
    
protected:
    
    set<Index> sizes_left;
    //map<size_t, Index> sizes_left;
    default_random_engine rng_;
};

bool operator<(const Plane::Value& v_0, const Plane::Value& v_1);
bool operator>(const Plane::Value& v_0, const Plane::Value& v_1);
bool operator==(const Plane::Value& v_0, const Plane::Value& v_1);

#endif /* defined(__RectanglesAndHoles__plane__) */
