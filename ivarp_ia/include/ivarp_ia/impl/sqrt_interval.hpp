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

#include "i64_to_interval.hpp"

namespace ivarp {
namespace impl {
    inline __m128d sqrt_intervald(__m128d x) noexcept IVARP_FN_PURE;
    __m128d sqrt_intervald(__m128d x) noexcept {
        /* the most problematic part of this function (w.r.t. compiler bugs) is
         * the compiler moving rounding-mode setting across the sqrt operation. */
        std::uint32_t mxcsr;
        double lb = x[0], ub = x[1];
        asm("stmxcsr %0\n"
            "vsqrtsd %1, %1, %1\n"
            "xorl $0x6000, %0\n"
            "ldmxcsr %0\n"
            "vsqrtsd %2, %2, %2\n"
            "xorl $0x6000, %0\n"
            "ldmxcsr %0\n" : "=m"(mxcsr), "+x"(lb), "+x"(ub));
        return _mm_set_pd(ub, lb);
    }
}
}
