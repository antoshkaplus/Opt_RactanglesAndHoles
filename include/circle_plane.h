//
//  new_plane.h
//  RectanglesAndHoles
//
//  Created by Anton Logunov on 5/29/14.
//  Copyright (c) 2014 Anton Logunov. All rights reserved.
//

#ifndef __RectanglesAndHoles__new_plane__
#define __RectanglesAndHoles__new_plane__

#include <iostream>
#include <string>

#include "plane.h"

const string root = "/Users/antoshkaplus/Documents/Programming/Contests/TopCoder/RectanglesAndHoles/Scripts/";

using d2Segment = ant::d2::i::Segment;

struct CirclePlane {
    
    struct SolutionItem {
        SolutionItem(Index size_index, const Rectangle& rect)
        : size_index(size_index), rect(rect) {}
        Index size_index;
        Rectangle rect;
    };
    
    struct Place {
        bool isBetterThan(const Place& p) {
            return heuristic() < p.heuristic();
        }
        
        double heuristic() const {
            return
            (touch.width*rect.size.height + 
             touch.height*rect.size.width + 
             - touch.width*touch.height) + alignment*alignment;
        }
        
        Rectangle rect;
        Size touch{0, 0};
        double alignment{0};
        Index size_index;
    };
    
    struct PlaceResult {
        bool is_initialized{false};
        Place place;
    };
    
    using Solution = vector<SolutionItem>;
    using Ring = ant::circular_list<Point>;
    using RingIt = Ring::iterator;
    
    vector<Index> largest_area_sizes(const vector<Size>& sizes, Count count) {
        vector<Index> result(sizes.size());
        iota(result.begin(), result.end(), 0);
        nth_element(result.begin(), result.begin()+count, result.end(), [&](Index i_0, Index i_1) {
            return sizes[i_0].area() > sizes[i_1].area();
        });
        result.resize(count);
        return result;
    }
    
    size_t total_width(const vector<Index>& inds, const vector<bool>& b) {
        size_t s = 0;
        for (auto i : inds) {
            auto &sz = (*sizes_)[i];
            s += b[i] ? sz.height : sz.width;
        }
        return s;
    }
    
    size_t total_height(const vector<Index>& inds, const vector<bool>& b) {
        size_t s = 0;
        for (auto i : inds) {
            auto &sz = (*sizes_)[i];
            s += b[i] ? sz.width : sz.height;
        }
        return s;
    }
    
    size_t total_perimeter(const vector<Index>& inds) {
        size_t s = 0;
        for (auto i : inds) {
            s += (*sizes_)[i].perimeter();
        }
        return s;
    }
    
    void improve(vector<Index>& g_0, vector<bool>& b_0, vector<Index>& g_1, vector<bool>& b_1) {
        auto &sizes = *this->sizes_; 
        size_t 
        p_0 = total_perimeter(g_0), 
        p_1 = total_perimeter(g_1);
        bool again = true;
        // first equal areas
        while (again) {
            again = false;
            for (Index i = 0; i < g_0.size(); ++i) {
                for (Index j = 0; j < g_1.size(); ++j) {
                    size_t 
                    s_0 = sizes[g_0[i]].perimeter(),
                    s_1 = sizes[g_1[j]].perimeter();
                    if (labs((p_0 - s_0 + s_1) - (p_1 - s_1 + s_0)) < labs(p_0 - p_1)) {
                        swap(g_0[i], g_1[j]);
                        swap(b_0[i], b_1[j]);
                        p_0 += s_1 - s_0;
                        p_1 += s_0 - s_1;
                        again = true;
                    }
                }
            }
        }
        size_t 
        w_0 = total_width(g_0, b_0),
        w_1 = total_width(g_1, b_1);
        again = true;
        // now equal widths
        while (again) {
            again = false;
            for (auto i = 0; i < g_0.size(); ++i) {
                auto s = sizes[g_0[i]];
                if (b_0[i]) s.swap();
                if (labs(w_0 - s.width + s.height - w_1) < labs(w_0 - w_1)) {
                    b_0[i] = !b_0[i];
                    w_0 += s.height - s.width;
                    again = true;
                }
            }
            for (auto i = 0; i < g_1.size(); ++i) {
                auto s = sizes[g_1[i]];
                if (b_1[i]) s.swap();
                if (labs(w_1 - s.width + s.height - w_0) < labs(w_1 - w_0)) {
                    b_1[i] = !b_1[i];
                    w_1	 += s.height - s.width;
                    again = true;
                }
            }
        }
        // think about it later
        //g_0 = sortedHeights(g_0);
        //g_1 = sortedHeights(g_1);
    }
    
