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
#include "below_45_isoceles_derivatives.hpp"


using IDouble = ivarp::IDouble;
using IBool = ivarp::IBool;

IDouble diff_restweight_by_r1(IDouble alpha, IDouble r1, IDouble r2) {
    using ivarp::cos;
    using ivarp::sin;
    using ivarp::square;
    using ivarp::sqrt;
    IDouble x0 = 8.0 * square(r1);
    IDouble x1 = 2.0 * square(r2);
    IDouble x2 = cos(alpha);
    IDouble x3 = 2.0 * alpha;
    IDouble x4 = 2.0 * r2 * sin(x3);
    IDouble x5 = x1 * cos(x3);
    IDouble x6 = x0 * x2 - x0 + x1 - x4 - x5 + 1.0;
    IDouble x7 = sqrt(x6/(-x1 + x4 + x5 - 1.0));
    return -2.0 * r1 * (x6 + 2.0*x7*(x2 - 1.0)*(x7 - tan(0.5 * alpha))) / x6;
}

IDouble diff_restweight_by_r2(IDouble alpha, IDouble r2) {
    using ivarp::sin;
    using ivarp::cos;
    using ivarp::square;
    using ivarp::sqrt;
    using ivarp::tan;
    IDouble x0 = 2.0 * square(r2);
    IDouble x1 = 2.0 * alpha;
    IDouble x2 = sin(x1);
    IDouble x3 = 2.0 * r2;
    IDouble x4 = cos(x1);
    IDouble x5 = x0*x4 - x0 + x2*x3;
    IDouble x6 = x5 - 1.0;
    IDouble x7 = 1.0 / x6;
    IDouble x8 = cos(alpha);
    IDouble x9 = x5 - 2.0 * x8 + 1.0;
    IDouble x10 = sqrt(-x7 * x9);
    return -x7*(x10*(x10 - tan(0.5 * alpha))*(x8 - 1.0)*(x2 + x3*x4 - x3) + x3*x6*x9)/x9;
}

IDouble diff_restweight_by_alpha(IDouble alpha) {
    using ivarp::sin;
    using ivarp::cos;
    using ivarp::square;
    using ivarp::cube;
    using ivarp::sqrt;
    using ivarp::tan;
    IDouble x0 = sin(alpha);
    IDouble x1 = 2*alpha;
    IDouble x2 = cos(x1);
    IDouble x3 = sin(x1);
    IDouble x4 = 2*x3;
    IDouble x5 = x2 + x4 - 3;
    IDouble x6 = 1 / x5;
    IDouble x7 = cos(alpha);
    IDouble x8 = -x2 - x4 + 4*x7 - 1;
    IDouble x9 = square(x0);
    IDouble x10 = 0.5 * alpha;
    IDouble x11 = sqrt(x6*x8);
    IDouble x12 = x11 - tan(x10);
    IDouble x13 = square(x12) + 2;
    IDouble x14 = x5 * x8;
    IDouble x15 = 2 * x14 * x7;
    IDouble x16 = 3 * alpha;
    return x6*(x15*(x13*x9 - 1) + x9*(x0*x12*(2*x11*(-9*x0 - 8*x2 + 4*x3 + 6*x7 - sin(x16) + 2*cos(x16)) +
           x14/square(cos(x10))) - x13*x15)) / (4*cube(x0)*x8);
}

template<typename VariableSet>
struct DiffR1Negative : Constraint<VariableSet> {
    std::string name() const override {
        return "Exclude regions where the partial derivative of Δ for r_1 is non-positive";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        return diff_restweight_by_r1(vars.get_alpha(), vars.get_r1(), vars.get_r2()) > 0.0;
    }
};

template<typename VariableSet>
struct DiffR2Negative : Constraint<VariableSet> {
    std::string name() const override {
        return "Exclude regions where the partial derivative of Δ for r_2 is non-positive";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        return diff_restweight_by_r2(vars.get_alpha(), vars.get_r2()) > 0.0;
    }
};

template<typename VariableSet>
struct DiffAlphaNegative : Constraint<VariableSet> {
    std::string name() const override {
        return "Exclude regions where the partial derivative of Δ for alpha is non-positive";
    }

    ivarp::IBool satisfied(const VariableSet& vars) override {
        return diff_restweight_by_alpha(vars.get_alpha()) > 0.0;
    }
};

