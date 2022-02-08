/*
 * Copyright 2022 Phillip Keldenich, Algorithms Department, TU Braunschweig
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <ivarp_ia/ivarp_ia.hpp>
#include "constraint.hpp"
#include "rectangle_cover.hpp"
#include "geometry.hpp"

template<typename VariableSet>
struct TwoLargeDiskChecker {
    using IDouble = ivarp::IDouble;
    using IBool = ivarp::IBool;

    explicit TwoLargeDiskChecker(const VariableSet& vset) noexcept :
        alpha(vset.get_alpha()),
        cos_alpha(vset.cos_alpha),
        r1(vset.get_r1()),
        r2(vset.get_r2()),
        r1sq(ivarp::square(r1)),
        r2sq(ivarp::square(r2)),
        remaining_weight(vset.weight - r1sq - r2sq),
        x4(vset.tan_alpha_half)
    {}

    IBool routine_fails() noexcept {
        r1w = 0.25 / x4 - 0.5 * r2 * (1.0 + cos_alpha);
        r1wsq = ivarp::square(r1w);
        r1hsq = r1sq - r1wsq;
        IBool r1_can_cover_width = (r1hsq >= 0.0);
        if(!possibly(r1_can_cover_width)) {
            return {true, true};
        }
        r1hsq.restrict_lb(0.0);
        r1h = ivarp::sqrt(r1hsq);
        IBool r1_intersects_top = compute_r1_intersections();
        if(!possibly(r1_intersects_top)) {
            return {true, true};
        }
        IBool only_one_pocket = (x_u_left >= 2.0 * r1w);
        pocket_height_right = 1.0 - x3;
        pocket_height_left = x4 * x_u_left;
        pocket_height = (ivarp::max)(pocket_height_left, pocket_height_right);
        return !r1_can_cover_width || !r1_intersects_top || !only_one_pocket ||
               !rectangle_cover_works(x_u_right, pocket_height, remaining_weight, IDouble{0.0, r2.ub()});
    }

    IBool compute_r1_intersections() noexcept {
        x0 = 0.5 * alpha;
        x1 = ivarp::square(ivarp::cos(x0));
        x3 = 2.0 * r1h;
        x5 = r1w * x4;
        x6 = ivarp::square(x4);
        IDouble x7sq = r1sq - r1hsq - r1wsq*x6 + r1sq*x6 - x3*x5 + x3 + 2.0*x5 - 1.0;
        IBool res = (x7sq >= 0.0);
        if(!possibly(res)) {
            return res;
        }
        x7sq.restrict_lb(0.0);
        x7 = ivarp::sqrt(x7sq);
        x8 = r1w - r1h*x4 + x4;
        x_u_right = x1 * (x8 - x7);
        x_u_left = x1 * (x7 + x8);
        return res;
    }

    ivarp::IDouble alpha, cos_alpha, r1, r2, r1sq, r2sq, remaining_weight, r1w, r1wsq, r1hsq, r1h;

    /*
     * sympy.cse([right_x_u, left_x_u, pocket_height_right, pocket_height_left])
     *
     *   ([(x0, alpha/2),
     *     (x1, cos(x0)**2),
     *     (r1sq, r_1**2),
     *     (x3, 2*r_1^h),
     *     (x4, tan(x0)),
     *     (x5, r_1^w*x4),
     *     (x6, x4**2),
     *     (x7, sqrt(-r_1^h**2 - r_1^w**2*x6 + r1sq*x6 + r1sq - x3*x5 + x3 + 2*x5 - 1)),
     *     (x8, -r_1^h*x4 + r_1^w + x4)],
     *    [x1*(-x7 + x8), # right_x_u
     *     x1*(x7 + x8),  # left_x_u
     *     1 - x3,        # pocket_height_right
     *     x4*x_u])       # pocket_height_left
     */
    ivarp::IDouble x0, x1, x3, x4, x5, x6, x7, x8, x_u_right, x_u_left;
    ivarp::IDouble pocket_height_right, pocket_height_left, pocket_height;
};

template<typename VariableSet>
struct TwoLargeDiskConvergentChecker {
    using IDouble = ivarp::IDouble;
    using IBool = ivarp::IBool;

