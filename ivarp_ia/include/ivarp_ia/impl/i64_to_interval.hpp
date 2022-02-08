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

#include <x86intrin.h>
#include <cstdint>
#include <limits>
#include <cfenv>
#include "attributes.hpp"

namespace ivarp {
namespace impl {
    static constexpr std::int64_t MIN_I64 = std::numeric_limits<std::int64_t>::min();
    static const __m128d SWITCH_UPPER_SIGN128 = _mm_castsi128_pd(_mm_set_epi64x(MIN_I64, 0)); // NOLINT

    inline __m128d i64_to_intervald(std::int64_t i) IVARP_FN_PURE;
    inline __m128d i64_to_intervald(std::int64_t i) {
        /* This uses inline ASM because clang and GCC both sometimes compile this wrong if
         * they think they know what happens here, in case of GCC even with -frounding-math
         * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84407), due to constant propagation.
         * This inline asm should result in exactly the same output the compiler produces if
         * everything works correctly. */
        __m128i x, tmp;
        asm("vmovq %2, %0\n"
            "negq %2\n"
            "vmovq %2, %1\n"
            "vpunpcklqdq %1, %0, %0\n" : "=x"(x), "=x"(tmp), "+r"(i));
        // x should now contain [i,-i]
        __m128i xH = _mm_srai_epi32(x, 16);
        xH = _mm_blend_epi16(xH, _mm_setzero_si128(), 0x33);
        xH = _mm_add_epi64(xH, _mm_castpd_si128(_mm_set1_pd(442721857769029238784.)));
        __m128i xL = _mm_blend_epi16(x, _mm_castpd_si128(_mm_set1_pd(0x0010000000000000)), 0x88);
        __m128d f = _mm_sub_pd(_mm_castsi128_pd(xH), _mm_set1_pd(442726361368656609280.));
        f = _mm_add_pd(f, _mm_castsi128_pd(xL));
        asm("vxorpd %1, %0, %0" : "+x"(f) : "m"(SWITCH_UPPER_SIGN128));
        return f;
    }

    inline __m128d u64_to_intervald(std::uint64_t i) IVARP_FN_PURE;
    inline __m128d u64_to_intervald(std::uint64_t i) {
        /* This uses inline ASM because clang and GCC both sometimes compile this wrong if
         * they think they know what happens here, in case of GCC even with -frounding-math
         * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84407) due to constant propagation.
         * This inline asm should result in exactly the same output the compiler produces if
         * everything works correctly. */
        const __m128i two52 = _mm_castpd_si128(_mm_set_pd(-4503599627370496., 4503599627370496.));
        const __m128i two84 = _mm_castpd_si128(_mm_set_pd(-19342813113834066795298816., 19342813113834066795298816.));
        const __m128d sum8452 = _mm_set_pd(-19342813118337666422669312., 19342813118337666422669312.);
        __m128i x;
        asm("vmovq %1, %0\n"
            "vpshufd $68, %0, %0\n" : "=x"(x) : "r"(i));
        __m128i xH = _mm_srli_epi64(x, 32);
        xH = _mm_or_si128(xH, two84);
        __m128i xL = _mm_blend_epi16(x, two52, 0xcc);
        __m128d f = _mm_sub_pd(_mm_castsi128_pd(xH), sum8452);
        f = _mm_add_pd(f, _mm_castsi128_pd(xL));
        asm("vxorpd %1, %0, %0" : "+x"(f) : "m"(SWITCH_UPPER_SIGN128));
        return f;
    }
}
}
