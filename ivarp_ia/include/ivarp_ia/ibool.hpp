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

namespace ivarp {
    class IBool {
    public:
        IBool() noexcept = default;

        explicit IBool(bool value) noexcept :
            lb(value), ub(value)
        {}

        IBool(bool lb, bool ub) noexcept :
            lb(lb), ub(ub)
        {}

        bool possibly() const noexcept {
            return ub;
        }

        bool definitely() const noexcept {
            return lb;
        }

        bool indeterminate() const noexcept {
            return lb != ub;
        }

        IBool operator!() const noexcept {
            return IBool(!ub, !lb);
        }

        IBool operator&(IBool other) const noexcept {
            return IBool(lb & other.lb, ub & other.ub);
        }

        IBool operator|(IBool other) const noexcept {
            return IBool(lb | other.lb, ub | other.ub);
        }

        IBool &operator&=(IBool other) noexcept {
            lb &= other.lb;
            ub &= other.ub;
            return *this;
        }

        IBool &operator|=(IBool other) noexcept {
            lb |= other.lb;
            ub |= other.ub;
            return *this;
        }

        IBool operator||(IBool other) const noexcept {
            return IBool(lb | other.lb, ub | other.ub);
        }

        IBool operator&&(IBool other) const noexcept {
            return IBool(lb & other.lb, ub & other.ub);
        }

        IBool operator^(IBool other) const noexcept {
            if(indeterminate() || other.indeterminate()) {
                return IBool{false, true};
            } else {
                bool neq = (lb != other.lb);
                return IBool{neq, neq};
            }
        }

        IBool &operator^=(IBool other) noexcept {
            IBool res = *this ^ other;
            lb = res.lb;
            ub = res.ub;
            return *this;
        }

        IBool operator==(IBool other) const noexcept {
            if(lb != ub || other.lb != other.ub) {
                return IBool{false, true};
            } else {
                bool res = (lb == other.lb);
                return IBool{res};
            }
        }

        IBool operator!=(IBool other) const noexcept {
            return !(*this == other);
        }

    private:
        bool lb, ub;
    };

    inline bool possibly(IBool v) noexcept {
        return v.possibly();
    }

    inline bool definitely(IBool v) noexcept {
        return v.definitely();
    }

    inline bool indeterminate(IBool v) noexcept {
        return v.indeterminate();
    }

    inline bool same(IBool v, IBool w) noexcept {
        return possibly(v) == possibly(w) &&
               definitely(v) == definitely(w);
    }

    inline std::ostream &operator<<(std::ostream& out, IBool b) {
        if(definitely(b)) {
            out << "{true}";
        } else if(!possibly(b)) {
            out << "{false}";
        } else {
            out << "{false, true}";
        }
        return out;
    }
}
