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
include(FindPackageHandleStandardArgs)

if(NOT TARGET ivarp::GMPXX)
set(GMP_ROOT "" CACHE STRING "A path where the GNU multiprecision library (GMP) can be found; if unset, it is searched for in the system default directories.")
set(MPFR_ROOT "" CACHE STRING "A path where the MPFR library can be found; if unset, it is searched for using GMP_ROOT and the system default directories.")
mark_as_advanced(GMP_ROOT MPFR_ROOT)

# unfortunately, there is no include directory for GMP except for the toplevel directory...
find_path(
		GMP_INCLUDE_DIR
		NAMES
			gmp.h
			gmpxx.h
		HINTS
			${GMP_HEADER_DIR}
			${GMP_ROOT}/include
			${GMP_ROOT}
		PATH_SUFFIXES
			GMP gmp Gmp mpir Mpir MPIR
)
mark_as_advanced(GMP_INCLUDE_DIR)

find_library(
		GMP_LIBRARY
		NAMES
		"gmp"
		"gmp-9"
		"gmp-10"
		"gmp-11"
		"gmp-12"
		"mpir"
		"libmpir"
		"mpir.lib"
		"libmpir.lib"
		"libgmp.lib"
		"libgmp-9.lib"
		"libgmp-10.lib"
		"libgmp-11.lib"
		"libgmp-12.lib"
		HINTS
		${GMP_LIBRARY_DIR}
		${GMP_ROOT}/lib
		${GMP_ROOT}
		PATH_SUFFIXES
		GMP
		gmp
		Gmp
		mpir
		Mpir
		MPIR
)
mark_as_advanced(GMP_LIBRARY)

find_library(
		GMPXX_LIBRARY
		NAMES
		"gmpxx"
		"gmpxx-3"
		"gmpxx-4"
		"gmpxx-5"
		"gmpxx-6"
		"mpirxx"
		"libmpirxx"
		"libmpirxx.lib"
		"libgmpxx-3.lib"
		"libgmpxx-4.lib"
		"libgmpxx-5.lib"
		"libgmpxx-6.lib"
		HINTS
		${GMPXX_LIBRARY_DIR}
		${GMP_LIBRARY_DIR}
		${GMP_ROOT}/lib
		${GMP_ROOT}
		PATH_SUFFIXES
		GMP
		gmp
		Gmp
		mpir
		Mpir
		MPIR
)

find_library(
		MPFR_LIBRARY
		NAMES
		"mpfr"
		"mpfr-3"
		"mpfr-4"
		"mpfr-5"
		"mpfr-6"
		"libmpfr.lib"
		"libmpfr-3.lib"
		"libmpfr-4.lib"
		"libmpfr-5.lib"
		"libmpfr-6.lib"
		HINTS
		${MPFR_LIBRARY_DIR}
		${GMP_LIBRARY_DIR}
		${MPFR_ROOT}/lib
		${MPFR_ROOT}
		${GMP_ROOT}/lib
		${GMP_ROOT}
		PATH_SUFFIXES
		MPFR
		mpfr
		Mpfr
)
mark_as_advanced(MPFR_LIBRARY)

find_path(
		MPFR_INCLUDE_DIR
		NAMES
		mpfr.h
		HINTS
		${MPFR_HEADER_DIR}
		${GMP_HEADER_DIR}
		${MPFR_ROOT}/include
		${MPFR_ROOT}
		${GMP_ROOT}/include
		${GMP_ROOT}
		PATH_SUFFIXES
		MPFR
		mpfr
		Mpfr
)
mark_as_advanced(MPFR_INCLUDE_DIR)

find_package_handle_standard_args(GMPXX DEFAULT_MSG
		GMPXX_LIBRARY GMP_LIBRARY GMP_INCLUDE_DIR MPFR_LIBRARY MPFR_INCLUDE_DIR)

if(GMPXX_FOUND)
	util_add_imported_library(ivarp::GMP ${GMP_LIBRARY} ${GMP_INCLUDE_DIR})
	util_add_imported_library(ivarp::MPFR ${MPFR_LIBRARY} ${MPFR_INCLUDE_DIR})
	util_add_imported_library(ivarp::GMPXX ${GMPXX_LIBRARY} ${GMP_INCLUDE_DIR})
	util_imported_link_libraries(ivarp::MPFR ivarp::GMP)
	util_imported_link_libraries(ivarp::GMPXX ivarp::GMP)
endif()

endif()