    vector<Index> sortedHeights(vector<Index> g) {
        auto &sizes = *this->sizes_;
        sort(g.begin(), g.end(), [&](Index i_0, Index i_1) {
            return sizes[i_0].height > sizes[i_1].height;
        });
        Index i_0 = 0, i_1 = g.size()-1;
        vector<Index> res(g.size());
        for (auto i = 0; i < g.size(); ++i) {
            if (i%2 == 0) {
                res[i_0++] = g[i];
            } else {
                res[i_1--] = g[i];
            }
        }
        return res;
    }
    
    static d2Segment bottom(const Rectangle& rect) {
        return d2Segment({rect.origin.x, rect.origin.y}, 
                         {rect.origin.x + static_cast<Int>(rect.size.width), rect.origin.y});
    }
    
    static d2Segment right(const Rectangle& rect) {
        return d2Segment({rect.origin.x + (Int)rect.size.width, rect.origin.y},
                         {rect.origin.x + (Int)rect.size.width, rect.origin.y + (Int)rect.size.height});
    }
    
    static d2Segment left(const Rectangle& rect) {
        return d2Segment(rect.origin, {rect.origin.x, rect.origin.y+(Int)rect.size.height}); 
    }
    
    static d2Segment top(const Rectangle& rect) {
        return d2Segment({rect.origin.x, rect.origin.y+(Int)rect.size.height}, 
                         {rect.origin.x+(Int)rect.size.width, rect.origin.y+(Int)rect.size.height});
    }
    
    void initSize(const vector<Index>& g, const vector<bool>& b, vector<Rectangle>& rs) {
        auto& sizes = *this->sizes_; 
        for (Index i = 0; i < g.size(); ++i) {
            rs[i].size = b[i] ? sizes[g[i]].swapped() : sizes[g[i]];
        }
    }
    
    void initOriginX(Int x_0, vector<Rectangle>& rs) {
        for (auto& r : rs) {
            r.origin.x = x_0;
            x_0 += r.size.width;
        }
    }
    
