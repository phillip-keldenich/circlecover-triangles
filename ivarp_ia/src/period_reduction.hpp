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

namespace ivarp {
namespace impl {
    template<typename IntervalType>
    struct PositivePeriodReduction
    {
        using Bound = BoundType<IntervalType>;
        IntervalType period; // the beginning and end (in full periods)
        Bound lb_period_fractional,
              ub_period_fractional, // the position in the period (in [0,1)) of the beginning and end
              lb_period_integral,
              ub_period_integral; // the number of full periods between 0 and the beginning and end
    };

    template<typename IT>
    inline PositivePeriodReduction<IT> positive_period_reduction(IT x, IT one_over_period)
    {
        using Bound = typename PositivePeriodReduction<IT>::Bound;
        IT result = x * one_over_period;
        std::pair<Bound, Bound> lb_split = ivarp::modf(lb(result)), ub_split = ivarp::modf(ub(result));
        return PositivePeriodReduction<IT>{result, lb_split.second, ub_split.second, lb_split.first, ub_split.first};
    }
}
}
