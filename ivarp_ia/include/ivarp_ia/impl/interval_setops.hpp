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
    inline __m128d join_intervald(__m128d a, __m128d b) {
        __m128d nanmask = _mm_cmpunord_pd(a, b);
        a = _mm_xor_pd(a, SWITCH_UPPER_SIGN128);
        b = _mm_xor_pd(b, SWITCH_UPPER_SIGN128);
        __m128d mins = _mm_min_pd(a, b);
        return _mm_or_pd(nanmask, _mm_xor_pd(mins, SWITCH_UPPER_SIGN128));
    }

    inline __m128d intersect_intervald(__m128d a, __m128d b) {
        __m128d nanmask = _mm_cmpunord_pd(a, b);
        a = _mm_xor_pd(a, SWITCH_UPPER_SIGN128);
        b = _mm_xor_pd(b, SWITCH_UPPER_SIGN128);
        __m128d maxs = _mm_max_pd(a, b);
        return _mm_or_pd(nanmask, _mm_xor_pd(maxs, SWITCH_UPPER_SIGN128));
    }

    inline __m128d max_intervald(__m128d a, __m128d b) {
        __m128d nanmask = _mm_cmpunord_pd(a, b);
        __m128d mmax = _mm_max_pd(a, b);
        return _mm_or_pd(nanmask, mmax);
    }

    inline __m128d min_intervald(__m128d a, __m128d b) {
        __m128d nanmask = _mm_cmpunord_pd(a, b);
        __m128d mmin = _mm_min_pd(a, b);
        return _mm_or_pd(nanmask, mmin);
    }
}
}
