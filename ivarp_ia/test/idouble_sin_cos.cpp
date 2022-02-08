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
#include <doctest/doctest.hpp>
#include <vector>
#include <random>

using namespace ivarp;

DOCTEST_TEST_CASE("[ivarp_ia][IDouble] IDouble sine") {
    std::pair<IDouble,IDouble> tests[] = {
        {IDouble{0}, IDouble{0}}, {IDouble{0, 8}, IDouble{-1,1}},
        {IDouble{1}, IDouble{0.8414709848078965, 0.8414709848078966}},
        {IDouble{5.0e-324, 5.0e-324}, IDouble{0.0, 5.0e-324}},
        {IDouble{0.0, 5.0e-324}, IDouble{0.0, 5.0e-324}},
        {IDouble{-1.0, 1.0}, IDouble{-0.8414709848078966, 0.8414709848078966}},
        {IDouble{1.57, 1.571}, IDouble{0.999999682931834610322141543292673304677009582519531250, 1.0}},
        {IDouble{3.1, 3.25}, IDouble{-0.1081951345301083861061286484073207248002290725708, 0.04158066243329049116}},
        {IDouble{4.5, 4.75}, IDouble{-1.0, -0.9775301176650970091941417194902896881103515625}},
        {IDouble{1.5707963267948965579989817342720925807952880859375, 4.712388980384691}, IDouble{-1.0,1.0}}
    };

    for(auto p : tests) {
        DOCTEST_REQUIRE(same(sin(p.first), p.second));
    }
}
