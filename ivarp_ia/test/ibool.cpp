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

using namespace ivarp;

DOCTEST_TEST_CASE("[ivarp_ia][IBool] Boolean unary operators") {
    IBool tt{true};
    IBool ff{false};
    IBool id{false, true};
    DOCTEST_REQUIRE(tt.possibly());   DOCTEST_REQUIRE(possibly(tt));
    DOCTEST_REQUIRE(tt.definitely()); DOCTEST_REQUIRE(definitely(tt));
    DOCTEST_REQUIRE(!ff.possibly());  DOCTEST_REQUIRE(!ff.definitely());
    DOCTEST_REQUIRE(!possibly(ff));   DOCTEST_REQUIRE(!definitely(ff));
    DOCTEST_REQUIRE(possibly(id));    DOCTEST_REQUIRE(!definitely(id));
    DOCTEST_REQUIRE(possibly(!id));   DOCTEST_REQUIRE(!definitely(!id));
    IBool nt = !tt;
    IBool nf = !ff;
    DOCTEST_REQUIRE(same(nt, ff));         DOCTEST_REQUIRE(same(nf, tt));
    DOCTEST_REQUIRE(indeterminate(id));    DOCTEST_REQUIRE(indeterminate(!id));
    DOCTEST_REQUIRE(definitely(ff == ff)); DOCTEST_REQUIRE(!possibly(ff != ff));
    DOCTEST_REQUIRE(definitely(tt == tt)); DOCTEST_REQUIRE(!possibly(tt != tt));
    DOCTEST_REQUIRE(indeterminate(id == id));
    DOCTEST_REQUIRE(indeterminate(id != id));
    DOCTEST_REQUIRE(indeterminate(id == tt));
    DOCTEST_REQUIRE(indeterminate(id != ff));
    DOCTEST_REQUIRE(same(!id, id));
}

DOCTEST_TEST_CASE("[ivarp_ia][IBool] Boolean binary operators") {
    IBool tt{true};
    IBool ff{false};
    IBool id{false, true};
    DOCTEST_REQUIRE(definitely(tt || tt));
    DOCTEST_REQUIRE(definitely(tt || ff));
    DOCTEST_REQUIRE(definitely(tt || id));
    DOCTEST_REQUIRE(!possibly(ff || ff));
    DOCTEST_REQUIRE(definitely(ff || tt));
    DOCTEST_REQUIRE(indeterminate(ff || id));
    DOCTEST_REQUIRE(definitely(tt && tt));
    DOCTEST_REQUIRE(!possibly(tt && ff));
    DOCTEST_REQUIRE(indeterminate(tt && id));
    DOCTEST_REQUIRE(!possibly(ff && id));
    DOCTEST_REQUIRE(!possibly(ff && ff));
    DOCTEST_REQUIRE(indeterminate(id && id));
    DOCTEST_REQUIRE(!possibly(ff ^ ff));
    DOCTEST_REQUIRE(!possibly(tt ^ tt));
    DOCTEST_REQUIRE(definitely(tt ^ ff));
    DOCTEST_REQUIRE(indeterminate(ff ^ id));
    DOCTEST_REQUIRE(indeterminate(tt ^ id));
    DOCTEST_REQUIRE(indeterminate(id ^ id));
}
