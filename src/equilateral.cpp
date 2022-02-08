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

using IDouble = ivarp::IDouble;
using IBool = ivarp::IBool;

struct EquilateralCase3Variables : BasicVariableSet<EquilateralCase3Variables, 2> {
    using Super = BasicVariableSet<EquilateralCase3Variables, 2>;
    DECLARE_NAMED_VARIABLE(r1, 0)
    DECLARE_NAMED_VARIABLE(delta, 1)

    EquilateralCase3Variables() noexcept : Super(+initial_values, +change_handlers) {}

    static const ivarp::IDouble initial_values[Super::num_vars];

    void on_r1_changed(bool lbc, bool ubc) noexcept {
        if(ubc) {
            restrict_delta_ub(ivarp::square(get_r1()).ub());
        }
    }

    void on_delta_changed(bool lbc, bool ubc) noexcept {
        if(lbc) {
            restrict_r1_lb(ivarp::sqrt(get_delta()).lb());
        }
    }

    template<typename Callback>
        void split(Callback&& cb, std::uint64_t depth) noexcept
    {
        this->default_split(std::forward<Callback>(cb), depth);
    }

    static constexpr Super::OnChangeHandler change_handlers[Super::num_vars] = {
        &EquilateralCase3Variables::on_r1_changed,
        &EquilateralCase3Variables::on_delta_changed
    };
};

static const ivarp::IDouble c_1{ // bounds on 6 / sqrt(77)
    0.683763458757827624623359952238388359546661376953125,
    0.68376345875782773564566241475404240190982818603515625
};

static const ivarp::IDouble c_2{ // bounds on (11/16 - sqrt(249/256 - 11sqrt(3)/24))
    0.26465294743339573546592191632953472435474395751953125,
    0.264652947433395790977073147587361745536327362060546875
};

static const ivarp::IDouble c_3{ // bounds on sqrt(3)/2
    0.8660254037844385965883020617184229195117950439453125,
    0.86602540378443870761060452423407696187496185302734375
};

static const ivarp::IDouble c_4{ // bounds on 2 / sqrt(3)
    1.15470053837925146211773608229123055934906005859375,
    1.1547005383792516841623410073225386440753936767578125
};

struct FormulaViolated : Constraint<EquilateralCase3Variables> {
    virtual std::string name() const { return "Equilateral Case 3 formula is violated"; }

    IDouble compute_y(IDouble delta) const noexcept {
        return c_1 * c_2 + 12 * delta / 11;
    }

    IDouble compute_x(IDouble r1, IDouble y) const noexcept {
        return c_3 - y - 1.5 * r1;
    }

    IDouble compute_w(IDouble y) const noexcept {
        return 1.0 - c_4 * y;
    }

    IDouble compute_lambda(IDouble x, IDouble w) const noexcept {
        return (ivarp::max)(x/w, w/x);
    }

    virtual IBool satisfied(const EquilateralCase3Variables& vars) {
        IDouble r1 = vars.get_r1(), delta = vars.get_delta();
        IDouble y = compute_y(delta);
        IDouble x = compute_x(r1, y);
        IDouble w = compute_w(y);
        IDouble shortside = (ivarp::min)(x, w);
        IDouble longside = (ivarp::max)(x, w);
        IDouble ssq = ivarp::square(shortside);
        IDouble lsq = ivarp::square(longside);
        IDouble needed_weight = 0.25 * (2 * ssq + lsq);
        IDouble lhs = 0.5 - ivarp::square(r1) - 11 * c_1 * c_2 / 12 - delta;
        return lhs < needed_weight;
    }
};

std::ostream& operator<<(std::ostream& out, const EquilateralCase3Variables& vars) {
    return out << vars.get_r1() << ", " << vars.get_delta();
}

const ivarp::IDouble EquilateralCase3Variables::initial_values[EquilateralCase3Variables::num_vars] = {
    {0.1794035468292133617129735512207844294607639312744140625,
     0.264652947433395790977073147587361745536327362060546875},
    {0.0, 0.07004118258518375605969907837788923643529415130615234375}
};

constexpr EquilateralCase3Variables::OnChangeHandler
EquilateralCase3Variables::change_handlers[EquilateralCase3Variables::num_vars];

bool proof_equilateral() {
    Prover<EquilateralCase3Variables> prover_equilateral;
    EquilateralCase3Variables variables;
    prover_equilateral.add_variable_set(variables);
    prover_equilateral.emplace_constraint<FormulaViolated>();
    prover_equilateral.abort_on_satisfiable();
    prover_equilateral.abort_at_height(100);
    if(!prover_equilateral.prove()) {
		return false;
	}
	std::cout << "Equilateral done!" << std::endl;
	return true;
}

