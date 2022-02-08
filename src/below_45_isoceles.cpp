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
#include "rectangle_base_cover.hpp"
#include "r1_in_center.hpp"
#include "two_large_disks.hpp"
#include "below_45_isoceles.hpp"
#include "below_45_isoceles_derivatives.hpp"

using IDouble = ivarp::IDouble;
using IBool = ivarp::IBool;

/**
 * Variable set for proving the critical covering density for isoceles triangles with
 * alpha <= 45 degrees.
 */
class Below45IsocelesVariables : public BasicVariableSet<Below45IsocelesVariables, 4> {
    using Super = BasicVariableSet<Below45IsocelesVariables, 4>;
public:
    explicit Below45IsocelesVariables() noexcept : Super(+initial_values, +change_handlers) {}

    DECLARE_NAMED_VARIABLE(alpha, 0)
    DECLARE_NAMED_VARIABLE(r1, 1)
    DECLARE_NAMED_VARIABLE(r2, 2)
    DECLARE_NAMED_VARIABLE(r3, 3)

    IDouble tan_alpha_half;
    IDouble sin_alpha;
    IDouble cos_alpha;
    IDouble weight;
    IDouble height;
    IDouble goal_efficiency;

    template<typename Callback>
        void split(Callback&& cb, std::uint64_t depth) noexcept
    {
        this->default_split(std::forward<Callback>(cb), depth);
    }

private:
    IDouble raw_goal_efficiency(IDouble alpha) {
        return ivarp::square(ivarp::sin(alpha)) / ivarp::tan(0.5 * alpha);
    }

    void on_alpha_changed(bool lb_changed, bool ub_changed) noexcept {
        tan_alpha_half = ivarp::tan(0.5 * get_alpha());
        sin_alpha = ivarp::sin(get_alpha());
        cos_alpha = ivarp::cos(get_alpha());
        IDouble max_r1 = 0.5 / sin_alpha;
        weight = ivarp::square(max_r1);
        height = 0.5 / tan_alpha_half;
        goal_efficiency = IDouble(
            raw_goal_efficiency(IDouble(get_alpha().lb())).lb(),
            raw_goal_efficiency(IDouble(get_alpha().ub())).ub()
        );
        restrict_r1_ub(max_r1.ub());
        restrict_r2_ub(ivarp::sqrt(0.5 * weight).ub());
        restrict_r3_ub(ivarp::sqrt(weight / 3.0).ub());
    }

    void on_r1_changed(bool lb_changed, bool ub_changed) noexcept {
        if(ub_changed) {
            restrict_r2_ub(get_r1().ub());
        }
    }

    void on_r2_changed(bool lb_changed, bool ub_changed) noexcept {
        if(lb_changed) {
            restrict_r1_lb(get_r2().lb());
        }
        if(ub_changed) {
            restrict_r3_ub(get_r2().ub());
        }
    }

    void on_r3_changed(bool lb_changed, bool ub_changed) noexcept {
        if(lb_changed) {
            restrict_r2_lb(get_r3().lb());
        }
    }

    static constexpr Super::OnChangeHandler change_handlers[Super::num_vars] = {
        &Below45IsocelesVariables::on_alpha_changed,
        &Below45IsocelesVariables::on_r1_changed,
        &Below45IsocelesVariables::on_r2_changed,
        &Below45IsocelesVariables::on_r3_changed
    };
    static const ivarp::IDouble initial_values[Super::num_vars];
};