    void buildRing(const vector<Index>& inds) {
        auto &sizes = *this->sizes_;
        
        vector<Index> 
        g_0(inds.begin(), inds.begin()+inds.size()/2),
        g_1(inds.begin()+inds.size()/2, inds.end());
        vector<bool> 
        b_0(g_0.size(), false), 
        b_1(g_1.size(), false);
        
        improve(g_0, b_0, g_1, b_1);
        size_t 
        w_0 = total_width(g_0, b_0),
        w_1 = total_width(g_1, b_1);
        if (w_0 < w_1) {
            swap(g_0, g_1);
            swap(b_0, b_1);
            swap(w_0, w_1);
        }
        vector<Rectangle>
        g_r_0(g_0.size()),
        g_r_1(g_1.size());
        
        initSize(g_0, b_0, g_r_0);
        initSize(g_1, b_1, g_r_1);
        
        initOriginX(0, g_r_0);
        initOriginX((Int)(w_0-w_1)/2, g_r_1);

        // go up with g_r_0
        Index 
        i_0 = 0, 
        i_1 = g_0.size()-1;
        Int 
        h_0 = 0, 
        h_1 = 0;
        
        g_r_0[i_0].origin.y = h_0;
        g_r_0[i_1].origin.y = h_1;
        h_0 += g_r_0[i_0].size.height;
        h_1 += g_r_0[i_1].size.height;
        
        // it_fst and it_snd are necessary for g_r_1
        ant::circular_list<d2Segment> seg_ring;
        auto 
        it_0 = seg_ring.insert_after(seg_ring.focus(), right(g_r_0[i_0])), 
        it_fst = it_0;
        auto 
        it_1 = seg_ring.insert_after(it_0, left(g_r_0[i_1]).swapped()), 
        it_snd = it_1;
        ++i_0;
        --i_1;
        // should be minimum 3 of those guys
        while (i_0 != i_1) {
            if (h_0 > h_1) {
                g_r_0[i_1].origin.y = h_1;
                h_1 += g_r_0[i_1].size.height;
                it_1 = seg_ring.insert_before(it_1, bottom(g_r_0[i_1]));
                it_1 = seg_ring.insert_before(it_1, left(g_r_0[i_1]).swapped());
                --i_1; 
            }
            else {
                g_r_0[i_0].origin.y = h_0;
                h_0 += g_r_0[i_0].size.height;
                it_0 = seg_ring.insert_after(it_0, bottom(g_r_0[i_0]));
                it_0 = seg_ring.insert_after(it_0, right(g_r_0[i_0]));
                ++i_0;
            }
        }
        Int y = g_r_0[i_0].origin.y = min(h_0, h_1);
        {
            Int top = y + (Int)g_r_0[i_0].size.height;
            if (top < h_0 || top < h_1) {
                cerr << "work more on balance top";
                throw;
            }
        }
        if (it_0->fst.y >= y) {
            seg_ring.erase(it_0--);
            // it's prev one!
            if (it_0->snd.y > y) {
                auto s = left(g_r_0[i_0]).swapped();
                s.fst = it_0->snd;
                it_0 = seg_ring.insert_after(it_0, s);
            }
        } else {
            it_0->snd.y = y;
        }
        
        seg_ring.insert_after(it_0, bottom(g_r_0[i_0]));
        
        if (it_1->snd.y >= y) {
            seg_ring.erase(it_1++);
            // it's next one!
            if (it_1->fst.y > y) {
                auto s = right(g_r_0[i_1]);
                s.snd = it_1->fst;
                it_1 = seg_ring.insert_before(it_1, s);
            }
        } else {
            it_1->fst.y = y;
        }
        
        // go down with g_r_1
        i_0 = 0;
        i_1 = g_1.size()-1;
        it_0 = it_fst;
        it_1 = it_snd;
        h_0 = g_r_1[i_0].origin.y = -(Int)g_r_1[i_0].size.height;
        h_1 = g_r_1[i_1].origin.y = -(Int)g_r_1[i_1].size.height;
        Int x_0 = it_fst->fst.x;
        Int x_1 = it_snd->snd.x;
        Int x_0_right = g_r_1[i_0].origin.x + (Int)g_r_1[i_0].size.width;
        if (x_0_right != x_0) {
            it_0 = seg_ring.insert_before(it_0, {{x_0_right, 0}, {x_0, 0}});
        } // else we do nothing 
        Int x_1_left = g_r_1[i_1].origin.x;
        if (x_1_left != x_1) {
            it_1 = seg_ring.insert_after(it_1, {{x_1, 0}, {x_1_left, 0}});
        }
        
        it_0 = seg_ring.insert_before(it_0, right(g_r_1[i_0]));
        it_1 = seg_ring.insert_after(it_1, left(g_r_1[i_1]).swapped());
        
        ++i_0;
        --i_1;
        while (i_0 != i_1) {
            if (h_0 < h_1) {
                h_1 -= g_r_1[i_1].size.height;
                g_r_1[i_1].origin.y = h_1;
                it_1 = seg_ring.insert_after(it_1, top(g_r_1[i_1]).swapped());
                it_1 = seg_ring.insert_after(it_1, left(g_r_1[i_1]).swapped());
                --i_1; 
            }
            else {
                h_0 -= g_r_1[i_0].size.height;
                g_r_1[i_0].origin.y = h_0;
                it_0 = seg_ring.insert_before(it_0, top(g_r_1[i_0]).swapped());
                it_0 = seg_ring.insert_before(it_0, right(g_r_1[i_0]));
                ++i_0;
            }
        }
        
        y = max(h_0, h_1);
        g_r_1[i_0].origin.y = y - (Int)g_r_1[i_0].size.height;
      
        if (it_0->snd.y <= y) {
            seg_ring.erase(it_0++);
            if (it_0->fst.y < y) {
                auto s = left(g_r_1[i_0]).swapped();
                s.snd = it_0->fst;
                it_0 = seg_ring.insert_before(it_0, s);
            }
        } else {
            it_0->fst.y = y;
        }
        
        it_0 = seg_ring.insert_before(it_0, this->top(g_r_1[i_0]).swapped());
        
        if (it_1->fst.y <= y) {
            seg_ring.erase(it_1--);
            if (it_1->snd.y < y) {
                auto s = right(g_r_1[i_0]);
                s.fst = it_1->snd;
                it_1 = seg_ring.insert_after(it_1, s);
            }
        } else {
            it_1->snd.y = y;
        }
        
        auto seg_it = seg_ring.focus(), seg_it_end = seg_it;
        auto it = ring_.focus();
        while (!seg_ring.empty()) {
            it = ring_.insert_after(it, seg_it->fst);
            if (++seg_it == seg_it_end) break; 
        }
        
        for (auto i = 0; i < g_0.size(); ++i) {
            solution_.push_back(SolutionItem(g_0[i], g_r_0[i]));
        }
        for (auto i = 0; i < g_1.size(); ++i) {
            solution_.push_back(SolutionItem(g_1[i], g_r_1[i]));
        }
    }
    