class VariableSetProofRestweightPartialR1Negative;
inline std::ostream& operator<<(std::ostream& output, const VariableSetProofRestweightPartialR1Negative& vars);
class VariableSetProofRestweightPartialR1Negative :
public BasicVariableSet<VariableSetProofRestweightPartialR1Negative, 3>
{
    using Super = BasicVariableSet<VariableSetProofRestweightPartialR1Negative, 3>;
public:
    explicit VariableSetProofRestweightPartialR1Negative() : Super(+initial_values, +change_handlers) {}

    template<typename Callback>
        void split(Callback&& cb, std::uint64_t depth) noexcept
    {
        this->default_split(std::forward<Callback>(cb), depth);
    }

    DECLARE_NAMED_VARIABLE(alpha, 0)
    DECLARE_NAMED_VARIABLE(r1, 1)
    DECLARE_NAMED_VARIABLE(r2, 2)

    std::string trace_string(std::uint64_t id, std::uint64_t parent_id) const {
        std::ostringstream output;
        output << "NODE " << id << " [PARENT " << parent_id << "]\n";
        output << *this;
        return output.str();
    }

private:
    void on_alpha_changed(bool lb_changed, bool ub_changed) noexcept {}

    void on_r1_changed(bool lb_changed, bool ub_changed) noexcept {
        if(ub_changed) {
            restrict_r2_ub(get_r1().ub());
        }
    }

    void on_r2_changed(bool lb_changed, bool ub_changed) noexcept {
        if(lb_changed) {
            restrict_r1_lb(get_r2().lb());
        }
    }

    static constexpr Super::OnChangeHandler change_handlers[Super::num_vars] = {
        &VariableSetProofRestweightPartialR1Negative::on_alpha_changed,
        &VariableSetProofRestweightPartialR1Negative::on_r1_changed,
        &VariableSetProofRestweightPartialR1Negative::on_r2_changed
    };

    static const ivarp::IDouble initial_values[Super::num_vars];
};

constexpr VariableSetProofRestweightPartialR1Negative::OnChangeHandler
VariableSetProofRestweightPartialR1Negative::change_handlers[VariableSetProofRestweightPartialR1Negative::num_vars];

const ivarp::IDouble VariableSetProofRestweightPartialR1Negative::
    initial_values[VariableSetProofRestweightPartialR1Negative::num_vars] =
{
    {0.7679448708775049592389905228628776967525482177734375,   // < 44 degrees
     0.78539816339744839002179332965170033276081085205078125}, // > 45 degrees
    {0.48, 0.5},
    {0.48, 0.5}
};

inline std::ostream& operator<<(std::ostream& output, const VariableSetProofRestweightPartialR1Negative& vars) {
    output << "α ∈ " << vars.get_alpha() << " (" << vars.get_alpha().center() << ")\nr_1 ∈ " << vars.get_r1() <<  " (" << vars.get_r1().center() << ")\nr_2 ∈ " << vars.get_r2() << " (" << vars.get_r2().center() << ")\n";
    return output << "dΔ/dr_1: " << diff_restweight_by_r1(vars.get_alpha(), vars.get_r1(), vars.get_r2());
}

class VariableSetProofRestweightPartialR2Negative;
inline std::ostream& operator<<(std::ostream& output, const VariableSetProofRestweightPartialR2Negative& vars);
class VariableSetProofRestweightPartialR2Negative :
public BasicVariableSet<VariableSetProofRestweightPartialR2Negative, 2>
{
    using Super = BasicVariableSet<VariableSetProofRestweightPartialR2Negative, 2>;
public:
    explicit VariableSetProofRestweightPartialR2Negative() : Super(+initial_values, +change_handlers) {}

    template<typename Callback>
        void split(Callback&& cb, std::uint64_t depth) noexcept
    {
        this->default_split(std::forward<Callback>(cb), depth);
    }

    DECLARE_NAMED_VARIABLE(alpha, 0)
    DECLARE_NAMED_VARIABLE(r2, 1)

    std::string trace_string(std::uint64_t id, std::uint64_t parent_id) const {
        std::ostringstream output;
        output << "NODE " << id << " [PARENT " << parent_id << "]\n";
        output << *this;
        return output.str();
    }

private:
    void on_alpha_changed(bool lb_changed, bool ub_changed) noexcept {}
    void on_r2_changed(bool lb_changed, bool ub_changed) noexcept {}

    static constexpr Super::OnChangeHandler change_handlers[Super::num_vars] = {
        &VariableSetProofRestweightPartialR2Negative::on_alpha_changed,
        &VariableSetProofRestweightPartialR2Negative::on_r2_changed
    };

    static const ivarp::IDouble initial_values[Super::num_vars];
};

constexpr VariableSetProofRestweightPartialR2Negative::OnChangeHandler
VariableSetProofRestweightPartialR2Negative::change_handlers[VariableSetProofRestweightPartialR2Negative::num_vars];

const ivarp::IDouble VariableSetProofRestweightPartialR2Negative::
    initial_values[VariableSetProofRestweightPartialR2Negative::num_vars] =
{
    {0.7679448708775049592389905228628776967525482177734375,   // < 44 degrees
     0.78539816339744839002179332965170033276081085205078125}, // > 45 degrees
    {0.48, 0.5}
};

