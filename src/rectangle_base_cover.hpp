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

template<typename VariableSet> struct RectangleBaseRectangleCoverLemma4 : Constraint<VariableSet> {
    using IBool = ivarp::IBool;
    using IDouble = ivarp::IDouble;

    std::string name() const override {
        return "Cover Base Rectangle with RC Lemma 4";
    }

    IBool satisfied(const VariableSet& vars) override {
        IDouble r1 = vars.get_r1(), r2 = vars.get_r2(), r3 = vars.get_r3();
        IDouble r1sq = ivarp::square(r1), r2sq = ivarp::square(r2), r3sq = ivarp::square(r3);
        IDouble remaining_weight = vars.weight;
        IDouble remaining_weight2 = vars.weight - r1sq;
        IDouble remaining_weight3 = remaining_weight2 - r2sq;
        IBool w3 = works_with(vars, r3, r3sq, remaining_weight3);
        if(definitely(w3)) {
            return {false, false};
        }
        IBool w2 = works_with(vars, r2, r3sq, remaining_weight2);
        if(definitely(w2)) {
            return {false, false};
        }
        return !w3 && !w2 && !works_with(vars, r1, r3sq, remaining_weight);
    }

    static IDouble inverse_lemma4_coefficient() noexcept {
        return {1.6393442622950817888494157159584574401378631591796875,
             // 1.63934426229508196721311475... // 1/0.61
                1.63934426229508201089402064098976552486419677734375};
    }

    static IDouble lemma4_coefficient() noexcept {
        return {0.60999999999999998667732370449812151491641998291015625,
                0.6100000000000000976996261670137755572795867919921875};
    }

    IBool works_with(const VariableSet& vars, IDouble largest_rect_disk,
                     IDouble additional_weight, IDouble remaining_weight)
    {
        IDouble lambda_4_min = largest_rect_disk / 0.375;
        IDouble h4 = (ivarp::max)(IDouble(1.0), lambda_4_min);
        IDouble h4rc4 = lemma4_coefficient() * h4;
        IDouble width4plus = lambda_4_min + additional_weight / h4rc4;
        IDouble weight4plus = h4rc4 * lambda_4_min + additional_weight;
        IBool enough_weight = (weight4plus <= remaining_weight);
        IDouble efficiency = inverse_lemma4_coefficient() * (1.0 - width4plus * vars.tan_alpha_half);
        return enough_weight && efficiency >= vars.goal_efficiency;
    }
};

/*template<typename VariableSet> struct RectangleBaseRectangleCoverLemma3 : Constraint<VariableSet> {
    using IBool = ivarp::IBool;
    using IDouble = ivarp::IDouble;

    std::string name() const override {
        return "Cover Base Rectangle with RC Lemma 3";
    }

    IBool satisfied(const VariableSet& vset) override {
        const IDouble lambda3_scaling_factor{
            1.0764854636994567460561711413902230560779571533203125,
         // 1.0764854636994569506... // sqrt(1/sigma_hat)
            1.076485463699456968100776066421531140804290771484375
        };
        double lemma3_weight_per_area = 195.0 / 256;
        IDouble alpha = vset.get_alpha();
        IDouble r1 = vset.get_r1();
        IDouble r2 = vset.get_r2();
        IDouble r2sq = ivarp::square(r2);
        IDouble lambda3min = r2 * lambda3_scaling_factor;
        IDouble lambda3max = lambda3min + r2sq / lemma3_weight_per_area;
        IDouble lemma3weight = lambda3max * lemma3_weight_per_area;
        IBool enough_without_r1 = (lemma3weight <= vset.weight - ivarp::square(r1));
        if(!possibly(enough_without_r1)) {
            return {true, true};
        }
        IDouble triangle_area_covered = lambda3max - vset.tan_alpha_half * ivarp::square(lambda3max);
        IDouble area_per_weight = triangle_area_covered / lemma3weight;
        return !enough_without_r1 || area_per_weight < vset.goal_efficiency;
    }
}; */

template<typename VariableSet> struct R1R2RectangleBaseCover : Constraint<VariableSet> {
    using IBool = ivarp::IBool;
    using IDouble = ivarp::IDouble;

    std::string name() const override {
        return "Cover Base Rectangle with r_1 and r_2";
    }

    IBool satisfied(const VariableSet& vset) override {
        const IDouble alpha = vset.get_alpha(), r1 = vset.get_r1(), r2 = vset.get_r2();
        IDouble r1sq = ivarp::square(r1);
        IDouble r2sq = ivarp::square(r2);
        IDouble covered_width_sq = -16*(ivarp::square(r1sq) + ivarp::square(r2sq)) + 32*r1sq*r2sq + 8*r1sq + 8*r2sq - 1;
        IBool can_cover_rect = (covered_width_sq >= 0);
        if(!possibly(can_cover_rect)) {
            return {true, true};
        }
        covered_width_sq.restrict_lb(0.0);
        IDouble covered_width = 0.5 * ivarp::sqrt(covered_width_sq);
        IDouble rem_triangle_scale = 1.0 - (covered_width / vset.height);
        IDouble remaining_weight = vset.weight - r1sq - r2sq;
        IDouble required_weight = vset.weight * ivarp::square(rem_triangle_scale);
        return !can_cover_rect || remaining_weight < required_weight;
    }
};

template<typename VariableSet> struct R1R2R3RectangleBaseCover : Constraint<VariableSet> {
    using IBool = ivarp::IBool;
    using IDouble = ivarp::IDouble;

    std::string name() const override {
        return "Cover Base Rectangle with r_1, r_2 and r_3";
    }

    IBool satisfied(const VariableSet& vset) override {
        IDouble r1 = vset.get_r1(), r2 = vset.get_r2(), r3 = vset.get_r3();
        IDouble r1sq = ivarp::square(r1), r2sq = ivarp::square(r2), r3sq = ivarp::square(r3);
        IDouble remaining_weight = vset.weight - r1sq - r2sq - r3sq;
        IBool have_weight = (remaining_weight > 0);
        if(!possibly(have_weight)) {
            return {true, true};
        }
        remaining_weight.restrict_lb(0.0);
        IDouble scale_factor = sqrt(remaining_weight / vset.weight);
        IDouble remaining_cov_height = scale_factor * vset.height;
        IDouble must_cover_height = vset.height - remaining_cov_height;
        IDouble mcsq = ivarp::square(must_cover_height);
        IDouble h3_sq = 4.0 * r3sq - mcsq;
        IBool h3_can_cover = (h3_sq >= 0);
        if(!possibly(h3_can_cover)) {
            return {true, true};
        }
        h3_sq.restrict_lb(0.0);
        IDouble h2_sq = 4.0 * r2sq - mcsq;
        h2_sq.restrict_lb(0.0);
        IDouble h1_sq = 4.0 * r1sq - mcsq;
        h1_sq.restrict_lb(0.0);
        IDouble total_width = sqrt(h1_sq) + sqrt(h2_sq) + sqrt(h3_sq);
        return !h3_can_cover || (total_width < 1.0);
    }
};
