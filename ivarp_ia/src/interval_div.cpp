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

namespace ivarp {
namespace impl {
    __m128d div_intervald_with_infinities(__m128d num, __m128d den) noexcept {
        double lbnum = num[0], ubnum = num[1];
        double lbden = den[0], ubden = den[1];
        if(lbden < 0.0) {
            std::swap(lbden, ubden);
            lbden = -lbden;
            ubden = -ubden;
            std::swap(lbnum, ubnum);
            lbnum = -lbnum;
            ubnum = -ubnum;
        }

        __m128d act_num, act_den;
        if(lbnum >= 0.0) {
            /* possible infinities: ubnum, ubden */
            act_num = _mm_set_pd(-ubnum, lbnum);
            act_den = _mm_set_pd(lbden, ubden);
        } else if(ubnum <= 0.0) {
            /* possible infinities: lbnum, ubden */
            act_num = _mm_set_pd(-ubnum, lbnum);
            act_den = _mm_set_pd(ubden, lbden);
        } else {
            /* possible infinities: ubnum, lbnum, ubden */
            act_num = _mm_set_pd(-ubnum, lbnum);
            act_den = _mm_set_pd(lbden, lbden);
        }
        asm("vdivpd %1, %0, %0" : "+x"(act_num) : "x"(act_den));
        return _mm_xor_pd(act_num, SWITCH_UPPER_SIGN128);
    }
}
}

