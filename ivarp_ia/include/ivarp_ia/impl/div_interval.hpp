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

#include "mul_interval.hpp"

namespace ivarp {
namespace impl {
    static const __m128d POSITIVE_INF128 = _mm_set1_pd(std::numeric_limits<double>::infinity()); // NOLINT

    __m128d div_intervald_with_infinities(__m128d num, __m128d den) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    inline __m128d div_intervald(__m128d num, __m128d den) noexcept IVARP_FN_PURE;
    __m128d div_intervald(__m128d num, __m128d den) noexcept {
        __m128d smask_num = _mm_andnot_pd(SWITCH_BOTH_SIGNS128, num);
        __m128d smask_den = _mm_andnot_pd(SWITCH_BOTH_SIGNS128, den);
        if(__builtin_expect(_mm_movemask_pd(_mm_cmpunord_pd(num, den)), 0)) {
            /* found NaN in the input */
            return NAN_INTERVAL;
        }
        int num_inf = _mm_movemask_pd(_mm_cmpeq_pd(smask_num, POSITIVE_INF128));
        int den_inf = _mm_movemask_pd(_mm_cmpeq_pd(smask_den, POSITIVE_INF128));
        if(__builtin_expect(den[0] <= 0.0 && den[1] >= 0.0, 0)) {
            return NAN_INTERVAL;
        }
        if(__builtin_expect(num_inf | den_inf, 0)) {
            return div_intervald_with_infinities(num, den);
        }
        /* num_rd = [a_lb, a_lb, a_ub, a_ub] */
        __m256d num_rd = _mm256_permute4x64_pd(_mm256_castpd128_pd256(num), 0x50);
        /* num_ru = [-a_lb, -a_lb, -a_ub, -a_ub] */
        __m256d num_ru = _mm256_xor_pd(num_rd, SWITCH_ALL_SIGNS256);
        /* rhs = [b_lb, b_ub, b_lb, b_ub] (= 2*[b_lb,b_ub]) */
        __m256d den_both =  _mm256_insertf128_pd(_mm256_castpd128_pd256(den), den, 1);
        /* divide; use inline asm to hide the rounding instructions
         * from the compiler (prevent compiler bugs - see addition). */
        asm("vdivpd %1, %0, %0" : "+x"(num_rd) : "x"(den_both));
        asm("vdivpd %1, %0, %0" : "+x"(num_ru) : "x"(den_both));
        /* compute minimum entry in the vectors */
        __m128d rdmin = horizontal_min(num_rd);
        __m128d rumin = horizontal_min(num_ru);
        /* switch sign of upper bound and extract result */
        __m128d rumax = _mm_xor_pd(rumin, SWITCH_LOWER_SIGN128);
        return _mm_set_pd(rumax[0], rdmin[0]);
    }
}
}
