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
#include "period_reduction.hpp"
#define MPFR_USE_INTMAX_T 1
#include <mpfr.h>

namespace ivarp {
namespace impl {
    /// Implementation of rounded cosine for nonnegative floating-point values.
    template<bool RoundUp> static double round_cos(double x, unsigned /*precision_ignored*/)
    {
        MPFR_DECL_INIT(mx, 53); // NOLINT
        int ter = mpfr_set_d(mx, x, RoundUp ? MPFR_RNDD : MPFR_RNDU);
        assert(ter == 0); (void)ter;
        mpfr_cos(mx, mx, RoundUp ? MPFR_RNDU : MPFR_RNDD);
        return mpfr_get_d(mx, RoundUp ? MPFR_RNDU : MPFR_RNDD);
    }

    /// Implementation of interval cosine for interval that do not wrap across a multiple of 2pi.
    template<typename IT> static inline IT
        interval_cos_nowrap(const PositivePeriodReduction<IT>& period, const IT& x, unsigned precision)
    {
        using Bound = BoundType<IT>;
        if(period.lb_period_fractional <= 0.5) {
            if(period.ub_period_fractional <= 0.5) {
                return IT{round_cos<false>(ub(x), precision), round_cos<true>(lb(x), precision)};
            } else {
                return IT{Bound(-1), (std::max)(round_cos<true>(ub(x), precision), round_cos<true>(lb(x), precision))};
            }
        } else {
            return IT{round_cos<false>(lb(x), precision), round_cos<true>(ub(x), precision)};
        }
    }

    /// Implementation of interval cosine for intervals for which the lower bound is one 2pi period before the upper bound.
    template<typename IT> static inline IT
        interval_cos_wrap(const PositivePeriodReduction<IT>& period, const IT& x, unsigned precision)
    {
        using Bound = BoundType<IT>;
        if(period.lb_period_fractional <= 0.5 || period.ub_period_fractional >= 0.5) {
            return IT{Bound(-1), Bound(1)};
        } else {
            return IT{(std::min)(round_cos<false>(lb(x), precision), round_cos<false>(ub(x), precision)), Bound(1)};
        }
    }

    /// Implementation of cosine for non-negative intervals.
    template<typename IT> static inline IT
        interval_cos_nonnegative(const IT& x, unsigned precision)
    {
        using Bound = BoundType<IT>;
        PositivePeriodReduction<IT> period = positive_period_reduction(x, get_constants<IT>().rec_2pi(precision));
        // the bounds are not definitely in adjacent periods
        if(add_rd(period.lb_period_integral, Bound(1)) < period.ub_period_integral) {
            return IT{Bound(-1), Bound(1)};
        }

        if(period.lb_period_integral == period.ub_period_integral) {
            // no wrap-around
            return interval_cos_nowrap(period, x, precision);
        } else {
            // wrap-around
            return interval_cos_wrap(period, x, precision);
        }
    }

    template<typename IT> static inline
        IT interval_cos_symm(const IT& x, unsigned precision)
    {
        using Bound = BoundType<IT>;
        if(!is_finite(x) || possibly_undefined(x)) {
            return IT(Bound(-1), Bound(1), possibly_undefined(x));
        }

        if(ub(x) <= 0) {
            return interval_cos_nonnegative(-x, precision);
        } else if(lb(x) < 0) {
            Bound mx = -lb(x);
            if(mx < ub(x)) {
                mx = ub(x);
            }
            return interval_cos_nonnegative(IT{0, mx}, precision);
        } else {
            return interval_cos_nonnegative(x, precision);
        }
    }
}
    IDouble cos(IDouble x) noexcept {
        return impl::interval_cos_symm(x, 0);
    }
}
