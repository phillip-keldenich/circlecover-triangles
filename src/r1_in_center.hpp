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
#include "constraint.hpp"
#include "rectangle_cover.hpp"

struct R1InCenterChecker {
    using IBool = ivarp::IBool;
    using IDouble = ivarp::IDouble;

    template<typename VariableSet>
    explicit R1InCenterChecker(const VariableSet& vset) :
        alpha(vset.get_alpha()),
        r1(vset.get_r1()),
        r2(vset.get_r2()),
        r3(vset.get_r3()),
        weight(vset.weight),
        x5(vset.tan_alpha_half),
        x6(vset.cos_alpha)
    {}

    IBool routine_fails() {
        compute_chi1();
        IBool fail_due_to_large_chi1 = (chi_1 >= r1);
        if(possibly(fail_due_to_large_chi1)) {
            return fail_due_to_large_chi1;
        }
        compute_remaining_sizes();
        check_coverage_options();
        IBool both_pockets = both_pockets_work();
        if(definitely(both_pockets)) {
            return {false, false};
        }
        IBool pocket_and_triangle = pocket_and_triangle_works();
        if(definitely(pocket_and_triangle)) {
            return {false, false};
        }
        IBool only_triangle = only_triangle_works();
        if(definitely(only_triangle)) {
            return {false, false};
        }
        IBool only_pocket = only_pocket_works();
        if(definitely(only_pocket)) {
            return {false, false};
        }
        return !both_pockets && !pocket_and_triangle && !only_triangle && !only_pocket && !no_pocket_works();
    }

    void check_coverage_options() {
        r3pocket = (r3sq >= pocket_weight_bound);
        r2pocket = (r2sq >= pocket_weight_bound);
        r2triangle = check_r2_triangle();
    }

    IBool both_pockets_work() {
        return r3pocket && (rw3 >= weight_for_triangle);
    }

    IBool pocket_and_triangle_works() {
        return r3pocket && r2triangle &&
               rectangle_cover_works(remaining_pocket_width, remaining_pocket_height,
                                     rw3, IDouble{0.0, r3.ub()});
    }

    IBool only_triangle_works() {
        IDouble min_weight_per_pocket = 0.5 * rw2 - r3sq;
        return r2triangle && rectangle_cover_works(remaining_pocket_width, remaining_pocket_height,
                                                   min_weight_per_pocket, IDouble{0.0, r3.ub()});
    }

    IBool only_pocket_works() {
        IDouble rem_weight_for_pocket = rw3 - weight_for_triangle;
        return r2pocket && rectangle_cover_works(remaining_pocket_width, remaining_pocket_height,
                                                 rem_weight_for_pocket, IDouble{0.0, r3.ub()});
    }

    IBool no_pocket_works() {
        IDouble rem_weight_for_pockets = rw2 - weight_for_triangle;
        IDouble min_weight_for_pockets = 0.5 * (rem_weight_for_pockets - r3sq);
        return rectangle_cover_works(remaining_pocket_width, remaining_pocket_height,
                                     min_weight_for_pockets, IDouble{0.0, r3.ub()});
    }

    void compute_chi1() {
        x0 = 0.5 * alpha;
        x1 = ivarp::sin(x0);
        x4 = ivarp::cos(x0);
        x2 = x1 + 1.0;
        x3 = 1.0 / x2;
        chi_1 = x3*(r1*x1 + r1 - 0.5 * x4);
    }

    void compute_remaining_sizes() {
        x7 = ivarp::square(r1);
        x8 = 8.0 * x7;
        x9 = 4.0 * x7;
        x10 = ivarp::square(x5) * x9;
        x11 = x4 * ivarp::sqrt((16.0*x1*x7 + x10*x6 + x10 - x6 + x8 - 1.0)/(x6 + 1.0));
        x12 = x1 - 1.0;
        x13 = x12 * x5;
        x14 = 0.5 / x4;
        x15 = ivarp::square(x4);
        x16 = x14 * (x2 - x11);
        remaining_triangle_half_base = x14*(x13*(x11 + x2) + x4);
        remaining_triangle_height = remaining_triangle_half_base/x5;
        remaining_pocket_width = -x12*x16;
        IDouble right_pocket_height = 0.5*x3*(x2 - ivarp::sqrt(x1*x8 - x15*x9 - x15 + x8));
        IDouble left_pocket_height = -x13*x16;
        remaining_pocket_height = ivarp::max(right_pocket_height, left_pocket_height);
        r2sq = ivarp::square(r2);
        r3sq = ivarp::square(r3);
        pocket_weight_bound = 0.25 * (ivarp::square(remaining_pocket_width) + ivarp::square(remaining_pocket_height));
        weight_for_triangle = compute_weight_for_triangle();
        rw1 = weight - x7;
        rw2 = rw1 - r2sq;
        rw3 = rw2 - r3sq;
    }

    IDouble compute_weight_for_triangle() const noexcept {
        IDouble scale = 2.0 * remaining_triangle_half_base;
        return weight * ivarp::square(scale);
    }

    IBool check_r2_triangle() const noexcept {
        IDouble base_distance = remaining_triangle_height - r2;
        return ivarp::square(base_distance) + ivarp::square(remaining_triangle_half_base) <= r2sq;
    }

    IDouble alpha;
    IDouble r1, r2, r3;
    IDouble weight;
    IDouble chi_1;
    IDouble remaining_triangle_height, remaining_triangle_half_base;
    IDouble remaining_pocket_height;
    IDouble remaining_pocket_width;
    IDouble x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16;
    IDouble r2sq, r3sq;
    IDouble pocket_weight_bound, weight_for_triangle;
    IDouble rw1, rw2, rw3;
    IBool r3pocket, r2pocket, r2triangle;
    /*
     * sympy.cse([chi_1_v, remaining_triangle_halpha, pocket_height, pocket_width, left_pocket_height])
     *   ([(x0, alpha/2),
     *     (x1, sin(x0)),
     *     (x2, x1 + 1),
     *     (x3, 1/x2),
     *     (x4, cos(x0)),
     *     (x5, tan(x0)),
     *     (x6, cos(alpha)),
     *     (x7, r_1**2),
     *     (x8, 8*x7),
     *     (x9, 4*x7),
     *     (x10, x5**2*x9),
     *     (x11, x4*sqrt((16*x1*x7 + x10*x6 + x10 - x6 + x8 - 1)/(x6 + 1))),
     *     (x12, x1 - 1),
     *     (x13, x12*x5),
     *     (x14, 1/(2*x4)),
     *     (x15, x4**2),
     *     (x16, x14*(-x11 + x2))],
     *    [x3*(r_1*x1 + r_1 - x4/2), #chi_1
     *     x14*(x13*(x11 + x2) + x4)/x5, #remaining_triangle_height
     *     x3*(x2 - sqrt(x1*x8 - x15*x9 - x15 + x8))/2, # pocket_height
     *     -x12*x16, # pocket_width
     *     -x13*x16]) # left_pocket_height
     */
};


template<typename VariableSet> struct R1InCenterCover : Constraint<VariableSet> {
    std::string name() const override {
        return "Place r_1 on vertical center line";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        R1InCenterChecker checker(vars);
        return checker.routine_fails();
    }
};
