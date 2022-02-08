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
    /// Implementation of rounded sine for non-negative floating-point values.
    template<bool RoundUp>
    static inline double round_sin(double x, unsigned /*precision_ignored*/) IVARP_FN_RPURE;
    template<bool RoundUp>
    static inline double round_sin(double x, unsigned /*precision_ignored*/)
    {
        MPFR_DECL_INIT(mx, 53); // NOLINT
        int ter = mpfr_set_d(mx, x, RoundUp ? MPFR_RNDU : MPFR_RNDD);
        assert(ter == 0); (void)ter; // There should not be rounding here.
        mpfr_sin(mx, mx, RoundUp ? MPFR_RNDU : MPFR_RNDD);
        return mpfr_get_d(mx, RoundUp ? MPFR_RNDU : MPFR_RNDD);
    }

    /// Implementation of interval sine for intervals that do not wrap across a multiple of 2pi.
    template<typename IT> static inline IT
        interval_sin_nowrap(const PositivePeriodReduction<IT>& period, const IT& x, unsigned precision)
    {
        using Bound = BoundType<IT>;
        if(period.lb_period_fractional <= 0.25) {
            if(period.ub_period_fractional < 0.25) {
                return IT{round_sin<false>(lb(x), precision), round_sin<true>(ub(x), precision)};
            } else if(period.ub_period_fractional < 0.75) {
                return IT{(std::min)(round_sin<false>(lb(x), precision), round_sin<false>(ub(x), precision)), Bound(1)};
            } else {
                return IT{Bound(-1), Bound(1)};
            }
        } else {
            if(period.ub_period_fractional < 0.75) {
                return IT{round_sin<false>(ub(x), precision), round_sin<true>(lb(x), precision)};
            } else if(period.lb_period_fractional <= 0.75) {
                return IT{Bound(-1), (std::max)(round_sin<true>(lb(x), precision), round_sin<true>(ub(x), precision))};
            } else {
                return IT{round_sin<false>(lb(x), precision), round_sin<true>(ub(x), precision)};
            }
        }
    }

    /// Implementation of interval sine for intervals for which the lower bound is one 2pi period before the upper bound.
    template<typename IT> static inline IT
        interval_sin_wrap(const PositivePeriodReduction<IT>& period, const IT& x, unsigned precision)
    {
        using Bound = BoundType<IT>;
        if(period.lb_period_fractional <= 0.25) {
            return IT{Bound(-1), Bound(1)};
        } else if(period.lb_period_fractional <= 0.75) {
            if(period.ub_period_fractional < 0.25) {
                return IT{Bound(-1), (std::max)(round_sin<true>(lb(x), precision), round_sin<true>(ub(x), precision))};
            } else {
                return IT{Bound(-1), Bound(1)};
            }
        } else {
            if(period.ub_period_fractional < 0.25) {
                return IT{round_sin<false>(lb(x), precision), round_sin<true>(ub(x), precision)};
            } else if(period.ub_period_fractional < 0.75) {
                return IT{(std::min)(round_sin<false>(lb(x), precision), round_sin<false>(ub(x), precision)), Bound(1)};
            } else {
                return IT{Bound(-1), Bound(1)};
            }
        }
    }

    /// Compute interval sine for non-negative intervals.
    template<typename IT> static inline IT
        interval_sin_nonnegative(const IT& x, unsigned precision)
    {
        using Bound = BoundType<IT>;
        // determine which period we are in (and where we are)
        PositivePeriodReduction<IT> period = positive_period_reduction(x, get_constants<IT>().rec_2pi(precision));
        // the bounds are not definitely in adjacent periods
        if(add_rd(period.lb_period_integral, Bound(1)) < period.ub_period_integral) {
            return IT{Bound(-1), Bound(1)};
        }

        if(period.lb_period_integral == period.ub_period_integral) {
            // no wrap-around
            return interval_sin_nowrap(period, x, precision);
        } else {
            // wrap-around
            return interval_sin_wrap(period, x, precision);
        }
    }

    /// Use sine's symmetry to get rid of all negative input values.
    template<typename IT>
    static inline IT interval_sin_symm(const IT& x, unsigned precision) {
        using Bound = BoundType<IT>;
        if(!is_finite(x) || possibly_undefined(x)) {
            return IT(Bound(-1), Bound(1), possibly_undefined(x));
        }

        const auto& l = lb(x);
        const auto& u = ub(x);
        if(u <= 0) {
            // negative
            return -interval_sin_nonnegative(-x, precision);
        } else if(l < 0) {
            // mixed
            IT rneg = -interval_sin_nonnegative(IT{0, -l}, precision);
            IT rpos = interval_sin_nonnegative(IT{0, u}, precision);
            return join(rpos, rneg);
        } else {
            // positive
            return interval_sin_nonnegative(x, precision);
        }
    }
}

    IDouble sin(IDouble x) noexcept {
        return impl::interval_sin_symm(x, 0);
    }
}
