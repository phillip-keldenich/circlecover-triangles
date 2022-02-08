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

#ifndef IVARP_DEFAULT_IRRATIONAL_PRECISION
#define IVARP_DEFAULT_IRRATIONAL_PRECISION 256
#endif

namespace ivarp {
    template<typename IntervalType> class RealConstantCache {

    };

    template<typename IntervalType> class ConstantCache {
    public:
        ConstantCache() : m_real(&get_real()) {}

    private:
        static RealConstantCache<IntervalType>& get_real() {
            static RealConstantCache<IntervalType> real;
            return real;
        }

        RealConstantCache<IntervalType> *m_real;
    };

    template<> class ConstantCache<IDouble> {
    public:
        // π
        inline IDouble pi(unsigned /*ignored_precision*/) const noexcept {
            return IDouble(
                3.1415926535897931159979634685441851615905761718750/*0...*/,
             /* 3.141592653589793238... (π) */
                3.1415926535897935600871733186068013310432434082031250/*0...*/
            );
        }

        // π/2
        inline IDouble pi_half(unsigned /*ignored_precision*/) const noexcept {
            return IDouble(
                1.57079632679489655799898173427209258079528808593750/*0...*/,
             /* 1.570796326794896619... (π/2) */
                1.57079632679489678004358665930340066552162170410156250/*0...*/
            );
        }

        //  1/(2π)
        inline IDouble rec_2pi(unsigned /*ignored_precision*/) const noexcept {
            return IDouble(
                0.1591549430918953178526464853348443284630775451660156250/*0...*/,
             /* 0.159154943091895335... (1/(2π)) */
                0.15915494309189534560822210096375783905386924743652343750/*0...*/
            );
        }
    };

    template<typename IntervalType> inline auto get_constants() noexcept {
        return ConstantCache<IntervalType>{};
    }
}
