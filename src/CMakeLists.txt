add_executable(triangle_cover_by_disks main.cpp acute_isoceles.cpp below_45_isoceles.cpp
	                                   below_45_isoceles_derivatives.cpp
	                                   equilateral.cpp halfsquares.cpp)
target_link_libraries(triangle_cover_by_disks PRIVATE ivarp_ia)
