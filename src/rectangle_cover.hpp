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

#include <ivarp_ia/ivarp_ia.hpp>

namespace impl {
    using ivarp::IDouble;
    using ivarp::IBool;

    template<typename Ignored>
    struct RectangleCoverChecker {
        RectangleCoverChecker(IDouble width, IDouble height, IDouble weight, IDouble r1) :
            raw_min(ivarp::min(width, height)),
            raw_max(ivarp::max(width, height)),
            raw_weight(weight),
            raw_r1(r1)
        {
            IDouble scale = 1.0 / raw_min;
            lambda = scale * raw_max;
            this->weight = ivarp::square(scale) * weight;
            this->r1 = scale * r1;
        }

        IBool check() noexcept {
            compute_thm1_weight();
            IBool thm1 = (weight >= thm1_weight_needed);
            if(definitely(thm1)) {
                return thm1;
            }
            IBool lem3 = check_lem3();
            if(definitely(lem3)) {
                return lem3;
            }
            return check_lem4() || thm1 || lem3;
        }

    private:
        static const IDouble thm1_lambda_switch_value;
        static const IDouble lem3_sigma_hat;
        static const IDouble lem4_efficiency;

        IDouble thm1_weight_below_switch() const noexcept {
            IDouble lsq = square(lambda);
            return (3.0 / 16.0) * lsq + (15.0 / 32.0) + (27.0 / 256.0) / lsq;
        }

        IDouble thm1_weight_above_switch() const noexcept {
            IDouble lsq = square(lambda);
            return 0.25 * (lsq + 2.0);
        }

        void compute_thm1_weight() noexcept {
            IBool lambda_switch = (lambda > thm1_lambda_switch_value);
            if(definitely(lambda_switch)) {
                thm1_weight_needed = thm1_weight_above_switch();
            } else if(!possibly(lambda_switch)) {
                thm1_weight_needed = thm1_weight_below_switch();
            } else {
                thm1_weight_needed = join(thm1_weight_below_switch(), thm1_weight_above_switch());
            }
        }

        IBool check_lem3() noexcept {
            IDouble sigma = (ivarp::max)(square(r1), lem3_sigma_hat);
            IDouble eff_sigma = 0.5 * ivarp::sqrt(ivarp::sqrt(ivarp::square(sigma) + 1.0) + 1.0);
            IDouble weight_req = lambda * eff_sigma;
            return weight >= weight_req;
        }

        IBool check_lem4() noexcept {
            if(r1.ub() <= 0.375) {
                return weight >= lem4_efficiency * lambda;
            } else {
                IDouble necessary_side_length = r1 / 0.375;
                IDouble long_side = ivarp::max(necessary_side_length, lambda);
                return weight >= lem4_efficiency * long_side * necessary_side_length;
            }
        }

        // unnormalized values
        IDouble raw_min, raw_max, raw_weight, raw_r1;
        // normalized values
        IDouble lambda, weight, r1;
        // weight function according to Thm 1 from rectangle packing
        IDouble thm1_weight_needed;
    };

    template<typename Ignored>
    const ivarp::IDouble RectangleCoverChecker<Ignored>::thm1_lambda_switch_value =
        IDouble{1.035797111181671059654263444826938211917877197265625,
             // 1.03579711118167118... (exact value)
                1.0357971111816712816988683698582462966442108154296875};

    template<typename Ignored>
    const ivarp::IDouble RectangleCoverChecker<Ignored>::lem3_sigma_hat =
        IDouble{0.862946080609917398618335937499068677425384521484375,
             // 0.862946080609917412... (exact value)
                0.86294608060991750964063840001472271978855133056640625};

    template<typename Ignored>
    const ivarp::IDouble RectangleCoverChecker<Ignored>::lem4_efficiency =
        IDouble{0.60999999999999998667732370449812151491641998291015625,
                0.6100000000000000976996261670137755572795867919921875};
}

static inline ivarp::IBool rectangle_cover_works(ivarp::IDouble width, ivarp::IDouble height,
                                                 ivarp::IDouble weight, ivarp::IDouble r1)
{
    if(width.ub() <= 0.0 || height.ub() <= 0.0) {
        return {true, true};
    }
    width.restrict_lb(0.0);
    height.restrict_lb(0.0);
    bool poss = (width.lb() <= 0.0 || height.lb() <= 0.0);
    impl::RectangleCoverChecker<void> checker(width, height, weight, r1);
    ivarp::IBool result = checker.check();
    return {definitely(result), possibly(result) || poss};
}
