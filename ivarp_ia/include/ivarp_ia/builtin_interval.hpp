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

#include "meta.hpp"
#include "ibool.hpp"
#include "impl/i64_to_interval.hpp"
#include "impl/add_interval.hpp"
#include "impl/mul_interval.hpp"
#include "impl/sqrt_interval.hpp"
#include "impl/div_interval.hpp"
#include "impl/interval_setops.hpp"
#include "impl/compare_interval.hpp"

namespace ivarp {
    class alignas(16) IDouble {
    public:
        explicit IDouble() noexcept {}

        explicit IDouble(double value) noexcept :
            m_data(_mm_set1_pd(value))
        {}

        explicit IDouble(float value) noexcept :
            m_data(_mm_set1_pd(static_cast<double>(value)))
        {}

        template<typename ShortIntType,
                 Enabler<std::is_integral<ShortIntType>::value &&
                         (sizeof(ShortIntType) * CHAR_BIT < 53)> = 0>
        explicit IDouble(ShortIntType s) noexcept :
            m_data(_mm_set1_pd(static_cast<double>(s)))
        {}

        explicit IDouble(std::int64_t i) noexcept :
            m_data(impl::i64_to_intervald(i))
        {}

        explicit IDouble(std::uint64_t i) noexcept :
            m_data(impl::u64_to_intervald(i))
        {}

        IDouble(double l, double u) noexcept :
            m_data(_mm_set_pd(u, l))
        {}

        explicit IDouble(double l, double u, bool poss_undef) noexcept :
            m_data(_mm_set_pd(poss_undef ? std::numeric_limits<double>::quiet_NaN() : u, l))
        {}

        double lb() const noexcept {
            return m_data[0];
        }

        void set_lb(double l) noexcept {
            m_data[0] = l;
        }

        void set_ub(double u) noexcept {
            m_data[1] = u;
        }

        double ub() const noexcept {
            return m_data[1];
        }

        IDouble min IVARP_NO_MACRO (IDouble other) const noexcept {
            return IDouble(impl::min_intervald(m_data, other.m_data));
        }

        IDouble max IVARP_NO_MACRO (IDouble other) const noexcept {
            return IDouble(impl::max_intervald(m_data, other.m_data));
        }

        double center() const noexcept {
            return 0.5 * (lb() + ub());
        }

        IDouble sqrt() const noexcept {
            return IDouble(impl::sqrt_intervald(m_data));
        }

        bool possibly_undefined() const noexcept {
            // this looks bad, but should compile
            // to two instructions (vpermilp/vshufpd + ucomisd) with no branches
            // and result in the parity-flag;
            // unfortunately, there does not seem to be a way
            // except for inline assembly to enforce that
            // (no intrinsic exposes the parity bit after ucomisd)
            return m_data[0] != m_data[0] || m_data[1] != m_data[1];
        }

        bool definitely_defined() const noexcept {
            return !possibly_undefined();
        }

        bool is_finite() const noexcept {
            return std::isfinite(m_data[0]) && std::isfinite(m_data[1]);
        }

        bool restrict_lb(double value) noexcept {
            if(value > lb()) {
                set_lb(value);
                return true;
            }
            return false;
        }

        bool restrict_ub(double value) noexcept {
            if(value < ub()) {
                set_ub(value);
                return true;
            }
            return false;
        }

        IDouble& operator+=(IDouble other) noexcept {
            m_data = impl::add_intervald(m_data, other.m_data);
            return *this;
        }

        IDouble& operator-=(IDouble other) noexcept {
            m_data = impl::sub_intervald(m_data, other.m_data);
            return *this;
        }

        IDouble& operator*=(IDouble other) noexcept {
            m_data = impl::mul_intervald(m_data, other.m_data);
            return *this;
        }

        IDouble& operator/=(IDouble other) noexcept {
            m_data = impl::div_intervald(m_data, other.m_data);
            return *this;
        }

        IDouble operator-() const noexcept {
            return IDouble(impl::negate_intervald(m_data));
        }

        IDouble operator+() const noexcept {
            return *this;
        }

        template<unsigned N> IDouble fixed_pow() const noexcept {
            return IDouble(impl::fixed_pow<N>(m_data));
        }

        IDouble join(IDouble y) const noexcept {
            return IDouble(impl::join_intervald(m_data, y.m_data));
        }

        IDouble intersection(IDouble y) const noexcept {
            return IDouble(impl::intersect_intervald(m_data, y.m_data));
        }

        IBool operator<(IDouble other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = impl::broadcast_nan_intervald(other_.m_data);
            return impl::lt_intervald(self, other);
        }

        IBool operator>(IDouble other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = impl::broadcast_nan_intervald(other_.m_data);
            return impl::gt_intervald(self, other);
        }

        IBool operator<=(IDouble other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = impl::broadcast_nan_intervald(other_.m_data);
            return impl::le_intervald(self, other);
        }

        IBool operator>=(IDouble other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = impl::broadcast_nan_intervald(other_.m_data);
            return impl::le_intervald(other, self);
        }

        template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
        IBool operator<(IntOrFloatType other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = IDouble(other_).m_data;
            return impl::lt_intervald(self, other);
        }

        template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
        IBool operator>(IntOrFloatType other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = IDouble(other_).m_data;
            return impl::gt_intervald(self, other);
        }

        template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
        IBool operator<=(IntOrFloatType other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = IDouble(other_).m_data;
            return impl::le_intervald(self, other);
        }

