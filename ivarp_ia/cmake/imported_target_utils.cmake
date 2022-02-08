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
# These utilities are used to generate imported targets for libraries found by our FindPackage scripts,
# for packages with non-existant or broken cmake support.

# add imported target for a found library, given the libraries location and include directories
function(util_add_imported_library TARGETNAME LOCATION INCLUDE_DIRS)
	# create the imported target
	add_library(${TARGETNAME} UNKNOWN IMPORTED)

	# check if the library has a debug and an optimized version
	string(REGEX MATCH "debug;([^;]+);optimized;([^;]+)" _DEBOPT_MATCHED "${LOCATION}")
	if(_DEBOPT_MATCHED)
		set_target_properties(${TARGETNAME} PROPERTIES
				IMPORTED_LOCATION "${CMAKE_MATCH_2}"
				IMPORTED_LOCATION_RELEASE "${CMAKE_MATCH_2}"
				IMPORTED_LOCATION_MINSIZEREL "${CMAKE_MATCH_2}"
				IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_MATCH_2}"
				IMPORTED_LOCATION_DEBUG "${CMAKE_MATCH_1}"
				)
	else()
		string(REGEX MATCH "optimized;([^;]+);debug;([^;]+)" _DEBOPT_MATCHED "${LOCATION}")
		if(_DEBOPT_MATCHED)
			set_target_properties(${TARGETNAME} PROPERTIES
					IMPORTED_LOCATION "${CMAKE_MATCH_1}"
					IMPORTED_LOCATION_RELEASE "${CMAKE_MATCH_1}"
					IMPORTED_LOCATION_MINSIZEREL "${CMAKE_MATCH_1}"
					IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_MATCH_1}"
					IMPORTED_LOCATION_DEBUG "${CMAKE_MATCH_2}"
					)
		else()
			set_target_properties(${TARGETNAME} PROPERTIES IMPORTED_LOCATION "${LOCATION}")
		endif()
	endif()

	# if there are include dirs, add them
	if(NOT "${INCLUDE_DIRS}" STREQUAL "")
		set_target_properties(${TARGETNAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIRS}")
	endif()
endfunction()

# used to add libraries (preferrably imported targets) to an imported library;
# makes sure the dependencies are linked against transitively
function(util_imported_link_libraries TARGET LIBRARIES)
	get_target_property(_EXISTING_LIBS ${TARGET} INTERFACE_LINK_LIBRARIES)
	if(NOT _EXISTING_LIBS)
		set(_EXISTING_LIBS "")
	endif()

	foreach(LIBNAME IN LISTS LIBRARIES)
		if(NOT "${LIBNAME}" STREQUAL "")
			set(_EXISTING_LIBS "${_EXISTING_LIBS};${LIBNAME}")

			if(TARGET ${LIBNAME})
				get_target_property(_LIBNAME_INTERFACE_LIBS ${LIBNAME} INTERFACE_LINK_LIBRARIES)
				if(_LIBNAME_INTERFACE_LIBS)
					set(_EXISTING_LIBS "${_EXISTING_LIBS};${_LIBNAME_INTERFACE_LIBS}")
				endif()
			endif()
		endif()
	endforeach(LIBNAME)

	set_target_properties(${TARGET} PROPERTIES INTERFACE_LINK_LIBRARIES "${_EXISTING_LIBS}")
endfunction()

