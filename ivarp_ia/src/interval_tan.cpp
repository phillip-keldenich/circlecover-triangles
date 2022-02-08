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
#define MPFR_USE_INTMAX_T 1
#include <mpfr.h>

namespace ivarp {
    namespace impl {
        /// Implementation of rounded sine for non-negative floating-point values.
        template<bool RoundUp>
        static inline double round_tan(double x, unsigned /*precision_ignored*/) IVARP_FN_RPURE;
        template<bool RoundUp>
        static inline double round_tan(double x, unsigned /*precision_ignored*/)
        {
            MPFR_DECL_INIT(mx, 53); // NOLINT
            int ter = mpfr_set_d(mx, x, RoundUp ? MPFR_RNDU : MPFR_RNDD);
            assert(ter == 0); (void)ter; // There should not be rounding here.
            mpfr_tan(mx, mx, RoundUp ? MPFR_RNDU : MPFR_RNDD);
            return mpfr_get_d(mx, RoundUp ? MPFR_RNDU : MPFR_RNDD);
        }

        template<typename IT>
            static inline IT interval_tan(const IT& it, unsigned precision)
        {
            const IT& pi_half = get_constants<IT>().pi_half(precision);
            if(it.lb() < -pi_half.lb() || it.ub() > pi_half.lb()) {
                return IT::undefined_value();
            }
            return IT(round_tan<false>(it.lb(), precision), round_tan<true>(it.ub(), precision));
        }
    }

    IDouble tan(IDouble x) noexcept {
        return impl::interval_tan(x, 0);
    }
}
