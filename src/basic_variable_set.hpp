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

#include <tuple>
#include <functional>


template<typename ConcreteVariableSet, std::size_t NumVars>
class BasicVariableSet {
public:
    static constexpr std::size_t num_vars = NumVars;
    using OnChangeHandler = void(ConcreteVariableSet::*)(bool, bool);

    template<typename ForwardIteratorIDouble, typename ForwardIteratorHandlers>
    explicit BasicVariableSet(ForwardIteratorIDouble start_values, ForwardIteratorHandlers handler_begin) noexcept {
        for(std::size_t i = 0; i < num_vars; ++i, ++handler_begin, ++start_values) {
            m_handlers[i] = *handler_begin;
            m_variable_values[i] = *start_values;
        }
        for(std::size_t i = 0; i < num_vars; ++i) {
            p_call_handler(i, true, true);
        }
    }

    BasicVariableSet(const BasicVariableSet&) noexcept = default;
    BasicVariableSet &operator=(const BasicVariableSet&) noexcept = default;

protected:
    template<typename Callback> void default_split(Callback&& callback, std::uint64_t height) const noexcept {
        std::size_t idx = static_cast<std::size_t>(height % num_vars);
        ivarp::IDouble half1, half2;
        std::tie(half1, half2) = ivarp::split_half(m_variable_values[idx]);
        ConcreteVariableSet vset1(*static_cast<const ConcreteVariableSet*>(this));
        ConcreteVariableSet vset2(*static_cast<const ConcreteVariableSet*>(this));
        BasicVariableSet* bvs1 = static_cast<BasicVariableSet*>(&vset1);
        BasicVariableSet* bvs2 = static_cast<BasicVariableSet*>(&vset2);
        bvs1->m_variable_values[idx] = half1;
        bvs2->m_variable_values[idx] = half2;
        bvs1->p_call_handler(idx, false, true);
        bvs2->p_call_handler(idx, true, false);
        callback(vset1);
        callback(vset2);
    }

    template<std::size_t Index> ivarp::IDouble get_value() const noexcept {
        return m_variable_values[Index];
    }

    template<std::size_t Index> void set_value(ivarp::IDouble value) noexcept {
        m_variable_values[Index] = value;
        p_call_handler(Index, true, true);
    }

    template<std::size_t Index> bool restrict_lb(double lower_bound) noexcept {
        ivarp::IDouble& ref = m_variable_values[Index];
        if(ref.lb() < lower_bound) {
            ref.set_lb(lower_bound);
            p_call_handler(Index, true, false);
            return true;
        }
        return false;
    }

    template<std::size_t Index> bool restrict_ub(double upper_bound) noexcept {
        ivarp::IDouble& ref = m_variable_values[Index];
        if(ref.ub() > upper_bound) {
            ref.set_ub(upper_bound);
            p_call_handler(Index, false, true);
            return true;
        }
        return false;
    }

    template<std::size_t Index> bool restrict(ivarp::IDouble bounds) noexcept {
        ivarp::IDouble& ref = m_variable_values[Index];
        bool lbc = false, ubc = false;
        if(ref.lb() < bounds.lb()) {
            ref.set_lb(bounds.lb());
            lbc = true;
        }
        if(ref.ub() > bounds.ub()) {
            ref.set_ub(bounds.ub());
            ubc = true;
        }
        if(lbc || ubc) {
            p_call_handler(Index, lbc, ubc);
        }
        return lbc | ubc;
    }

private:
    void p_call_handler(std::size_t index, bool lb_changed, bool ub_changed) {
        (static_cast<ConcreteVariableSet&>(*this).*(m_handlers[index]))(lb_changed, ub_changed);
    }

    ivarp::IDouble m_variable_values[num_vars];
    OnChangeHandler m_handlers[num_vars];
};

#define DECLARE_NAMED_VARIABLE(name, index) \
    ivarp::IDouble get_##name() const noexcept {   \
        return this->template get_value<index>();\
    }                                       \
    void set_##name(ivarp::IDouble value) noexcept { \
        this->template set_value<index>(value); \
    }                                       \
    bool restrict_##name##_lb(double lower_bound) noexcept { \
        return this->template restrict_lb<index>(lower_bound); \
    }                                       \
    bool restrict_##name##_ub(double upper_bound) noexcept { \
        return this->template restrict_ub<index>(upper_bound); \
    } \
    bool restrict_##name(ivarp::IDouble bounds) noexcept {          \
        return this->template restrict<index>(bounds);\
    }

