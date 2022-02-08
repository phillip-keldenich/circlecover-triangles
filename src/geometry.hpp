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

struct CircleResult {
    ivarp::IDouble cx, cy;
    ivarp::IBool exists;
};

struct Point {
    ivarp::IDouble x, y;
};

struct IntersectionResult {
    Point first_on_line, second_on_line;
    ivarp::IBool exists;
};

inline Point center_of(Point p1, Point p2) noexcept {
    return Point{0.5 * (p1.x + p2.x), 0.5 * (p1.y + p2.y)};
}

inline ivarp::IDouble squared_distance(Point p1, Point p2) noexcept {
    return ivarp::square(p1.x - p2.x) + ivarp::square(p1.y - p2.y);
}

/**
 * Find the circle center of a circle of radius r to the right of p -> q
 * such that p and q lie on the circle.
 *
 * @param p
 * @param q
 * @param r
 * @return
 */
inline CircleResult circle_right_of(Point p, Point q, ivarp::IDouble r) noexcept {
    using ivarp::IDouble;
    using ivarp::IBool;
    Point lc = center_of(p, q);
    IDouble ellsq = squared_distance(p, q);
    IDouble musq = ivarp::square(r) / ellsq - 0.25;
    if(musq.ub() < 0.0) {
        return {IDouble::undefined_value(), IDouble::undefined_value(), IBool{false, false}};
    }
    IBool exists{musq.lb() >= 0.0, true};
    musq.restrict_lb(0.0);
    IDouble cwrot_dx = q.y - p.y;
    IDouble cwrot_dy = p.x - q.x;
    IDouble mu = ivarp::sqrt(musq);
    IDouble cx = lc.x + mu * cwrot_dx;
    IDouble cy = lc.y + mu * cwrot_dy;
    return {cx, cy, exists};
}

struct Circle {
    Point center;
    ivarp::IDouble radius;
};

inline IntersectionResult line_circle_intersection(Point anchor, Point orientation, Circle circle) {
    using ivarp::IDouble;
    using ivarp::IBool;
    using ivarp::sqrt;
    using ivarp::square;
    Point undef = Point{IDouble::undefined_value(), IDouble::undefined_value()};
    IntersectionResult result{undef, undef, {false, false}};
    IDouble a_x = anchor.x, a_y = anchor.y;
    IDouble d_x = orientation.x, d_y = orientation.y;
    IDouble c_x = circle.center.x, c_y = circle.center.y, r = circle.radius;
    IDouble x0 = square(d_x);
    IDouble x1 = square(d_y);
    IDouble or_check = x0 + x1;
    IBool or_exists = (or_check > 0);
    if(!possibly(or_exists)) {
        result.exists = IBool{false, true};
        return result;
    }
    IDouble x2 = 1.0 / or_check;
    if(!x2.definitely_defined()) {
        x2 = IDouble{0.0, std::numeric_limits<double>::infinity()};
    }
    IDouble x3 = a_y * d_y;
    IDouble x4 = a_x * d_x;
    IDouble x5 = 2.0 * x4;
    IDouble x6 = c_y * d_y;
    IDouble x7 = c_x * d_x;
    IDouble x8 = 2.0 * x7;
    IDouble x9 = square(r);
    IDouble x10sq = -square(a_x) * x1 + 2*a_x*c_x*x1 - square(a_y)*x0 + 2*a_y*c_y*x0 -
                    square(c_x)*x1 - square(c_y)*x0 +
                    x0*x9 + x1*x9 + x3*x5 - x3*x8 - x5*x6 + x6*x8;
    IBool x10sqnonneg = (x10sq >= 0);
    if(!possibly(x10sqnonneg)) {
        return result;
    }
    x10sq.restrict_lb(0.0);
    IDouble x10 = sqrt(x10sq);
    IDouble x11 = -x3 - x4 + x6 + x7;
    result.exists = or_exists && x10sqnonneg;
    IDouble mu_first = x2 * (x11 - x10);
    IDouble mu_second = x2 * (x10 + x11);
    result.first_on_line.x = a_x + mu_first * d_x;
    result.first_on_line.y = a_y + mu_first * d_y;
    result.second_on_line.x = a_x + mu_second * d_x;
    result.second_on_line.y = a_y + mu_second * d_y;
    return result;
}
