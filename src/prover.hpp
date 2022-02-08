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
#include "constraint.hpp"

template<typename VariableSet> class Prover {
public:
    using Constr = Constraint<VariableSet>;
    using ConstrPtr = std::unique_ptr<Constr>;

    template<typename VS,
             decltype((std::declval<const VS&>().trace_string(std::uint64_t(0), std::uint64_t(0)), int(0))) = 0>
        constexpr static bool supports_tracing_fn(int)
    {
        return true;
    }

    template<typename VS>
        constexpr static bool supports_tracing_fn(...)
    {
        return false;
    }

    constexpr static bool tracing_supported = supports_tracing_fn<VariableSet>(0);

    struct StackElement {
        StackElement(const Prover& prover, VariableSet domain, std::uint64_t id) :
            domain(std::move(domain)),
            height(0),
            id(id), parent_id(0)
        {}

        StackElement(VariableSet domain, const StackElement& parent, std::uint64_t id) :
            domain(std::move(domain)),
            height(parent.height + 1u),
            id(id),
            parent_id(parent.id)
        {}

        VariableSet domain;
        std::uint64_t height;
        std::uint64_t id, parent_id;
    };

    explicit Prover() = default;

    void add_variable_set(const VariableSet& vars) {
        m_basic.push_back(vars);
    }

    void add_constraint(ConstrPtr m) {
        m_constraints.push_back(std::move(m));
    }

    template<typename ConstraintType, typename... ConstraintArgs>
        void emplace_constraint(ConstraintArgs&&... args)
    {
        m_constraints.push_back(std::make_unique<ConstraintType>(std::forward<ConstraintArgs>(args)...));
    }

    void abort_on_satisfiable(bool value = true) noexcept {
        m_abort_satisfiable = value;
    }

    void abort_at_height(std::uint64_t height) noexcept {
        m_abort_height = height;
    }

    void trace(bool active = true) noexcept {
        m_trace = active && tracing_supported;
    }

    template<typename Callable>
        void set_reporter(Callable&& callable)
    {
        m_reporter = std::forward<Callable>(callable);
    }

    bool prove() {
        setup_proof();
        bool result = true;
        while(!m_stack.empty()) {
            StackElement element = m_stack.back();
            m_stack.pop_back();
            trace_node(element);
            if(run_propagators(element)) {
                if(m_trace) {
                    *m_tracer << "Empty after propagation!" << std::endl;
                }
                continue;
            }
            ivarp::IBool cresult = run_checkers(element);
            if(!possibly(cresult)) {
                if(m_trace) {
                    *m_tracer << "Constraints violated!" << std::endl;
                }
                continue;
            }
            bool def = definitely(cresult);
            if(def) {
                cresult &= run_propagators_as_checkers(element);
                if(!possibly(cresult)) {
                    if(m_trace) {
                        *m_tracer << "Constraints violated!" << std::endl;
                    }
                    continue;
                }
                def = definitely(cresult);
            }
            if(def) {
                result = false;
                report_satisfiable(element.domain, true);
                if(m_abort_satisfiable) {
                    m_stack.clear();
                }
                assert(all_possible(element));
            } else {
                if(element.height == m_abort_height) {
                    result = false;
                    report_satisfiable(element.domain, false);
                    if(m_abort_satisfiable) {
                        m_stack.clear();
                    }
                    assert(all_possible(element));
                } else {
                    auto split_callback = [&] (VariableSet split_domain) {
                        m_stack.push_back(StackElement(split_domain, element, ++m_id_counter));
                    };
                    element.domain.split(split_callback, element.height);
                }
            }
        }
        return result;
    }

private:
#ifndef NDEBUG
    bool all_possible(const StackElement& element) noexcept {
        ivarp::IBool props = run_checker_collection(element, m_propagators);
        ivarp::IBool cstrs = run_checker_collection(element, m_checkers);
        return possibly(props && cstrs);
    }
#endif

    void setup_proof() {
        m_checkers.clear();
        m_propagators.clear();
        for(const auto& c : m_constraints) {
            if(c->can_propagate()) {
                m_propagators.push_back(c.get());
            } else {
                m_checkers.push_back(c.get());
            }
        }
        m_stack.clear();
        for(const VariableSet& v : m_basic) {
            m_stack.push_back(StackElement(*this, v, ++m_id_counter));
        }
    }

    void trace_node(const StackElement& element) const {
        if constexpr(tracing_supported) {
            if(m_trace) {
                std::string t = element.domain.trace_string(element.id, element.parent_id);
                *m_tracer << t << std::endl;
            }
        }
    }

    bool run_propagators(StackElement& element) {
        PropagateResult any_change;
        do {
            any_change = PropagateResult::UNCHANGED;
            for(Constr* p : m_propagators) {
                PropagateResult pr = p->propagate(element.domain);
                any_change |= pr;
                if(pr == PropagateResult::EMPTY) {
                    break;
                }
            }
        } while(any_change == PropagateResult::CHANGED);
        return (any_change & PropagateResult::EMPTY) != PropagateResult::UNCHANGED;
    }

    ivarp::IBool run_checker_collection(const StackElement& element, const std::vector<Constr*>& collection) const {
        ivarp::IBool cresult{true, true};
        for(Constr* p : collection) {
            ivarp::IBool r = p->satisfied(element.domain);
            cresult &= r;
            if(!possibly(r)) {
                break;
            }
        }
        return cresult;
    }

    ivarp::IBool run_checkers(const StackElement& element) const {
        return run_checker_collection(element, m_checkers);
    }

    ivarp::IBool run_propagators_as_checkers(const StackElement& element) const {
        return run_checker_collection(element, m_propagators);
    }

    void report_satisfiable(const VariableSet& vset, bool definitely_satisfiable) const {
        m_reporter(vset, definitely_satisfiable);
    }

    static void default_report_function(const VariableSet& vset, bool /*definitely_satisfiable*/) {
        std::cerr << vset << std::endl;
    }

    std::vector<VariableSet> m_basic;
    std::vector<ConstrPtr> m_constraints;
    std::vector<Constr*> m_propagators;
    std::vector<Constr*> m_checkers;
    std::vector<StackElement> m_stack;
    std::function<void(const VariableSet&, bool)> m_reporter = &default_report_function;
    bool m_abort_satisfiable = false;
    bool m_trace = false;
    std::ostream *m_tracer = &std::cout;
    std::uint64_t m_abort_height = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t m_id_counter = 0;
};