inline std::ostream& operator<<(std::ostream& output, const VariableSetProofRestweightPartialR2Negative& vars) {
    output << "α ∈ " << vars.get_alpha() << " (" << vars.get_alpha().center() << ")\nr_2 ∈ " << vars.get_r2() <<  " (" << vars.get_r2().center() << ")\n";
    return output << "dΔ/dr_2: " << diff_restweight_by_r2(vars.get_alpha(), vars.get_r2());
}

class VariableSetProofRestweightPartialAlphaNegative;
inline std::ostream& operator<<(std::ostream& output, const VariableSetProofRestweightPartialAlphaNegative& vars);
class VariableSetProofRestweightPartialAlphaNegative :
public BasicVariableSet<VariableSetProofRestweightPartialAlphaNegative, 1>
{
    using Super = BasicVariableSet<VariableSetProofRestweightPartialAlphaNegative, 1>;
public:
    explicit VariableSetProofRestweightPartialAlphaNegative() : Super(+initial_values, +change_handlers) {}

    template<typename Callback>
        void split(Callback&& cb, std::uint64_t depth) noexcept
    {
        this->default_split(std::forward<Callback>(cb), depth);
    }

    DECLARE_NAMED_VARIABLE(alpha, 0)

    std::string trace_string(std::uint64_t id, std::uint64_t parent_id) const {
        std::ostringstream output;
        output << "NODE " << id << " [PARENT " << parent_id << "]\n";
        output << *this;
        return output.str();
    }

private:
    void on_alpha_changed(bool lb_changed, bool ub_changed) noexcept {}

    static constexpr Super::OnChangeHandler change_handlers[Super::num_vars] = {
        &VariableSetProofRestweightPartialAlphaNegative::on_alpha_changed
    };

    static const ivarp::IDouble initial_values[Super::num_vars];
};

constexpr VariableSetProofRestweightPartialAlphaNegative::OnChangeHandler
VariableSetProofRestweightPartialAlphaNegative::change_handlers[VariableSetProofRestweightPartialAlphaNegative::num_vars];

const ivarp::IDouble VariableSetProofRestweightPartialAlphaNegative::
    initial_values[VariableSetProofRestweightPartialAlphaNegative::num_vars] =
{
    {0.7679448708775049592389905228628776967525482177734375,   // < 44 degrees
     0.78539816339744839002179332965170033276081085205078125} // > 45 degrees
};

inline std::ostream& operator<<(std::ostream& output, const VariableSetProofRestweightPartialAlphaNegative& vars) {
    output << "α ∈ " << vars.get_alpha() << " (" << vars.get_alpha().center() << ")\n";
    return output << "dΔ/dα: " << diff_restweight_by_alpha(vars.get_alpha());
}

bool prove_r1_diff_negative(bool trace = false) {
    Prover<VariableSetProofRestweightPartialR1Negative> prover_r1_diff_negative;
    VariableSetProofRestweightPartialR1Negative variables;
    prover_r1_diff_negative.add_variable_set(variables);
    prover_r1_diff_negative.abort_on_satisfiable(true);
    prover_r1_diff_negative.abort_at_height(100);
    prover_r1_diff_negative.trace(trace);
    prover_r1_diff_negative.emplace_constraint<DiffR1Negative<VariableSetProofRestweightPartialR1Negative>>();
    return prover_r1_diff_negative.prove();
}

bool prove_r2_diff_negative(bool trace = false) {
    Prover<VariableSetProofRestweightPartialR2Negative> prover_r2_diff_negative;
    VariableSetProofRestweightPartialR2Negative variables;
    prover_r2_diff_negative.add_variable_set(variables);
    prover_r2_diff_negative.abort_on_satisfiable(true);
    prover_r2_diff_negative.abort_at_height(100);
    prover_r2_diff_negative.trace(trace);
    prover_r2_diff_negative.emplace_constraint<DiffR2Negative<VariableSetProofRestweightPartialR2Negative>>();
    return prover_r2_diff_negative.prove();
}

bool prove_alpha_diff_negative(bool trace = false) {
    Prover<VariableSetProofRestweightPartialAlphaNegative> prover_alpha_diff_negative;
    VariableSetProofRestweightPartialAlphaNegative variables;
    prover_alpha_diff_negative.add_variable_set(variables);
    prover_alpha_diff_negative.abort_on_satisfiable(true);
    prover_alpha_diff_negative.abort_at_height(100);
    prover_alpha_diff_negative.trace(trace);
    prover_alpha_diff_negative.emplace_constraint<DiffAlphaNegative<VariableSetProofRestweightPartialAlphaNegative>>();
    return prover_alpha_diff_negative.prove();
}

bool prove_below45_isoceles_derivative_signs() {
    return prove_r1_diff_negative() && prove_r2_diff_negative() && prove_alpha_diff_negative();
}
