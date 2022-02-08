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
#include <string>
#include <cstdint>
#include <limits>
#include <functional>
#include <vector>
#include <cassert>
#include <sstream>
#include "basic_variable_set.hpp"
#include "prover.hpp"
#include "rectangle_cover.hpp"

using IDouble = ivarp::IDouble;
using IBool = ivarp::IBool;

static const IDouble sqrt2{1.41421356237309492343001693370752036571502685546875,
                           1.4142135623730951454746218587388284504413604736328125};
static const IDouble rsqrt2{0.707106781186547461715008466853760182857513427734375,
                            0.70710678118654757273731092936941422522068023681640625};

struct HalfsquaresVariablesCase3 : BasicVariableSet<HalfsquaresVariablesCase3, 2> {
    using Super = BasicVariableSet<HalfsquaresVariablesCase3, 2>;
    DECLARE_NAMED_VARIABLE(r1, 0)
    DECLARE_NAMED_VARIABLE(r2, 1)

    HalfsquaresVariablesCase3() : Super(+initial_values, +change_handlers) {}

    template<typename Callback>
        void split(Callback&& cb, std::uint64_t depth) noexcept
    {
        this->default_split(std::forward<Callback>(cb), depth);
    }

    void on_r1_changed(bool lbc, bool ubc) {
        if(ubc) {
            restrict_r2_ub(get_r1().ub());
            IDouble r2min = rsqrt2 - get_r1();
            restrict_r2_lb(r2min.lb());
        }
    }

    void on_r2_changed(bool lbc, bool ubc) {
        if(lbc) {
            restrict_r1_lb(get_r2().lb());
        }
    }

    static constexpr Super::OnChangeHandler change_handlers[Super::num_vars] = {
        &HalfsquaresVariablesCase3::on_r1_changed,
        &HalfsquaresVariablesCase3::on_r2_changed
    };

    static const IDouble initial_values[Super::num_vars];
};

struct HalfsquaresVariablesCase3Checker {
    HalfsquaresVariablesCase3Checker(IDouble r1, IDouble r2) noexcept :
        r1(r1), r2(r2), r1sq(square(r1)), r2sq(square(r2))
    {}

    IBool check() {
        coeff1 = (r1sq - r2sq + 0.5) * rsqrt2;
        coeff2 = (r2sq - r1sq + 0.5) * rsqrt2;
        coeff3 = r1sq - square(coeff1);
		coeff3.restrict_lb(0.0);
		coeff3 = sqrt(coeff3);
        m1x = (coeff1 + coeff3) * rsqrt2;
        m2dy = (coeff2 + coeff3) * rsqrt2;
        IDouble hrem = 1.0 - 2.0 * m2dy;
        IDouble wrem = 1.0 - 2.0 * m1x;
        return rectangle_cover_works(hrem, wrem, 0.5 - r1sq - r2sq, IDouble{0.0, r2.ub()});
    }

    IDouble r1, r2, r1sq, r2sq;
    IDouble coeff1, coeff2, coeff3;
    IDouble m1x, m2dy;
};

struct HalfsquaresCase3WeightInsufficient : Constraint<HalfsquaresVariablesCase3> {
    virtual std::string name() const { return "Halfsquares Case 3 weight is insufficient"; }

    virtual IBool satisfied(const HalfsquaresVariablesCase3& vars) {
        HalfsquaresVariablesCase3Checker checker{vars.get_r1(), vars.get_r2()};
        return !checker.check();
    }
};

const ivarp::IDouble HalfsquaresVariablesCase3::initial_values[HalfsquaresVariablesCase3::num_vars] = {
        {0.3535533905932737308575042334268800914287567138671875, 0.5},
        {0.207106781186547517226159698111587204039096832275390625,
         0.47413793103448276244904491250053979456424713134765625}
};

constexpr HalfsquaresVariablesCase3::OnChangeHandler
HalfsquaresVariablesCase3::change_handlers[HalfsquaresVariablesCase3::num_vars];

std::ostream& operator<<(std::ostream& out, const HalfsquaresVariablesCase3& vars) {
    out << "r_1: " << vars.get_r1() << ", r_2: " << vars.get_r2();
	IDouble r1sq = square(vars.get_r1()), r2sq = square(vars.get_r2());
	IDouble coeff1 = (r1sq - r2sq + 0.5) * rsqrt2;
	IDouble coeff2 = (r2sq - r1sq + 0.5) * rsqrt2;
    IDouble coeff3 = sqrt(r1sq - square(coeff1));
	return out;
}

bool proof_halfsquares_case3() {
    Prover<HalfsquaresVariablesCase3> prover_halfsquares3;
    HalfsquaresVariablesCase3 variables;
    prover_halfsquares3.add_variable_set(variables);
    prover_halfsquares3.emplace_constraint<HalfsquaresCase3WeightInsufficient>();
    prover_halfsquares3.abort_on_satisfiable();
    prover_halfsquares3.abort_at_height(100);
    return prover_halfsquares3.prove();
}

bool proof_halfsquares() {
    if(!proof_halfsquares_case3()) {
		return false;
	}
	std::cout << "Halfsquares done!" << std::endl;
	return true;
}

