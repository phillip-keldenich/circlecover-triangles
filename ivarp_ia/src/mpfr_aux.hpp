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

#include <mpfr.h>
#include <stdexcept>
#include <exception>

namespace ivarp {
namespace impl {
    /**
     * @class DynamicMPFRNumber
     * An MPFR number with dynamic (i.e., controlled-at-runtime) precision.
     * This class is a really thin wrapper around the bare MPFR type that basically
     * just handles memory allocation/deallocation.
     */
    class DynamicMPFRNumber {
    public:
        DynamicMPFRNumber(unsigned precision) {
            mpfr_init2(m_num, precision);
            if(!m_num->_mpfr_d) {
                throw std::bad_alloc(); // LCOV_EXCL_LINE
            }
        }

        ~DynamicMPFRNumber() noexcept {
            if(m_num->_mpfr_d) {
                mpfr_clear(m_num);
            }
        }

        DynamicMPFRNumber(const DynamicMPFRNumber& c) :
            DynamicMPFRNumber(mpfr_get_prec(c))
        {
            mpfr_set(m_num, c.m_num, MPFR_RNDN);
        }

        DynamicMPFRNumber &operator=(const DynamicMPFRNumber& c) {
            if(&c != this) {
                if(!m_num->_mpfr_d) {
                    mpfr_init2(m_num, mpfr_get_prec(c));
                    if(!m_num->_mpfr_d) {
                        throw std::bad_alloc(); // LCOV_EXCL_LINE
                    }
                } else if(mpfr_get_prec(m_num) < mpfr_get_prec(c.m_num)) {
                    mpfr_set_prec(m_num, mpfr_get_prec(c.m_num));
                    if(!m_num->_mpfr_d) {
                        throw std::bad_alloc(); // LCOV_EXCL_LINE
                    }
                }
                mpfr_set(m_num, c.m_num, MPFR_RNDN);
            }
            return *this;
        }

        DynamicMPFRNumber(DynamicMPFRNumber&& num) noexcept {
            m_num[0] = num.m_num[0];
            num.m_num->_mpfr_d = nullptr;
        }

        DynamicMPFRNumber &operator=(DynamicMPFRNumber&& num) noexcept {
            std::swap(m_num[0], num.m_num[0]);
            return *this;
        }

        operator mpfr_ptr() noexcept // NOLINT
        {
            return m_num;
        }

        operator mpfr_srcptr() const noexcept // NOLINT
        {
            return m_num;
        }

        mpfr_ptr operator->() noexcept {
            return m_num;
        }

        mpfr_srcptr operator->() const noexcept {
            return m_num;
        }

    private:
        mpfr_t m_num;
    };
}
}
