# The code is open source under the MIT license.
# Copyright 2019-2020, Phillip Keldenich, TU Braunschweig, Algorithms Group
# https://ibr.cs.tu-bs.de/alg
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# provide a target which turns the address sanitizer on for debug builds; can be very helpful to catch memory errors.
if(NOT TARGET util::debug_use_asan)
	add_library(__util_debug_use_asan INTERFACE)
	add_library(util::debug_use_asan ALIAS __util_debug_use_asan)

	if((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0") AND ("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Gg][Nn][Uu]" OR
	   "${CMAKE_CXX_COMPILER_ID}" MATCHES "[Cc][Ll][Aa][Nn][Gg]"))

		target_compile_options(__util_debug_use_asan INTERFACE "$<$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CXX>>:$<$<CONFIG:Debug>:-fsanitize=address>>$<$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CUDA>>:$<$<CONFIG:Debug>:-Xcompiler=-fsanitize=address>>")
        target_link_options(__util_debug_use_asan INTERFACE "$<$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CXX>>:$<$<CONFIG:Debug>:-fsanitize=address>>$<$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CUDA>>:$<$<CONFIG:Debug>:-Xlinker=-fsanitize=address>>")
	endif()
endif()

