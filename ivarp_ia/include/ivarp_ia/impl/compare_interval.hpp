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


#ifndef IVARP_COMPARE_INTERVAL_HPP
#define IVARP_COMPARE_INTERVAL_HPP

namespace ivarp {
namespace impl {
    // if one of the entries is NaN, make both entries NaN
    static inline __m128d broadcast_nan_intervald(__m128d a) noexcept {
        __m128d nanmask = _mm_cmpunord_pd(a, a);
        __m128d fnanmask = _mm_permute_pd(nanmask, 1);
        a = _mm_or_pd(a, nanmask);
        a = _mm_or_pd(a, fnanmask);
        return a;
    }

    static inline IBool lt_intervald(__m128d a, __m128d b) noexcept {
        IBool result{a[1] < b[0], !(b[1] <= a[0])};
        return result;
    }

    static inline IBool gt_intervald(__m128d a, __m128d b) noexcept {
        IBool result{a[0] > b[1], !(a[1] <= b[0])};
        return result;
    }

    static inline IBool le_intervald(__m128d a, __m128d b) noexcept {
        IBool result{a[1] <= b[0], !(b[1] < a[0])};
        return result;
    }
}
}


#endif //IVARP_COMPARE_INTERVAL_HPP
