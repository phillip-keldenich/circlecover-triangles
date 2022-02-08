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

#include "add_interval.hpp"

namespace ivarp {
namespace impl {
    static const __m128d NAN_INTERVAL = _mm_castsi128_pd(_mm_set1_epi64x(std::int64_t(-1))); // NOLINT
    static const __m128d SWITCH_LOWER_SIGN128 = _mm_castsi128_pd( // NOLINT
        _mm_set_epi64x(0, std::numeric_limits<std::int64_t>::min())
    );
    static const __m256d SWITCH_ALL_SIGNS256 = _mm256_castsi256_pd( // NOLINT
        _mm256_set1_epi64x(std::numeric_limits<std::int64_t>::min())
    );

    inline __m128d horizontal_min(__m256d x) noexcept IVARP_FN_PURE;
    __m128d horizontal_min(__m256d x) noexcept {
        __m128d vlow  = _mm256_castpd256_pd128(x);
        __m128d vhigh = _mm256_extractf128_pd(x, 1);
        vlow = _mm_min_pd(vlow, vhigh);
        __m128d high64 = _mm_unpackhi_pd(vlow, vlow);
        return _mm_min_sd(vlow, high64);
    }

    inline __m128d horizontal_max128(__m128d x) noexcept IVARP_FN_PURE;
    __m128d horizontal_max128(__m128d x) noexcept {
        __m128d low = x;
        __m128d high = _mm_unpackhi_pd(x, x);
        return _mm_max_sd(high, low);
    }

    inline __m128d horizontal_sort(__m128d x) noexcept IVARP_FN_PURE;
    __m128d horizontal_sort(__m128d x) noexcept {
        __m128d low = x;
        __m128d high = _mm_unpackhi_pd(x, x);
        __m128d mn = _mm_min_sd(low, high);
        __m128d mx = _mm_max_sd(low, high);
        return _mm_set_pd(mx[0], mn[0]);
    }

    inline __m128d mul_intervald(__m128d a, __m128d b) noexcept IVARP_FN_PURE;
    __m128d mul_intervald(__m128d a, __m128d b) noexcept {
        if(__builtin_expect(_mm_movemask_pd(_mm_cmpunord_pd(a, b)), 0)) {
            return NAN_INTERVAL;
        }
        /* lhs_rd = [a_lb, a_lb, a_ub, a_ub] */
        __m256d lhs_rd = _mm256_permute4x64_pd(_mm256_castpd128_pd256(a), 0x50);
        /* lhs_ru = [-a_lb, -a_lb, -a_ub, -a_ub] */
        __m256d lhs_ru = _mm256_xor_pd(lhs_rd, SWITCH_ALL_SIGNS256);
        /* rhs = [b_lb, b_ub, b_lb, b_ub] (= 2*[b_lb,b_ub]) */
        __m256d rhs =  _mm256_insertf128_pd(_mm256_castpd128_pd256(b), b, 1);
        /* multiply and replace NaNs from 0*infinity by 0;
         * use inline asm to hide the rounding instructions
         * from the compiler (prevent compiler bugs - see addition). */
        asm("vmulpd %0, %1, %0" : "+x"(lhs_rd) : "x"(rhs));
        asm("vmulpd %0, %1, %0" : "+x"(lhs_ru) : "x"(rhs));
        __m256d nanmask = _mm256_cmp_pd(lhs_rd, _mm256_set1_pd(0.0), _CMP_ORD_Q);
        lhs_rd = _mm256_and_pd(lhs_rd, nanmask);
        lhs_ru = _mm256_and_pd(lhs_ru, nanmask);
        /* compute minimum entry in the vectors */
        __m128d rdmin = horizontal_min(lhs_rd);
        __m128d rumin = horizontal_min(lhs_ru);
        /* switch sign of upper bound and extract result */
        __m128d rumax = _mm_xor_pd(rumin, SWITCH_LOWER_SIGN128);
        return _mm_set_pd(rumax[0], rdmin[0]);
    }