    void snapshot(ostream& out_rects, ostream& out_ring) {
        for (auto& item : solution_) {
            auto& r = item.rect; 
            out_rects 
            << r.origin.x << " " << r.origin.y << " "
            << r.size.width << " " << r.size.height << endl;
        }
        auto it = ring_.focus(), it_next = next(it), it_end = it;
        while (!ring_.empty()) {
            out_ring 
            << it->x << " " << it->y << " " 
            << it_next->x << " " << it_next->y << endl;
            it = it_next++;
            if (it == it_end) break;
        }
    }
    
    void place(const vector<Size>& sizes) {
        this->sizes_ = &sizes;
        vector<Index> inds(sizes.size());
        iota(inds.begin(), inds.end(), 0);
        sizes_left_.insert(inds.begin(), inds.end());
        auto ring_sizes = largest_area_sizes(sizes, 30);
        buildRing(ring_sizes);
        for (auto i : ring_sizes) {
            sizes_left_.erase(i);
        }
    
        auto 
        i_pv_pv = ring_.focus(), 
        i_pv = next(i_pv_pv),
        i_0 = next(i_pv),
        i_1 = next(i_0),
        i_nt = next(i_1),
        i_nt_nt = next(i_nt); 
        PlaceResult pr;
        bool success = false;
        
        Count count = 0;
        Count unsuccess_count = 0;
        Count ladder_count = 0;
        while (!sizes_left_.empty()) {
            if (unsuccess_count == ring_.size()) {
                if (!placeLadder()) {
                    break;
                }
                unsuccess_count = 0;
                ++ladder_count; 
            }
            //if (++count == 1000) break;
            success = false;
            // vertical
            if (i_0->x == i_1->x) { 
                // left
                if (i_nt->x > i_0->x && i_pv->x > i_0->x && *i_pv != *i_nt_nt) {
                    pr = resolveLeft(*i_pv_pv, *i_pv, *i_0, *i_1, *i_nt, *i_nt_nt);
                    if (pr.is_initialized) {
                        auto& r = pr.place.rect;
                        if (i_nt->x > i_pv->x) {
                            ring_.erase(i_pv);
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            auto it = ring_.insert_after(i_pv_pv, r.origin);
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y});
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y+(Int)r.size.height});
                        } else {
                            ring_.erase(i_nt);
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            assert(ring_.isFeasible());
                            auto it = ring_.insert_after(i_pv, {r.origin.x+(Int)r.size.width, r.origin.y});
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, {r.origin.x, r.origin.y+(Int)r.size.height});
                        }
                        success = true;
                        solution_.emplace_back(pr.place.size_index, r);
                        goto iteration_end;
                    }
                }
                // right
                if (i_nt->x < i_0->x && i_pv->x < i_0->x && *i_pv != *i_nt_nt) {
                    pr = resolveRight(*i_pv_pv, *i_pv, *i_0, *i_1, *i_nt, *i_nt_nt);
                    if (pr.is_initialized) {
                        auto& r = pr.place.rect;
                        if (i_nt->x > i_pv->x) {
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            ring_.erase(i_nt);
                            auto it = ring_.insert_after(i_pv, {r.origin.x, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, r.origin);
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y});
                        } else {
                            ring_.erase(i_pv);
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            auto it = ring_.insert_after(i_pv_pv, {r.origin.x+(Int)r.size.width, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, {r.origin.x, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, r.origin);
                        }
                        success = true;
                        solution_.emplace_back(pr.place.size_index, r);
                        goto iteration_end;
                    }
                }
            }
            
            if (i_0->y == i_1->y) { 
                // bottom
                if (i_nt->y > i_0->y && i_pv->y > i_0->y && *i_pv != *i_nt_nt) {
                    pr = resolveBottom(*i_pv_pv, *i_pv, *i_0, *i_1, *i_nt, *i_nt_nt);
                    cerr << "try find bottom" << endl;
                    
                    if (pr.is_initialized) {
                        auto& r = pr.place.rect;
                        if (i_nt->y > i_pv->y) {
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            ring_.erase(i_pv);
                            auto it = ring_.insert_after(i_pv_pv, {r.origin.x+(Int)r.size.width, r.origin.y});
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, {r.origin.x, r.origin.y+(Int)r.size.height});
                        } else {
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            ring_.erase(i_nt);
                            auto it = ring_.insert_after(i_pv, {r.origin.x+(Int)r.size.width, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, {r.origin.x, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, r.origin);
                        }
                        success = true;
                        solution_.emplace_back(pr.place.size_index, r);
                        goto iteration_end;
                    }
                }
                if (i_nt->y < i_0->y && i_pv->y < i_0->y && *i_pv != *i_nt_nt) {
                    pr = resolveTop(*i_pv_pv, *i_pv, *i_0, *i_1, *i_nt, *i_nt_nt);
                    cerr << "try find top" << endl;
                    
                    if (pr.is_initialized) {
                        auto& r = pr.place.rect;
                        if (i_nt->y > i_pv->y) {
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            ring_.erase(i_nt);
                            auto it = ring_.insert_after(i_pv, r.origin);
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y});
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y+(Int)r.size.height});
                        } else {
                            ring_.erase(i_0);
                            ring_.erase(i_1);
                            ring_.erase(i_pv);
                            auto it = ring_.insert_after(i_pv_pv, {r.origin.x, r.origin.y+(Int)r.size.height});
                            it = ring_.insert_after(it, r.origin);
                            it = ring_.insert_after(it, {r.origin.x+(Int)r.size.width, r.origin.y});
                        }
                        success = true;
                        solution_.emplace_back(pr.place.size_index, r);
                        goto iteration_end;
                    }
                }
            }
            
            iteration_end:
            if (success) {
                i_pv_pv = i_nt_nt;
                i_pv = next(i_pv_pv);
                i_0 = next(i_pv);
                i_1 = next(i_0);
                i_nt = next(i_1);
                i_nt_nt = next(i_nt);
                sizes_left_.erase(pr.place.size_index);
                unsuccess_count = 0;
            } else {
                i_pv_pv = i_pv;
                i_pv = i_0;
                i_0 = i_1;
                i_1 = i_nt;
                i_nt = i_nt_nt;
                ++i_nt_nt; 
                ++unsuccess_count;
            }
        }
        cerr << "sizes left: " << sizes_left_.size() << endl;
        cerr << "ladder count: " << ladder_count << endl;
        ofstream out_rects(root + "out_rects.txt");
        ofstream out_ring(root + "out_segs.txt");
        snapshot(out_rects, out_ring);
    }
       
    // p_pv.y >= p_nt.y
    PlaceResult place(
        const Point& p_pv_pv, 
        const Point& p_pv,
        const Point& p_0,
        const Point& p_1,
        const Point& p_nt,
        const Point& p_nt_nt) {
        
        PlaceResult pr_best;
        Place pc;
        size_t 
        p_length = p_1.x - p_0.x,
        p_pv_length = p_pv.x - p_pv_pv.x,
        p_nt_length = p_nt_nt.x - p_nt.x;
        for (auto i_sz : sizes_left_) {
            for (auto sz : {(*sizes_)[i_sz], (*sizes_)[i_sz].swapped()}) {
                if (p_length <= sz.width && 
                    p_length + p_nt_length > sz.width && 
                    p_nt.y + sz.height != p_pv.y 
                    ) {
            
                    pc.rect.origin.set(p_pv.x, p_nt.y);
                    pc.rect.size = sz;
                    
                    pc.touch.width = sz.width - p_length; 
                    if (p_pv.y <= p_nt.y + sz.height) {
                        pc.touch.height = p_pv.y - p_nt.y;
                    }
                    else {
                        pc.touch.height = sz.height;
                    }
                    pc.size_index = i_sz;
                    
                    if (!pr_best.is_initialized || pc.isBetterThan(pr_best.place)) {
                        pr_best.place = pc;
                        pr_best.is_initialized = true;
                    }
                }
            }
        }
        return pr_best;
    }
    
    void axisYsymmetry(Point& p) {
        p.x *= -1;
    }
    
    void axisXsymmetry(Point& p) {
        p.y *= -1;
    }
    
    void axisYsymmetry(Rectangle& r) {
        r.origin.x = -(r.origin.x + (Int)r.size.width);
    }
    
    void axisXsymmetry(Rectangle& r) {
        r.origin.y = -(r.origin.y + (Int)r.size.height);
    }
    
    PlaceResult resolve(Point p_pv_pv, Point p_pv, Point p_0, Point p_1, Point p_nt, Point p_nt_nt) {
        PlaceResult pr;
        if (p_pv.y < p_nt.y) {
            for (auto *p : {&p_0, &p_1, &p_pv, &p_pv_pv, &p_nt, &p_nt_nt}) {
                axisYsymmetry(*p);
            }
            pr = place(p_nt_nt, p_nt, p_1, p_0, p_pv, p_pv_pv);
            axisYsymmetry(pr.place.rect);
        } else {
            pr = place(p_pv_pv, p_pv, p_0, p_1, p_nt, p_nt_nt);
        }
        return pr;
    }
    
    // placing ladder somewhere
    bool placeLadder() {
        auto 
        i_pv_pv = ring_.focus(),
        i_pv = next(i_pv_pv),
        i_0 = next(i_pv),
        i_nt = next(i_0),
        i_nt_nt = next(i_nt);
        
        PlaceResult pr;
        bool success = false;
        Count iteration_count = 0; 
        while (++iteration_count <= ring_.size()) {
            // top left ladder
            if (i_pv_pv->y < i_pv->y && i_pv->x < i_0->x && i_0->y < i_nt->y && i_nt->x < i_nt_nt->x) {
                pr = resolveTopLeft(*i_pv_pv, *i_pv, *i_0, *i_nt, *i_nt_nt);
                if (pr.is_initialized) {
                    Size sz = pr.place.rect.size;
                    auto it = ring_.insert_after(i_0, {i_0->x, i_0->y-(Int)sz.height});
                    it = ring_.insert_after(it, {i_0->x+(Int)sz.width, i_0->y-(Int)sz.height});
                    it = ring_.insert_after(it, {i_0->x+(Int)sz.width, i_0->y});
                    it = ring_.insert_after(it, *i_0);
                    success = true;
                    break;
                }
            }
            // top right ladder
            if (i_pv_pv->x < i_pv->x && i_pv->y > i_0->y && i_0->x < i_nt->x && i_nt->y > i_nt_nt->y) {
                pr = resolveTopRight(*i_pv_pv, *i_pv, *i_0, *i_nt, *i_nt_nt);
                if (pr.is_initialized) {
                    Size sz = pr.place.rect.size;
                    auto it = ring_.insert_after(i_0, {i_0->x-(Int)sz.width, i_0->y});
                    it = ring_.insert_after(it, {i_0->x-(Int)sz.width, i_0->y-(Int)sz.height});
                    it = ring_.insert_after(it, {i_0->x, i_0->y-(Int)sz.height});
                    it = ring_.insert_after(it, *i_0);
                    success = true;
                    break;
                }
            }
            // bottom right ladder
            if (i_pv_pv->y > i_pv->y && i_pv->x > i_0->x && i_0->y > i_nt->y && i_nt->x > i_nt_nt->x) {
                pr = resolveBottomRight(*i_pv_pv, *i_pv, *i_0, *i_nt, *i_nt_nt);
                if (pr.is_initialized) {
                    Size sz = pr.place.rect.size;
                    auto it = ring_.insert_after(i_0, {i_0->x, i_0->y+(Int)sz.height});
                    it = ring_.insert_after(it, {i_0->x-(Int)sz.width, i_0->y+(Int)sz.height});
                    it = ring_.insert_after(it, {i_0->x-(Int)sz.width, i_0->y});
                    it = ring_.insert_after(it, *i_0);
                    success = true;
                    break;
                }
            }
            // bottom left ladder
            if (i_pv_pv->x > i_pv->x && i_pv->y < i_0->y && i_0->x > i_nt->x && i_nt->y < i_nt_nt->y) {
                pr = resolveBottomLeft(*i_pv_pv, *i_pv, *i_0, *i_nt, *i_nt_nt);
                if (pr.is_initialized) {
                    Size sz = pr.place.rect.size;
                    auto it = ring_.insert_after(i_0, {i_0->x+(Int)sz.width, i_0->y});
                    it = ring_.insert_after(it, {i_0->x+(Int)sz.width, i_0->y+(Int)sz.height});
                    it = ring_.insert_after(it, {i_0->x, i_0->y+(Int)sz.height});
                    it = ring_.insert_after(it, *i_0);
                    success = true;
                    break;
                }
            }
            
            i_pv_pv = i_pv;
            i_pv = i_0;
            i_0 = i_nt;
            i_nt = i_nt_nt;
            ++i_nt_nt;
        }
        
        
        if (success) { 
            sizes_left_.erase(pr.place.size_index);
            solution_.emplace_back(pr.place.size_index, pr.place.rect);
        }
        return success;
    }
    
    
    //Place
    
    PlaceResult resolveBottomRight(Point p_pv_pv, Point p_pv, Point p_0, Point p_nt, Point p_nt_nt) {
        return placeLadder(p_nt_nt, p_nt, p_0, p_pv, p_pv_pv);
    }
    
    PlaceResult resolveTopRight(Point p_pv_pv, Point p_pv, Point p_0, Point p_nt, Point p_nt_nt) {
        for (auto *p : {&p_0, &p_pv, &p_pv_pv, &p_nt, &p_nt_nt}) {
            axisXsymmetry(*p);
        }
        PlaceResult pr = placeLadder(p_pv_pv, p_pv, p_0, p_nt, p_nt_nt);
        axisXsymmetry(pr.place.rect);
        return pr;
    }
    
    PlaceResult resolveTopLeft(Point p_pv_pv, Point p_pv, Point p_0, Point p_nt, Point p_nt_nt) {
        for (auto *p : {&p_0, &p_pv, &p_pv_pv, &p_nt, &p_nt_nt}) {
            axisYsymmetry(*p);
        }
        PlaceResult pr = resolveTopRight(p_nt_nt, p_nt, p_0, p_pv, p_pv_pv);
        axisYsymmetry(pr.place.rect);
        return pr;
    }
    
    PlaceResult resolveBottomLeft(Point p_pv_pv, Point p_pv, Point p_0, Point p_nt, Point p_nt_nt) {
        for (auto *p : {&p_0, &p_pv, &p_pv_pv, &p_nt, &p_nt_nt}) {
            axisYsymmetry(*p);
        }
        PlaceResult pr = placeLadder(p_pv_pv, p_pv, p_0, p_nt, p_nt_nt);
        axisYsymmetry(pr.place.rect);
        return pr;
    }
    
    PlaceResult placeLadder(
        const Point& p_pv_pv,
        const Point& p_pv,
        const Point& p_0,
        const Point& p_nt,
        const Point& p_nt_nt) {
        
        PlaceResult pr_best;
        Place pc;
        size_t
        p_pv_w = p_pv.x - p_pv_pv.x,
        p_nt_h = p_nt_nt.y - p_nt.y;
        for (auto i_sz : sizes_left_) {
            for (auto sz : {(*sizes_)[i_sz], (*sizes_)[i_sz].swapped()}) {
                pc.rect.set(p_0.x-(Int)sz.width, p_0.y, sz.width, sz.height);
                pc.size_index = i_sz;
                pc.alignment = sqrt(pow(abs((Int)sz.width - (Int)p_pv_w), 2) 
                                    + pow(abs((Int)sz.height - (Int)p_nt_h), 2)); 
                if (!pr_best.is_initialized || pc.isBetterThan(pr_best.place)) {
                    pr_best.place = pc;
                    pr_best.is_initialized = true;
                }
            }
        }
        return pr_best;
    }
    
    
    // better use point references
    PlaceResult resolveBottom(Point p_pv_pv, Point p_pv, Point p_0, Point p_1, Point p_nt, Point p_nt_nt) {
        return resolve(p_nt_nt, p_nt, p_1, p_0, p_pv, p_pv_pv);
    }
    
    PlaceResult resolveRight(Point p_pv_pv, Point p_pv, Point p_0, Point p_1, Point p_nt, Point p_nt_nt) {
        for (auto *p : {&p_0, &p_1, &p_pv, &p_pv_pv, &p_nt, &p_nt_nt}) {
            axisYsymmetry(*p);
        }
        PlaceResult pr = resolveLeft(p_nt_nt, p_nt, p_1, p_0, p_pv, p_pv_pv);
        axisYsymmetry(pr.place.rect);
        return pr;
    }
    PlaceResult resolveLeft(Point p_pv_pv, Point p_pv, Point p_0, Point p_1, Point p_nt, Point p_nt_nt) {
        for (auto *p : {&p_0, &p_1, &p_pv, &p_pv_pv, &p_nt, &p_nt_nt}) {
            p->swap();
        }
        PlaceResult pr = resolve(p_pv_pv, p_pv, p_0, p_1, p_nt, p_nt_nt);
        pr.place.rect.swap();
        pr.place.touch.swap();
        return pr;
    }
    PlaceResult resolveTop(Point p_pv_pv, Point p_pv, Point p_0, Point p_1, Point p_nt, Point p_nt_nt) {
        for (auto *p : {&p_0, &p_1, &p_pv, &p_pv_pv, &p_nt, &p_nt_nt}) {
            axisXsymmetry(*p);
        }
        PlaceResult pr = resolve(p_nt_nt, p_nt, p_1, p_0, p_pv, p_pv_pv);
        axisXsymmetry(pr.place.rect);
        return pr;
    }
    
    
    
    set<Index> sizes_left_;
    Solution solution_;
    // should be ring of points
    
    
    Ring ring_;
    default_random_engine rng_;
    const vector<Size>* sizes_;
};


#endif /* defined(__RectanglesAndHoles__new_plane__) */
