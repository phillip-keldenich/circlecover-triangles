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


#include <ivarp_ia/ivarp_ia.hpp>
#include <doctest/doctest.hpp>
#include <vector>
#include <random>
#include <array>

std::mt19937_64 rng(std::random_device{}());
std::uniform_real_distribution<double> categories[] = {
    std::uniform_real_distribution<double>{1.0, 2.0},
    std::uniform_real_distribution<double>{-1.0, -2.0},
    std::uniform_real_distribution<double>{-1.0, 1.0},
    std::uniform_real_distribution<double>{-1000.0, 1000.0},
    std::uniform_real_distribution<double>{-1000000000.0, 1000000000.0}
};
std::uniform_int_distribution<std::size_t>
    category_selection(0, sizeof(categories) / sizeof(std::uniform_real_distribution<double>) - 1);

using namespace ivarp;

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Integer to double interval") {
    short s1 = -1500, s2 = 15540;
    unsigned short us = 37700;
    int i1 = -1110040, i2 = 11121314;
    unsigned ui = (1u << 31) + 2000;
    std::int64_t i641 = -(1ll << 62), i642 = -(1l << 62) - 10000011,
                 i643 = (1ll << 61) + 33;
    std::uint64_t u641 = (1ull << 63) - 55u, u642 = 0,
                  u643 = std::numeric_limits<std::uint64_t>::max();
    IDouble ids1(s1), ids2(s2);
    DOCTEST_REQUIRE(singleton(ids1));  DOCTEST_REQUIRE(lb(ids1) == -1500.0);
    DOCTEST_REQUIRE(singleton(ids2));  DOCTEST_REQUIRE(lb(ids2) == 15540.0);
    IDouble idus(us);
    DOCTEST_REQUIRE(singleton(idus));  DOCTEST_REQUIRE(lb(idus) == 37700.0);
    IDouble idi1(i1), idi2(i2);
    DOCTEST_REQUIRE(singleton(idi1));  DOCTEST_REQUIRE(lb(idi1) == -1110040.0);
    DOCTEST_REQUIRE(singleton(idi2));  DOCTEST_REQUIRE(lb(idi2) == 11121314.0);
    IDouble idui(ui);
    DOCTEST_REQUIRE(singleton(idui));  DOCTEST_REQUIRE(lb(idui) == 2147485648.0);
    IDouble id641(i641);
    DOCTEST_REQUIRE(singleton(id641)); DOCTEST_REQUIRE(lb(id641) == -4611686018427387904.0);
    IDouble id642(i642);
    DOCTEST_REQUIRE(!singleton(id642));
    DOCTEST_REQUIRE(std::int64_t(lb(id642)) < i642);
    DOCTEST_REQUIRE(std::int64_t(ub(id642)) > i642);
    IDouble id643(i643);
    DOCTEST_REQUIRE(!singleton(id643));
    DOCTEST_REQUIRE(std::int64_t(lb(id643)) < i643);
    DOCTEST_REQUIRE(std::int64_t(ub(id643)) > i643);
    IDouble idu641(u641);
    DOCTEST_REQUIRE(!singleton(idu641));
    DOCTEST_REQUIRE(std::uint64_t(lb(idu641)) < u641);
    DOCTEST_REQUIRE(std::uint64_t(ub(idu641)) > u641);
    IDouble idu642(u642);
    DOCTEST_REQUIRE(singleton(idu642)); DOCTEST_REQUIRE(lb(idu642) == 0);
    IDouble idu643(u643);
    DOCTEST_REQUIRE(!singleton(idu643));
    DOCTEST_REQUIRE(ub(idu643) == 18446744073709551616.0);
    DOCTEST_REQUIRE(lb(idu643) == 18446744073709549568.0);
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Double addition") {
    IDouble d11(1.1);
    DOCTEST_REQUIRE(singleton(d11));
    IDouble d55 = d11 + d11 + d11 + d11 + d11;
    DOCTEST_REQUIRE(!singleton(d55));
    DOCTEST_REQUIRE(lb(d55) == 5.5);
    DOCTEST_REQUIRE(ub(d55) == 5.500000000000000888);
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Double subtraction") {
    IDouble d11(1.1);
    IDouble d0(0);
    DOCTEST_REQUIRE(singleton(d0));
    DOCTEST_REQUIRE(singleton(d11));
    DOCTEST_REQUIRE(same(-d11, IDouble(-1.1)));
    DOCTEST_REQUIRE(same(d0 - d11, -d11));
    IDouble d55 = d11 - d11 - d11 - d11 - d11 - d11 - d11;
    DOCTEST_REQUIRE(!singleton(d55));
    DOCTEST_REQUIRE(lb(d55) == -5.500000000000000888);
    DOCTEST_REQUIRE(ub(d55) == -5.5);
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Double addition, subtraction and multiplication randomized") {
    constexpr std::size_t n = 10000;
    std::vector<IDouble> intervals;
    while(intervals.size() < n) {
        auto& cat = categories[category_selection(rng)];
        double d1 = cat(rng), d2 = cat(rng);
        IDouble iv(std::min(d1,d2), std::max(d1,d2));
        if(ub(iv) - lb(iv) > 1.0e-6) {
            intervals.push_back(iv);
        }
    }

    for(IDouble i : intervals) {
        double xi = 0.5 * (lb(i) + ub(i));
        double sinx = std::min(1.0, std::max(-1.0, std::sin(xi))),
               cosx = std::min(1.0, std::max(-1.0, std::cos(xi)));
        IDouble isinx = sin(i);
        IDouble icosx = cos(i);
        DOCTEST_REQUIRE(lb(isinx) <= sinx);
        DOCTEST_REQUIRE(sinx <= ub(isinx));
        DOCTEST_REQUIRE(lb(icosx) <= cosx);
        DOCTEST_REQUIRE(cosx <= ub(icosx));
        for(IDouble j : intervals) {
            double xj = 0.5 * (lb(j) + ub(j));
            IDouble kplus = i + j;
            IDouble kminus = i - j;
            IDouble kmul = i * j;
            double xkp = xi + xj;
            double xkm = xi - xj;
            double xku = xi * xj;
            DOCTEST_REQUIRE(lb(kplus) <= xkp);
            DOCTEST_REQUIRE(xkp <= ub(kplus));
            DOCTEST_REQUIRE(lb(kminus) <= xkm);
            DOCTEST_REQUIRE(xkm <= ub(kminus));
            DOCTEST_REQUIRE(lb(kmul) <= xku);
            DOCTEST_REQUIRE(xku <= ub(kmul));

            if(lb(j) > 0 || ub(j) < 0) {
                IDouble kdiv = i / j;
                double xkd = xi / xj;
                DOCTEST_REQUIRE(lb(kdiv) <= xkd);
                DOCTEST_REQUIRE(xkd <= ub(kdiv));
            } else {
                DOCTEST_REQUIRE((i/j).possibly_undefined());
            }
        }
    }
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Double multiplication") {
    double inf = std::numeric_limits<double>::infinity();
    double mx = std::numeric_limits<double>::max();
    double mn = std::numeric_limits<double>::denorm_min();
    IDouble zero(0.0f);
    IDouble one(1.0, 1.0);
    IDouble mone(-1.0, -1.0);
    IDouble mixed_ex(-2.0, 3.0);
    IDouble pos_ex(2.0, 3.0);
    IDouble neg_ex(-2.0, -1.5);
    IDouble fifth(0.19999999999999998, 0.2);
    IDouble nonneg(0.0, inf);
    IDouble nonpos(-inf, 0.0);
    IDouble pos(1.0, inf);
    IDouble neg(-inf, -1.0);
    IDouble real(-inf, inf);
    double nan = std::numeric_limits<double>::quiet_NaN();
    IDouble undef1(nan, 1.0), undef2(1.0, nan), undef12(nan, nan);
    DOCTEST_REQUIRE(undef1.possibly_undefined());
    DOCTEST_REQUIRE(undef2.possibly_undefined());
    DOCTEST_REQUIRE(undef12.possibly_undefined());
    DOCTEST_REQUIRE(!undef1.definitely_defined());
    DOCTEST_REQUIRE(!undef2.definitely_defined());
    DOCTEST_REQUIRE(!undef12.definitely_defined());
    DOCTEST_REQUIRE((undef1 * undef1).possibly_undefined());
    DOCTEST_REQUIRE((undef1 * one).possibly_undefined());
    DOCTEST_REQUIRE((undef2 * undef2).possibly_undefined());
    DOCTEST_REQUIRE((undef2 * one).possibly_undefined());
    DOCTEST_REQUIRE((one * undef12).possibly_undefined());
    DOCTEST_REQUIRE((undef12 * undef12).possibly_undefined());
    DOCTEST_REQUIRE((undef1 / undef1).possibly_undefined());
    DOCTEST_REQUIRE((undef1 / one).possibly_undefined());
    DOCTEST_REQUIRE((undef2 / undef2).possibly_undefined());
    DOCTEST_REQUIRE((undef2 / one).possibly_undefined());
    DOCTEST_REQUIRE((one / undef12).possibly_undefined());
    DOCTEST_REQUIRE((undef12 / undef12).possibly_undefined());
    DOCTEST_REQUIRE(lb(fifth) < ub(fifth));
    DOCTEST_REQUIRE(same(zero * zero, zero));
    DOCTEST_REQUIRE(same(one * one, one));
    DOCTEST_REQUIRE(same(mone * mone * mone, mone));
    DOCTEST_REQUIRE(same(mixed_ex * mixed_ex, IDouble(-6.0, 9.0)));
    DOCTEST_REQUIRE(same(mixed_ex * pos_ex, IDouble(-6.0, 9.0)));
    DOCTEST_REQUIRE(same(mixed_ex * neg_ex, IDouble(-6.0, 4.0)));
    DOCTEST_REQUIRE(same(pos_ex * mixed_ex, IDouble(-6.0, 9.0)));
    DOCTEST_REQUIRE(same(neg_ex * mixed_ex, IDouble(-6.0, 4.0)));
    DOCTEST_REQUIRE(same(pos_ex * pos_ex, IDouble(4.0, 9.0)));
    DOCTEST_REQUIRE(same(pos_ex * neg_ex, IDouble(-6.0, -3.0)));
    DOCTEST_REQUIRE(same(neg_ex * pos_ex, IDouble(-6.0, -3.0)));
    DOCTEST_REQUIRE(same(neg_ex * neg_ex, IDouble(2.25, 4.0)));
    DOCTEST_REQUIRE(same(5 * fifth, IDouble(0.9999999999999999, 1.0000000000000002)));
    DOCTEST_REQUIRE(same(nonneg * nonpos, nonpos));
    DOCTEST_REQUIRE(same(nonpos * nonpos, nonneg));
    DOCTEST_REQUIRE(same(nonneg * nonneg, nonneg));
    DOCTEST_REQUIRE(same(real * real, real));
    DOCTEST_REQUIRE(same(nonneg * real, real));
    DOCTEST_REQUIRE(same(nonpos * real, real));
    DOCTEST_REQUIRE(same(pos * pos, pos));
    DOCTEST_REQUIRE(same(pos * neg, neg));
    DOCTEST_REQUIRE(same(neg * pos, neg));
    DOCTEST_REQUIRE(same(neg * neg, pos));
    DOCTEST_REQUIRE(same(fifth * real, real));
    DOCTEST_REQUIRE(same(zero * real, zero));
    DOCTEST_REQUIRE(same(mn * real, real));
    DOCTEST_REQUIRE(same(mx * real, real));
    DOCTEST_REQUIRE(same(IDouble(mx) * mx, IDouble(mx, inf)));
    DOCTEST_REQUIRE(same(IDouble(mn) * mn, IDouble(0, mn)));
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Double square root") {
    IDouble zero(0.0, 0.0);
    IDouble one(1.0, 1.0);
    IDouble two(2.0, 2.0);
    IDouble i23(2.0, 3.0);
    IDouble four(4.0, 4.0);
    IDouble mixed(-1.0, 1.0);
    IDouble szero = sqrt(zero);
    DOCTEST_REQUIRE(same(szero, zero));
    IDouble fifth(0.19999999999999998, 0.2);
    DOCTEST_REQUIRE(same(5 * fifth + szero, IDouble(0.9999999999999999, 1.0000000000000002)));
    DOCTEST_REQUIRE(same(sqrt(one), one));
    DOCTEST_REQUIRE(same(sqrt(four), two));
    DOCTEST_REQUIRE(sqrt(mixed).possibly_undefined());
    DOCTEST_REQUIRE(same(sqrt(two), IDouble(1.414213562373095, 1.4142135623730951)));
    DOCTEST_REQUIRE(same(sqrt(i23), IDouble(1.414213562373095, 1.7320508075688774)));
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Double division") {
    IDouble pos(2.5, 5.25);
    IDouble neg(-7.5, -2.25);
    IDouble mix(-9.5, 19.75);
    double inf = std::numeric_limits<double>::infinity();
    IDouble negi(-inf, -1.0);
    IDouble posi(1.0, inf);
    DOCTEST_REQUIRE(same(pos/pos, IDouble(0.47619047619047616, 2.1)));
    DOCTEST_REQUIRE(same(pos/neg, IDouble(-2.3333333333333335, -0.33333333333333331)));
    DOCTEST_REQUIRE(same(neg/pos, IDouble(-3, -0.42857142857142855)));
    DOCTEST_REQUIRE(same(neg/neg, IDouble(0.299999999999999988, 3.3333333333333335)));
    DOCTEST_REQUIRE(same(mix/pos, IDouble(-3.8000000000000003, 7.9)));
    DOCTEST_REQUIRE(same(mix/neg, IDouble(-8.777777777777779, 4.2222222222222223)));
    DOCTEST_REQUIRE((mix/mix).possibly_undefined());
    DOCTEST_REQUIRE(same(negi/pos, IDouble(-inf, -0.19047619047619046)));
    DOCTEST_REQUIRE(same(posi/pos, IDouble(0.1904761904761904656, inf)));
    DOCTEST_REQUIRE(same(negi/negi, IDouble(0.0, inf)));
    DOCTEST_REQUIRE(same(mix/negi, IDouble(-19.75, 9.5)));
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Double integral powers") {
    IDouble pos_int(5.0, 7.0);
    IDouble pos_real(0.25, 0.75);
    IDouble neg_int(-7.0, -5.0);
    IDouble neg_real(-1.25, -0.25);
    IDouble mixed_int(-3.0, 5.0);
    IDouble mixed_real(-1.5, 0.5);
    IDouble sqrt2(1.4142135623730949, 1.4142135623730951);
    DOCTEST_REQUIRE(same(sqrt2, sqrt(IDouble(2))));
    IDouble inputs[] = {pos_int, pos_real, neg_int, neg_real, mixed_int, mixed_real};
    for(IDouble input : inputs) {
        DOCTEST_REQUIRE(same(fixed_pow<0>(input), IDouble(1.0)));
        DOCTEST_REQUIRE(same(fixed_pow<1>(input), input));
    }
    // x^2
    DOCTEST_REQUIRE(same(square(pos_int), IDouble(25, 49)));
    DOCTEST_REQUIRE(same(square(pos_real), IDouble(0.0625, 0.5625)));
    DOCTEST_REQUIRE(same(square(neg_int), IDouble(25, 49)));
    DOCTEST_REQUIRE(same(square(neg_real), IDouble(0.0625, 1.5625)));
    DOCTEST_REQUIRE(same(square(mixed_int), IDouble(0, 25)));
    DOCTEST_REQUIRE(same(square(mixed_real), IDouble(0, 2.25)));
    DOCTEST_REQUIRE(same(square(sqrt2), IDouble(1.999999999999999555, 2.000000000000000444)));
    // x^3
    DOCTEST_REQUIRE(same(cube(pos_int), IDouble(125, 343)));
    DOCTEST_REQUIRE(same(cube(pos_real), IDouble(0.015625, 0.421875)));
    DOCTEST_REQUIRE(same(cube(neg_int), IDouble(-343.0, -125.0)));
    DOCTEST_REQUIRE(same(cube(sqrt2), -cube(-sqrt2)));
    // x^5
    DOCTEST_REQUIRE(same(fixed_pow<5>(pos_int), IDouble(3125, 16807)));
    DOCTEST_REQUIRE(same(fixed_pow<5>(neg_int), IDouble(-16807, -3125)));
    DOCTEST_REQUIRE(same(fixed_pow<5>(mixed_real), IDouble(-7.59375, 0.03125)));
    DOCTEST_REQUIRE(same(fixed_pow<5>(sqrt2), -fixed_pow<5>(-sqrt2)));
    // x^6
    DOCTEST_REQUIRE(same(fixed_pow<6>(pos_int), IDouble(15625, 117649)));
    DOCTEST_REQUIRE(same(fixed_pow<6>(neg_int), IDouble(15625, 117649)));
    DOCTEST_REQUIRE(same(fixed_pow<6>(mixed_real), IDouble(0, 11.390625)));
    DOCTEST_REQUIRE(same(fixed_pow<6>(sqrt2), fixed_pow<6>(-sqrt2)));
    // undefined values
    IDouble undef1(1.0, std::sqrt(-1.0));
    IDouble undef2(std::sqrt(-1.0), 1.0);
    IDouble undef12(std::sqrt(-1.0), std::sqrt(-1.0));
    IDouble undefs[] = {undef1, undef2, undef12};
    for(auto undef : undefs) {
        DOCTEST_REQUIRE(fixed_pow<0>(undef).possibly_undefined());
        DOCTEST_REQUIRE(fixed_pow<1>(undef).possibly_undefined());
        DOCTEST_REQUIRE(fixed_pow<2>(undef).possibly_undefined());
        DOCTEST_REQUIRE(fixed_pow<3>(undef).possibly_undefined());
    }
}

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] Comparisons") {
    IBool f{false, false}, t{true, true}, i{false, true};
    double nan = std::numeric_limits<double>::quiet_NaN();
    IDouble xs[] = {
            {-1.0, 1.0}, {0.0, 1.0}, {0.0, 1.0001},
            {-0.5, 0.5}, {-1.0, 0.0}, {-1.0001, 0.0},
            {-2.0, -1.5}, {1.5, 2.0}, {nan, 10.0},
            {10.0, nan}, {nan, nan}
    };
    constexpr std::size_t num_xs = sizeof(xs) / sizeof(IDouble);
    IDouble x1{-1.0, 1.0};
    IDouble x2{1.0, 2.0};
    DOCTEST_REQUIRE(same(0.25 < x1, i));
    DOCTEST_REQUIRE(same(0.25 > x1, i));
    DOCTEST_REQUIRE(same(0.25 >= x1, i));
    DOCTEST_REQUIRE(same(0.25 <= x1, i));
    DOCTEST_REQUIRE(same(-1.0 < x1, i));
    DOCTEST_REQUIRE(same(-1.0 <= x1, t));
    DOCTEST_REQUIRE(same(-1.0 > x1, f));
    DOCTEST_REQUIRE(same(-1.0 >= x1, i));
    DOCTEST_REQUIRE(same(1.1 < x1, f));
    DOCTEST_REQUIRE(same(1.1 > x1, t));
    DOCTEST_REQUIRE(same(1.1 <= x1, f));
    DOCTEST_REQUIRE(same(1.1 >= x1, t));
    DOCTEST_REQUIRE(same(x1 < x2, i));
    DOCTEST_REQUIRE(same(x1 <= x2, t));
    DOCTEST_REQUIRE(same(x2 < x1, f));
    for(std::size_t ii = 0; ii < num_xs; ++ii) {
        IDouble xi = xs[ii];
        for(std::size_t j = 0; j < num_xs; ++j) {
            IDouble xj = xs[j];
            bool lbltc, ubltc, lblec, ublec;
            bool havenan = (xi.lb() != xi.lb()) | (xi.ub() != xi.ub()) | (xj.lb() != xj.lb()) | (xj.ub() != xj.ub());
            lbltc = (xi.lb() < xj.lb()) & (xi.lb() < xj.ub()) & (xi.ub() < xj.lb()) & (xi.ub() < xj.ub()) & !havenan;
            ubltc = (xi.lb() < xj.lb()) | (xi.lb() < xj.ub()) | (xi.ub() < xj.lb()) | (xi.ub() < xj.ub()) | havenan;
            lblec = (xi.lb() <= xj.lb()) & (xi.lb() <= xj.ub()) & (xi.ub() <= xj.lb()) & (xi.ub() <= xj.ub()) & !havenan;
            ublec = (xi.lb() <= xj.lb()) | (xi.lb() <= xj.ub()) | (xi.ub() <= xj.lb()) | (xi.ub() <= xj.ub()) | havenan;
            IBool exp_lt{lbltc, ubltc};
            IBool exp_le{lblec, ublec};
            DOCTEST_REQUIRE(same(xi < xj, exp_lt));
            DOCTEST_REQUIRE(same(xi <= xj, exp_le));
        }
    }
}