    template<unsigned N> struct PowSingle {
        static double compute(double x, std::true_type) noexcept IVARP_FN_PURE {
            double xsq = PowSingle<N/2>::compute(x);
            double y = -xsq;
            asm("vmulsd %0, %1, %0" : "+x"(xsq) : "x"(y));
            asm("vmulsd %0, %1, %0" : "+x"(xsq) : "x"(x));
            return -xsq;
        }

        static double compute(double x, std::false_type) noexcept IVARP_FN_PURE {
            x = PowSingle<N/2>::compute(x);
            double y = -x;
            asm("vmulsd %0, %1, %0" : "+x"(x) : "x"(y));
            return -x;
        }

        static double compute(double x) {
            return compute(x, std::integral_constant<bool, N%2 == 1>{});
        }
    };
    template<> struct PowSingle<1u> {
        static double compute(double x) noexcept {
            return x;
        }
    };
    template<> struct PowSingle<0u> {
        static double compute(double x) noexcept {
            return 1.0;
        }
    };

    template<unsigned N> struct PowInterval {
        static __m128d compute(__m128d x, __m128d signswap, std::integral_constant<unsigned, 0>) noexcept IVARP_FN_PURE
        {
            __m128d xsq = PowInterval<N/2>::compute(x, signswap);
            __m128d y = _mm_xor_pd(xsq, signswap);
            asm("vmulpd %0, %1, %0" : "+x"(xsq) : "x"(y));
            return xsq;
        }

        static __m128d compute(__m128d x, __m128d signswap, std::integral_constant<unsigned, 1>) noexcept IVARP_FN_PURE
        {
            __m128d xsq = PowInterval<N/2>::compute(x, signswap);
            __m128d y = _mm_xor_pd(xsq, signswap);
            asm("vmulpd %0, %1, %0" : "+x"(xsq) : "x"(y));
            asm("vmulpd %0, %1, %0" : "+x"(xsq) : "x"(x));
            return xsq;
        }

        static __m128d compute(__m128d x, __m128d signswap) {
            return compute(x, signswap, std::integral_constant<unsigned, N%2>{});
        }
    };
    template<> struct PowInterval<1u> {
        static __m128d compute(__m128d x, __m128d) {
            return x;
        }
    };
    template<> struct PowInterval<0u> {
        static __m128d compute(__m128d x, __m128d) noexcept {
            return _mm_set1_pd(1.0);
        }
    };

    template<unsigned N> double pow_ru_single(double x) noexcept IVARP_FN_PURE;
    template<unsigned N> double pow_ru_single(double x) noexcept {
        return PowSingle<N>::compute(x);
    }

    template<unsigned N>
    inline __m128d fixed_pow(__m128d x) noexcept IVARP_FN_PURE;
    template<unsigned N>
    __m128d fixed_pow(__m128d x) noexcept
    {
        if(N == 0) {
            if(__builtin_expect(x[0] != x[0] || x[1] != x[1], 0)) {
                return NAN_INTERVAL;
            }
            return _mm_set1_pd(1.0);
        } else if(N == 1) {
            return x;
        } else if(N % 2 == 0) {
            __m128d masked_sign = _mm_andnot_pd(SWITCH_BOTH_SIGNS128, x);
            if(x[0] <= 0.0 && x[1] >= 0.0) {
                return _mm_set_pd(pow_ru_single<N>(horizontal_max128(masked_sign)[0]), 0.0);
            } else {
                if(__builtin_expect(x[0] != x[0] || x[1] != x[1], 0)) {
                    return NAN_INTERVAL;
                }
                __m128d signswitch = SWITCH_UPPER_SIGN128;
                __m128d sorted = horizontal_sort(masked_sign);
                return _mm_xor_pd(PowInterval<N>::compute(sorted, signswitch), signswitch);
            }
        } else {
            __m128d masked_sign = _mm_andnot_pd(SWITCH_BOTH_SIGNS128, x);
            __m128d signs = _mm_and_pd(SWITCH_BOTH_SIGNS128, x);
            signs = _mm_xor_pd(SWITCH_UPPER_SIGN128, signs);
            x = PowInterval<N>::compute(masked_sign, signs);
            return _mm_xor_pd(x, SWITCH_UPPER_SIGN128);
        }
    }
}
}
