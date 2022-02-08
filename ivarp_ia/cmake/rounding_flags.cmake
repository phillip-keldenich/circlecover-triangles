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

# enable C++ code that touches the rounding mode / floating-point environment
if(NOT TARGET util::cuda_enable_rounding)
	add_library(__util_cuda_enable_rounding INTERFACE)
	add_library(util::cuda_enable_rounding ALIAS __util_cuda_enable_rounding)

	target_compile_options(__util_cuda_enable_rounding INTERFACE "$<$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CUDA>>:--ftz=false;--prec-sqrt=true;--prec-div=true>")
endif()

if(NOT TARGET util::enable_rounding)
	add_library(__util_enable_rounding INTERFACE)
	add_library(util::enable_rounding ALIAS __util_enable_rounding)

	if(MSVC)
		target_compile_options(__util_enable_rounding INTERFACE "/fp:strict")
	elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Gg][Nn][Uu]")
		target_compile_options(__util_enable_rounding INTERFACE "-frounding-math" "-ffp-contract=off")
	elseif(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "[Cc][Ll][Aa][Nn][Gg]")
		message(WARNING "C++ compiler (ID ${CMAKE_CXX_COMPILER_ID}) not recognized; we do not know how to enable proper rounding support for this compiler!")
	endif()

	util_make_flags_cuda_compatible(__util_enable_rounding)
	target_link_libraries(__util_enable_rounding INTERFACE util::cuda_enable_rounding)
endif()

# try to make sure we are using SSE2 for our floating-point math
if(NOT TARGET util::use_sse)
	add_library(__util_use_sse INTERFACE)
	add_library(util::use_sse ALIAS __util_use_sse)

	# MSVC uses SSE2 by default
	if("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Gg][Nn][Uu]" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "[Cc][Ll][Aa][Nn][Gg]")
		target_compile_options(__util_use_sse INTERFACE "-mfpmath=sse" "-mavx2")
	else()
		message(WARNING "C++ compiler (ID ${CMAKE_CXX_COMPILER_ID}) not recognized; we do not know how to change to a floating-point engine that allows proper rounding support!")
	endif()

	util_make_flags_cuda_compatible(__util_use_sse)
endif()

