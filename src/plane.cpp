//
//  plane.cpp
//  RectanglesAndHoles
//
//  Created by Anton Logunov on 5/21/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#include "plane.h"

constexpr Index Plane::kSideIndices[];
constexpr Index Plane::kOppositeSideIndex[];



bool operator<(const Plane::Value& v_0, const Plane::Value& v_1) {
    return v_0.top < v_1.top;
}
bool operator>(const Plane::Value& v_0, const Plane::Value& v_1) {
    return v_0.top > v_1.top;
}
bool operator==(const Plane::Value& v_0, const Plane::Value& v_1) {
    return v_0.top == v_1.top;
}
bool operator!=(const Plane::Value& v_0, const Plane::Value& v_1) {
    return v_0.top != v_1.top;
}
bool operator<=(const Plane::Value& v_0, const Plane::Value& v_1) {
    return v_0.top <= v_1.top;
}
bool operator>=(const Plane::Value& v_0, const Plane::Value& v_1) {
    return v_0.top >= v_1.top;
}

size_t operator+(const Plane::Value& v, size_t s) {
    return v.top + s;
}
Int operator-(const Plane::Value& v_0, const Plane::Value& v_1) {
    return v_0.top - v_1.top; 
}
Int operator-(const Plane::Value& v_0, Int s) {
    return v_0.top - s; 
}
bool operator!=(size_t s, const Plane::Value& v) {
    return s != v.top;
}
bool operator<=(size_t s, const Plane::Value& v) {
    return s <= v.top;
}
bool operator<=(const Plane::Value& v, size_t s) {
    return v.top <= s;
}
bool operator>=(const Plane::Value& v, size_t s) {
    return v.top >= s;
}
bool operator>=(size_t s, const Plane::Value& v) {
    return s >= v.top;
}
bool operator>(size_t s, const Plane::Value& v) {
    return s > v.top;
}
bool operator>(const Plane::Value& v, size_t s) {
    return v.top > s;
}

vector<Plane::Hollow> Plane::hollows() {
    vector<Hollow> hs;
    for (auto i : kSideIndices) {
        auto& s = sides[i];
        auto b_pv = s.begin();
        if (b_pv == s.end()) continue;
        auto b = next(b_pv);
        if (b == s.end()) continue;
        auto b_nt = next(b);
        
        while (b_nt != s.end()) {
            if (b_pv->value > b->value &&
                b_nt->value > b->value) {
                hs.emplace_back(i, b);
            }
            b_pv = b;
            b = b_nt;
            ++b_nt;
        }
    }
    return hs;
}

vector<Plane::Ladder> Plane::ladders() {
    vector<Ladder> ls;
    for (auto i : kSideIndices) {
        auto& s = sides[i];
        auto b_pv = s.begin();
        if (b_pv == s.end()) continue;
        auto b = next(b_pv);
        if (b == s.end()) continue;
        auto b_nt = next(b);
        
        while (b_nt != s.end()) {
            if (b->value > b_pv->value &&
                b->value < b_nt->value) {
                
                ls.emplace_back(i, b, true);
            }
            if (b->value < b_pv->value &&
                b->value > b_nt->value) {
                
                ls.emplace_back(i, b, false);
            }
            b_pv = b;
            b = b_nt;
            ++b_nt;
        }
    }
    return ls;
};