    explicit TwoLargeDiskConvergentChecker(const VariableSet& vset) noexcept :
        alpha(vset.get_alpha()), r1(vset.get_r1()), r2(vset.get_r2()),
        tan_alpha_half(vset.tan_alpha_half),
        cos_alpha(vset.cos_alpha),
        sin_alpha(vset.sin_alpha),
        r1sq(ivarp::square(r1)),
        r2sq(ivarp::square(r2)),
        height(vset.height),
        weight(vset.weight),
        x1(0.5 / tan_alpha_half),
        y1(-0.5),
        x2(r2 * (1+cos_alpha)),
        y2(r2 * sin_alpha),
        delta_x(x1-x2), delta_y(y2-y1),
        ell_sq(ivarp::square(delta_x) + ivarp::square(delta_y)),
        mu_sq(r1sq / ell_sq - 0.25)
    {}

    IBool routine_fails() noexcept {
        IBool r1_covers_segment = (mu_sq >= 0.0);
        if(!possibly(r1_covers_segment)) {
            return {true, true};
        }
        mu_sq.restrict_lb(0.0);
        compute_r1_center();
        IBool r1_covers_bot_left = check_bot_left();
        if(!possibly(r1_covers_bot_left)) {
            return {true, true};
        }
        IBool sec_top_intersection_exists = compute_second_top_intersection();
        if(!possibly(sec_top_intersection_exists)) {
            return {true, true};
        }
        IBool upper_right_intersection_exists = compute_upper_right_intersection();
        if(!possibly(upper_right_intersection_exists)) {
            return {true, true};
        }
        IDouble remaining_height = compute_remaining_height();
        IDouble required_weight = ivarp::square(remaining_height / (1 + cos_alpha));
        IDouble remaining_weight = weight - r1sq - r2sq;
        return !r1_covers_segment || !r1_covers_bot_left || !sec_top_intersection_exists ||
               !upper_right_intersection_exists || remaining_weight < required_weight;
    }

    IDouble alpha, r1, r2;
    IDouble tan_alpha_half, cos_alpha, sin_alpha, r1sq, r2sq, height, weight;
    IDouble x1, x2, y1, y2;
    IDouble delta_x, delta_y;
    IDouble ell_sq, mu_sq;
    IDouble cx, cy;
    IDouble vx, vy;
    IDouble ty;

private:
    IDouble compute_remaining_height() {
        IDouble height_triangle_tip_v = height - vx;
        IBool triangle_tip_v_suffices = (vy - tan_alpha_half * height_triangle_tip_v <= ty);
        IDouble remaining_height;
        if(definitely(triangle_tip_v_suffices)) {
            remaining_height = height_triangle_tip_v;
        } else {
            remaining_height = (0.5 / tan_alpha_half) * (0.5 - vy);
            if(possibly(triangle_tip_v_suffices)) {
                remaining_height = ivarp::join(remaining_height, height_triangle_tip_v);
            }
        }
        return remaining_height;
    }

    void compute_r1_center() {
        IDouble mu = ivarp::sqrt(mu_sq);
        cx = 0.5 * (x1 + x2) + mu * delta_y;
        cy = 0.5 * (y1 + y2) + mu * delta_x;
    }

    IBool compute_second_top_intersection() {
        IDouble t0 = 2 * r2sq;
        IDouble t1 = 2 * alpha;
        IDouble t2 = 2 * r2 * ivarp::sin(t1);
        IDouble t3 = t0 * ivarp::cos(t1);
        IDouble t4 = 8 * r1sq;
        IDouble v_x_sqrt_term_squared = (t0 - t2 - t3 + t4*cos_alpha - t4 + 1) / (t2 - t0 + t3 - 1);
        IBool result = (v_x_sqrt_term_squared >= 0.0);
        if(!possibly(result)) {
            return result;
        }
        v_x_sqrt_term_squared.restrict_lb(0.0);
        vx = (ivarp::sqrt(v_x_sqrt_term_squared) * sin_alpha + cos_alpha) / (2.0 * tan_alpha_half);
        vy = tan_alpha_half * vx;
        return result;
    }

    IBool check_bot_left() {
        IDouble xdiff = cx - x2;
        IDouble ydiff = cy + y2;
        IDouble sqdist = ivarp::square(xdiff) + ivarp::square(ydiff);
        return sqdist <= r1sq;
    }

    IBool compute_upper_right_intersection() {
        IDouble ydiff_sq = r1sq - ivarp::square(height - cx);
        IBool result = (ydiff_sq >= 0.0);
        if(!possibly(result)) {
            return result;
        }
        ydiff_sq.restrict_lb(0.0);
        ty = cy + ivarp::sqrt(ydiff_sq);
        return result;
    }
};

