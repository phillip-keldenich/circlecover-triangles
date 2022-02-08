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

enum class PropagateResult : unsigned {
    UNCHANGED = 0,
    CHANGED = 1,
    EMPTY = 2,
    CHANGED_EMPTY = 3 // not a legal return for propagate
};

static inline PropagateResult operator|(PropagateResult r1, PropagateResult r2) noexcept {
    return static_cast<PropagateResult>(static_cast<unsigned>(r1) | static_cast<unsigned>(r2));
}

static inline PropagateResult &operator|=(PropagateResult& r1, PropagateResult r2) noexcept {
    r1 = static_cast<PropagateResult>(static_cast<unsigned>(r1) | static_cast<unsigned>(r2));
    return r1;
}

static inline PropagateResult operator&(PropagateResult r1, PropagateResult r2) noexcept {
    return static_cast<PropagateResult>(static_cast<unsigned>(r1) & static_cast<unsigned>(r2));
}

static inline PropagateResult &operator&=(PropagateResult& r1, PropagateResult r2) noexcept {
    r1 = static_cast<PropagateResult>(static_cast<unsigned>(r1) & static_cast<unsigned>(r2));
    return r1;
}