        template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
        IBool operator>=(IntOrFloatType other_) const noexcept {
            __m128d self = impl::broadcast_nan_intervald(m_data);
            __m128d other = IDouble(other_).m_data;
            return impl::le_intervald(other, self);
        }

        static IDouble undefined_value() noexcept {
            return IDouble(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
        }

    private:
        explicit IDouble(__m128d rep) noexcept :
            m_data(rep)
        {}

        __m128d m_data;
    };

    inline IDouble operator+(IDouble x, IDouble y) noexcept {
        x += y;
        return x;
    }

    inline IDouble operator-(IDouble x, IDouble y) noexcept {
        x -= y;
        return x;
    }

    inline IDouble operator*(IDouble x, IDouble y) noexcept {
        x *= y;
        return x;
    }

    inline IDouble operator/(IDouble x, IDouble y) noexcept {
        x /= y;
        return x;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator+(const IntOrFloatType& x, IDouble y) noexcept {
        y += IDouble(x);
        return y;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator+(IDouble x, const IntOrFloatType& y) noexcept {
        x += IDouble(y);
        return x;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator-(const IntOrFloatType& x, IDouble y) noexcept {
        IDouble ix(x);
        ix -= y;
        return ix;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator-(IDouble x, const IntOrFloatType& y) noexcept {
        x -= IDouble(y);
        return x;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator*(IDouble x, const IntOrFloatType& y) noexcept {
        x *= IDouble(y);
        return x;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator*(const IntOrFloatType& x, IDouble y) noexcept {
        y *= IDouble(x);
        return y;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator/(IDouble x, const IntOrFloatType& y) noexcept {
        x /= IDouble(y);
        return x;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IDouble operator/(const IntOrFloatType& x, IDouble y) noexcept {
        IDouble xx(x);
        xx /= y;
        return xx;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IBool operator<(IntOrFloatType x, IDouble y) noexcept {
        return IDouble(x) < y;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IBool operator>(IntOrFloatType x, IDouble y) noexcept {
        return IDouble(x) > y;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IBool operator<=(IntOrFloatType x, IDouble y) noexcept {
        return IDouble(x) <= y;
    }

    template<typename IntOrFloatType, Enabler<IsBuiltinNumber<IntOrFloatType>::value> = 0>
    inline IBool operator>=(IntOrFloatType x, IDouble y) noexcept {
        return IDouble(x) >= y;
    }

    inline bool same(IDouble x, IDouble y) noexcept {
        return x.lb() == y.lb() && x.ub() == y.ub();
    }

    inline double lb(IDouble x) noexcept {
        return x.lb();
    }

    inline double ub(IDouble x) noexcept {
        return x.ub();
    }

    inline double center(IDouble x) noexcept {
        return 0.5 * (lb(x) + ub(x));
    }

    inline bool singleton(IDouble x) noexcept {
        return lb(x) == ub(x);
    }

    inline bool is_finite(IDouble x) noexcept {
        return x.is_finite();
    }

    inline IDouble sqrt(IDouble x) noexcept {
        return x.sqrt();
    }

    template<unsigned N>
    inline IDouble fixed_pow(IDouble x) noexcept
    {
        return x.fixed_pow<N>();
    }

    inline IDouble square(IDouble x) noexcept {
        return x.fixed_pow<2>();
    }

    inline IDouble cube(IDouble x) noexcept {
        return x.fixed_pow<3>();
    }

    inline IDouble join(IDouble x, IDouble y) noexcept {
        return x.join(y);
    }

    inline IDouble intersection(IDouble x, IDouble y) noexcept {
        return x.intersection(y);
    }

    inline double add_rd(double x, double y) noexcept {
        return impl::add_rd(x, y);
    }

    inline double add_ru(double x, double y) noexcept {
        return -impl::add_rd(-x, -y);
    }

    inline bool possibly_undefined(IDouble x) noexcept {
        return x.possibly_undefined();
    }

    IDouble sin(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble cos(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble tan(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble asin(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble acos(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble atan(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble exp(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble log2(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;
    IDouble ln(IDouble x) noexcept IVARP_FN_PURE IVARP_FN_VISIBLE;

    template<typename CharType, typename Traits>
        inline std::basic_ostream<CharType, Traits>&
            operator<<(std::basic_ostream<CharType, Traits>& o, IDouble x)
    {
        return o << CharType('[') << lb(x) << CharType(',')
                 << CharType(' ') << ub(x) << CharType(']');
    }

    template<typename IntervalType> struct BoundTypeT;
    template<> struct BoundTypeT<IDouble> {
        using T = double;
    };
    template<typename IntervalType> using BoundType = typename BoundTypeT<IntervalType>::T;

    inline std::pair<double, double> modf(double v) noexcept {
        double integral, fractional;
        fractional = std::modf(v, &integral);
        return {integral, fractional};
    }

    template<typename IT> static inline std::pair<IT, IT> split_half(const IT& iv) {
        auto mid = iv.center();
        return {IT{iv.lb(), mid}, IT{mid, iv.ub()}};
    }

    static inline IDouble max IVARP_NO_MACRO (IDouble i1, IDouble i2) noexcept IVARP_FN_PURE;
    static inline IDouble max IVARP_NO_MACRO (IDouble i1, IDouble i2) noexcept {
        return i1.max IVARP_NO_MACRO (i2);
    }

    static inline IDouble min IVARP_NO_MACRO (IDouble i1, IDouble i2) noexcept IVARP_FN_PURE;
    static inline IDouble min IVARP_NO_MACRO (IDouble i1, IDouble i2) noexcept {
        return i1.min IVARP_NO_MACRO (i2);
    }
}