template<typename VariableSet>
struct TwoLargeDiskLongSideChecker
{
    using IDouble = ivarp::IDouble;
    using IBool = ivarp::IBool;

    explicit TwoLargeDiskLongSideChecker(const VariableSet& vset) noexcept :
        alpha(vset.get_alpha()),
        r1(vset.get_r1()),
        r2(vset.get_r2()),
        r1sq(square(r1)),
        r2sq(square(r2)),
        remaining_weight(vset.weight - r1sq - r2sq),
        sin_alpha(vset.sin_alpha),
        height(vset.height)
    {}

    IBool routine_fails() noexcept {
        IBool have_weight = (remaining_weight > 0);
        if(!possibly(have_weight)) {
            return {true, true};
        }
        remaining_weight.restrict_lb(0.0);
        compute_remaining_rho();
        IBool r1_can_be_placed = compute_covered_rect();
        if(!possibly(r1_can_be_placed)) {
            return {true, true};
        }
        return !have_weight || !r1_can_be_placed || !r2_suffices_for_rest();
    }

private:
    void compute_remaining_rho() {
        remaining_rho = ivarp::sqrt(remaining_weight);
        b_r = 2.0 * remaining_rho * sin_alpha;
        IDouble ahalf = 0.5 * alpha;
        cos_alpha_half = ivarp::cos(ahalf);
        sin_alpha_half = ivarp::sin(ahalf);
        s_w = (1.0 - b_r) * cos_alpha_half;
    }

    IBool compute_covered_rect() {
        IDouble r_w_sq = 4 * r1sq - ivarp::square(s_w);
        IBool can_be_placed = (r_w_sq >= 0);
        if(!possibly(can_be_placed)) {
            return can_be_placed;
        }
        r_w_sq.restrict_lb(0.0);
        r_w = ivarp::sqrt(r_w_sq);
        upper_intersection.x = height - cos_alpha_half * r_w;
        upper_intersection.y = 0.5 - sin_alpha_half * r_w;
        lower_intersection.x = upper_intersection.x + s_w * sin_alpha_half;
        lower_intersection.y = upper_intersection.y - s_w * cos_alpha_half;
        return can_be_placed;
    }

    IBool r2_suffices_for_rest() {
        IDouble rem_length_top = (0.5 / sin_alpha_half) - r_w;
        IDouble height_r2_sq = 4.0 * r2sq - square(rem_length_top);
        IDouble height_sw_sq = 4.0 * r2sq - square(s_w);
        IBool height_r2_possible = (height_r2_sq >= 0);
        IBool height_sw_possible = (height_sw_sq >= 0);
        if(!possibly(height_r2_possible) || !possibly(height_sw_possible)) {
            return {false, false};
        }
        height_r2_sq.restrict_lb(0.0);
        height_sw_sq.restrict_lb(0.0);
        IDouble height_r2 = ivarp::sqrt(height_r2_sq);
        IDouble height_sw = ivarp::sqrt(height_sw_sq);
        IBool approach1 = height_r2_possible && (height_r2 >= s_w);
        if(definitely(approach1)) {
            return approach1;
        }
        IBool approach2 = height_sw_possible && (height_sw >= rem_length_top);
        return approach1 || approach2;
    }

    IDouble alpha, r1, r2, r1sq, r2sq, remaining_weight, sin_alpha, height;
    IDouble cos_alpha_half, sin_alpha_half;
    IDouble remaining_rho, b_r, s_w, r_w;
    Point upper_intersection, lower_intersection;
};

template<typename VariableSet>
struct TwoLargeDiskLongSide : Constraint<VariableSet>
{
    std::string name() const override {
        return "Cover a strip along a long triangle side with two disks";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        TwoLargeDiskLongSideChecker<VariableSet> checker(vars);
        return checker.routine_fails();
    }
};

template<typename VariableSet>
struct TwoLargeDisks : Constraint<VariableSet> {
    std::string name() const override {
        return "Cover most of the triangle with two disks";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        TwoLargeDiskChecker<VariableSet> checker(vars);
        return checker.routine_fails();
    }
};

template<typename VariableSet>
struct TwoLargeDisksConvergent : Constraint<VariableSet> {
    std::string name() const override {
        return "Cover most of the triangle with two disks, converging to two-disk worst case";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        TwoLargeDiskConvergentChecker<VariableSet> checker(vars);
        return checker.routine_fails();
    }
};
