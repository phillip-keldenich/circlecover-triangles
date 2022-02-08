# Copyright 2022 Phillip Keldenich, Algorithms Department, TU Braunschweig
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software
#and associated documentation files (the "Software"), to deal in the Software without restriction,
#including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
#and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
#subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
#LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# only touch the standard and similar options if we are
# building the library itself.
if(IVARP_IA_BUILDING_SELF)
	# we cannot rely on more being supported by CUDA.
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_EXTENSIONS Off)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/cmake/cuda_support.cmake" NO_POLICY_SCOPE)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/imported_target_utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/enable_warnings.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/rounding_flags.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/debug_use_asan.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/enable_coverage.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/enable_lto.cmake")

# add find package scripts to the cmake module path and find packages
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")
find_package(Boost REQUIRED)
find_package(GMPXX REQUIRED)
find_package(Threads REQUIRED)

add_library(ivarp_ia SHARED)
set_target_properties(ivarp_ia PROPERTIES
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN On
)
target_include_directories(ivarp_ia PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src" "ivarp_ia_src")
target_link_libraries(ivarp_ia
		PUBLIC util::enable_rounding util::use_sse
		       Boost::boost ivarp::GMPXX ivarp::MPFR Threads::Threads
		       ivarp::cuda_support
		PRIVATE util::enable_warnings util::debug_use_asan __ivarp_ia_sources
)
target_enable_lto(ivarp_ia)

if(IVARP_IA_BUILDING_SELF)
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test" "ivarp_ia_test")
endif()