vector<Plane::HollowPlace> Plane::hollowPlaces(const Hollow& h) {
    vector<HollowPlace> result;
    
    auto& s = sides[h.side_index];
    auto 
    b = h.iterator, 
    b_pv = prev(b),
    b_nt = next(b),
    b_pv_pv = b, 
    b_nt_nt = b;
    if (b_pv == s.begin() || 
        (b_pv_pv = prev(b_pv))->value < b_pv->value) {
        
        b_pv_pv = s.end();
    }
    if ((b_nt_nt = next(b_nt)) != s.end() &&
         b_nt_nt->value < b_nt->value) {
        
        b_nt_nt = s.end();
    }
    
    ant::d1::Segment seg_width, seg_height;
    seg_width.set_first(b->segment.length);
    if (b_nt->value > b_pv->value) {
        seg_width.set_last(b->segment.length+b_pv->segment.length-1);
    } else {
        seg_width.set_last(b->segment.length+b_nt->segment.length-1);
    }
    
    
    Int v;
    ant::Int h_min, h_max;
    if (b_nt_nt != s.end() && b_pv_pv == s.end()) {
        
    } else if (b_nt_nt != s.end() && b_pv_pv != s.end()) {
        tie(h_min, h_max) = minmax(b_pv_pv->value-v, b_nt_nt->value-v);
        seg_height.set_first(h_min);
        seg_height.set_last(h_min+h_max);
    } else if (b_nt_nt == s.end() && b_pv_pv == s.end()){
        
    } else if (b_nt_nt == s.end() && b_pv_pv != s.end()) {
        
    }
    
    
    
    // don't forget plane
    HollowPlace h_pc(*this);
    for (auto i_sz_pair : sizes_left) {
        Index i_sz = i_sz_pair;//.second;
        for (auto sz : {(*sizes)[i_sz], (*sizes)[i_sz].swapped()}) {
            if (b_pv->value >= b_nt->value &&
                b->segment.length <= sz.width &&
                b->segment.length + b_nt->segment.length > sz.width &&
                b->value + sz.height != b_pv->value 
                ) {
                
                h_pc.rect.origin.set(b->segment.origin, b_nt->value.top);
                h_pc.rect.size = sz;
                h_pc.hole_area = b->segment.length*(b_nt->value - b->value);
                h_pc.touch.width = sz.width - b->segment.length;
                if (b_pv->value <= b_nt->value + sz.height) {
                    h_pc.touch.height = b_pv->value - b_nt->value;
                }
                else {
                    h_pc.touch.height = sz.height;
                }
                h_pc.size_index = i_sz;
                
                auto b_v = b_nt->value + sz.height;
                auto al_pv = 
                ((b_pv_pv == s.end() || b_pv->value >= b_v) ? 0. : fabs(b_pv_pv->value-b_v+0.00001));
                //if (b_pv->value >= b_v) al_pv = 500;
                auto al_nt = 
                (b_nt_nt == s.end() ? 0. : labs(b_nt_nt->value-b_v+0.00001));
                h_pc.alignment = sqrt(al_pv*b_pv->segment.length + al_nt*b_nt->segment.length)
                                /((al_pv > 0) + (al_nt > 0));
                //h_pc.alignment = sqrt(al_pv*al_pv + al_nt*al_nt);
                //h_pc.alignment = max(al_pv, al_nt);
                result.push_back(h_pc);
            }
            if (b_pv->value <= b_nt->value &&
                b->segment.length <= sz.width &&
                b->segment.length + b_pv->segment.length > sz.width &&
                b->value + sz.height != b_nt->value  
                ) {
                
                h_pc.rect.origin = {b->segment.last()-(Int)sz.width, b_pv->value.top};
                h_pc.rect.size = sz;
                h_pc.hole_area = b->segment.length*(b_pv->value - b->value);
                h_pc.touch.width = sz.width - b->segment.length;
                if (b_pv->value + sz.height >= b_nt->value) {
                    h_pc.touch.height = b_nt->value - b_pv->value;
                }
                else {
                    h_pc.touch.height = sz.height;
                }
                h_pc.size_index = i_sz;
                
                auto b_v = b_pv->value + sz.height;
                auto al_pv = 
                (b_pv_pv == s.end() ? 0. : fabs(b_pv_pv->value-b_v+0.00001));
                auto al_nt = 
                ((b_nt_nt == s.end() || b_nt->value >= b_v) ? 0. : fabs(b_nt_nt->value-b_v+0.00001));
                //if (b_nt->value >= b_v) al_nt = 500;
                h_pc.alignment = sqrt(al_pv*b_pv->segment.length + al_nt*b_nt->segment.length)
                                /((al_pv > 0) + (al_nt > 0));
                //h_pc.alignment = sqrt(al_pv*al_pv + al_nt*al_nt);
                //h_pc.alignment = max(al_pv, al_nt);
                 
                result.push_back(h_pc);
            }
        }
    }
    return result;
} 

