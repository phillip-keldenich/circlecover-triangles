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

# link against target util::enable_warnings to enable warnings (except for a few annoying/unnecessary ones)
if(NOT TARGET util::enable_warnings)
	add_library(__util_enable_warnings INTERFACE)
	add_library(util::enable_warnings ALIAS __util_enable_warnings)

	if(MSVC)
		target_compile_options(__util_enable_warnings INTERFACE "/W3" "/wd4127" "/wd4244" "/wd6287" "/wd6294"
		                                                        "/wd4701" "/wd4459" "/wd26495" "/wd26812"
																"/wd26439")
	elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Cc][Ll][Aa][Nn][Gg]")
		target_compile_options(__util_enable_warnings INTERFACE
		                       "-Wall" "-Wno-long-long" "-Wno-missing-braces"
		                       "-Wno-self-assign-overloaded")
	elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Gg][Nn][Uu]")
		target_compile_options(__util_enable_warnings INTERFACE "-Wall" "-Wno-long-long" "-Wno-missing-braces")
	endif()

	util_make_flags_cuda_compatible(__util_enable_warnings)
endif()