template<typename VariableSet> struct Radius123Consistency : Constraint<VariableSet> {
    using IBool = ivarp::IBool;
    using IDouble = ivarp::IDouble;

    bool can_propagate() const override {
        return true;
    }

    std::string name() const override {
        return "Consistency between r_1, r_2 and r_3";
    }

    IBool satisfied(const VariableSet& vset) override {
        using ivarp::square;
        IDouble r1 = vset.get_r1();
        IDouble r2 = vset.get_r2();
        IDouble r3 = vset.get_r3();
        if(bounds_inconsistent(vset)) {
            return {false, false};
        }
        return square(r1) + square(r2) + square(r3) <= vset.weight;
    }

    PropagateResult propagate(VariableSet& vset) override {
        IDouble rem_weight = vset.weight;
        rem_weight -= ivarp::square(vset.get_r1());
        if(rem_weight.restrict_lb(0.0)) {
            if(rem_weight.ub() < 0.0) {
                return PropagateResult::CHANGED_EMPTY;
            }
        }
        PropagateResult result = PropagateResult::UNCHANGED;
        if(vset.restrict_r2_ub(ivarp::sqrt(rem_weight).ub())) {
            result |= PropagateResult::CHANGED;
        }
        rem_weight -= ivarp::square(vset.get_r2());
        if(rem_weight.restrict_lb(0.0)) {
            if(rem_weight.ub() < 0.0) {
                return PropagateResult::CHANGED_EMPTY;
            }
        }
        if(vset.restrict_r3_ub(ivarp::sqrt(rem_weight).ub())) {
            result |= PropagateResult::CHANGED;
        }
        if(bounds_inconsistent(vset)) {
            return PropagateResult::CHANGED_EMPTY;
        }
        return result;
    }

    bool bounds_inconsistent(const VariableSet& v) const noexcept {
        IDouble r1 = v.get_r1(), r2 = v.get_r2(), r3 = v.get_r3();
        return r1.lb() > r1.ub() || r2.lb() > r2.ub() || r3.lb() > r3.ub();
    }
};

const ivarp::IDouble Below45IsocelesVariables::initial_values[Below45IsocelesVariables::num_vars] = {
    {0.3449678733707022271204323260462842881679534912109375,
     0.78539816339744839002179332965170033276081085205078125},
    {0.0, 0.5},
    {0.0, 0.5},
    {0.0, 0.5}
};

constexpr Below45IsocelesVariables::OnChangeHandler
Below45IsocelesVariables::change_handlers[Below45IsocelesVariables::num_vars];

std::ostream& operator<<(std::ostream& output, const Below45IsocelesVariables& vars) {
    IDouble pi{3.141592653589793115997963468544185161590576171875,
               3.141592653589793560087173318606801331043243408203125};
    auto output_var = [&] (IDouble value, const char* name) -> std::ostream& {
        return output << name << " ∈ " << value;
    };
    output_var(vars.get_alpha(), "α") << std::endl;
    output_var(vars.get_alpha() * 180.0 / pi, "α") << "°" << std::endl;
    output_var(vars.get_r1(), "r_1") << std::endl;
    output_var(vars.get_r2(), "r_2") << std::endl;
    output_var(vars.get_r3(), "r_3") << std::endl;
    IDouble remweight = vars.weight - ivarp::square(vars.get_r1()) - ivarp::square(vars.get_r2()) -
                        ivarp::square(vars.get_r3());
    output_var(remweight, "remaining weight") << " --- ";
    output_var(ivarp::sqrt(remweight), "remaining radius") << std::endl;
    return output;
}

template<typename VariableSet>
struct NotInManualRegion : Constraint<VariableSet> {
    std::string name() const override {
        return "Exclude the manual region of our proof";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        return vars.get_alpha() < 0.7679448708775049592389905228628776967525482177734375 ||
               vars.get_r1() < 0.48 ||
               vars.get_r2() < 0.48;
    }
};

bool prove_acute_isoceles_below45() {
    if(!prove_below45_isoceles_derivative_signs()) {
        return false;
    }
    Prover<Below45IsocelesVariables> prover_below45;
    Below45IsocelesVariables variables;
    prover_below45.add_variable_set(variables);
    prover_below45.emplace_constraint<Radius123Consistency<Below45IsocelesVariables>>(); // necessary (tested)
    prover_below45.emplace_constraint<RectangleBaseRectangleCoverLemma4<Below45IsocelesVariables>>(); // necessary (tested)
    prover_below45.emplace_constraint<R1R2RectangleBaseCover<Below45IsocelesVariables>>(); // necessary (tested)
    prover_below45.emplace_constraint<R1R2R3RectangleBaseCover<Below45IsocelesVariables>>(); // necessary (tested)
    prover_below45.emplace_constraint<NotInManualRegion<Below45IsocelesVariables>>(); // necessary (tested)
    prover_below45.emplace_constraint<R1InCenterCover<Below45IsocelesVariables>>(); // necessary (tested)
    prover_below45.emplace_constraint<TwoLargeDisksConvergent<Below45IsocelesVariables>>(); // necessary (tested)
    prover_below45.abort_on_satisfiable();
    prover_below45.abort_at_height(100);
    return prover_below45.prove();
}