vector<Plane::HollowPlace> lowLeftHollowPlaces() {
    return vector<Plane::HollowPlace>();
}

vector<Plane::HollowPlace> lowRightHollowPlaces() {
    return vector<Plane::HollowPlace>();
}




vector<Plane::LadderPlace> Plane::ladderPlaces(const Plane::Ladder& g) {
    vector<LadderPlace> result;
    auto s = sides[g.side_index];
    auto 
    b = g.iterator, 
    b_pv = prev(b),
    b_nt = next(b),
    b_pv_pv = b, 
    b_nt_nt = b;
    
    #define heur(d_pv, d_nt) sqrt((d_pv)*(d_pv) + (d_nt)*(d_nt)) 
    
    // don't forget plane
    LadderPlace h_pc(*this);
    for (auto i_sz_pair : sizes_left) {
        Index i_sz = i_sz_pair;//.second;
        for (auto sz : {(*sizes)[i_sz], (*sizes)[i_sz].swapped()}) {
            // big challenge here...
            if (g.goes_up) {
                b_pv_pv = b_pv;
                if (b_pv != s.begin()) {
                    ant::d1::Segment seg(b->segment.first()-(Int)sz.width, (Int)sz.width);
                    auto d = b_pv;
                    while ((--d)->segment.isIntersect(seg)) {
                        if (d->value > b->value) goto next_sz;
                        b_pv_pv = d;
                        if (d == s.begin()) break;
                    }
                }
            
                h_pc.rect.origin.set(
                    b->segment.origin - (Int)sz.width, 
                    b->value.top
                );
                h_pc.rect.size = sz;
                h_pc.size_index = i_sz;
                
                double d_pv = fabs(b->segment.origin - (Int)sz.width - (double)b_pv_pv->segment.first());
                //d_pv = fabs(b->segment.length - (Int)sz.width);
                double d_nt = fabs(sz.height - (b_nt->value - b->value));
                // should work on improving this metric
                h_pc.alignment = heur(d_pv, d_nt);
                //h_pc.alignment = sqrt(d_nt*b->segment.length + d_pv*(b->value-b_pv_pv->value));
                //h_pc.alignment = rand();
                
                                
                result.push_back(h_pc);
            } else {
                b_nt_nt = b_nt;
                if (next(b_nt) != s.end()) {
                    ant::d1::Segment seg(b->segment.last(), (Int)sz.width);
                    auto d = b_nt;
                    while (++d != s.end() && d->segment.isIntersect(seg)) {
                        b_nt_nt = d;
                        if (d->value > b->value) goto next_sz;
                    }
                }
                
                h_pc.rect.origin.set(
                    b->segment.last(), 
                    b->value.top
                );
                h_pc.rect.size = sz;
                h_pc.size_index = i_sz;
                
                double d_pv = fabs(sz.height - (b_pv->value - b->value));
                double d_nt = fabs(b->segment.last() + sz.width - (double)b_nt_nt->segment.last());
                //d_nt = fabs(b->segment.length - sz.width);
                h_pc.alignment = heur(d_pv, d_nt);
                //h_pc.alignment = sqrt(d_nt*(b->value - b_nt_nt->value) + d_pv*b->segment.length);
                //h_pc.alignment = rand();
                
                result.push_back(h_pc);
            }
            next_sz:
            ; // haha
        }
    }
    #undef heur
    return result;
}





