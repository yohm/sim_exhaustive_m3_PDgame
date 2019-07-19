cmake_minimum_required(VERSION 3.7)
project(cpp)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_VERBOSE_MAKEFILE 1)

set(SOURCE_FILES Action.cpp Action.hpp Strategy.cpp Strategy.hpp DirectedGraph.hpp DirectedGraph.cpp TraceNegativeDefensible.hpp TraceNegativeDefensible.cpp TopologicalEfficiency.hpp TopologicalEfficiency.cpp)
find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH} /usr/local/include/eigen3)

add_executable(test_Strategy test_Strategy.cpp ${SOURCE_FILES})
add_executable(test_DirectedGraph test_DirectedGraph.cpp ${SOURCE_FILES})

add_executable(main_efficient_defensible main_efficient_defensible.cpp ${SOURCE_FILES})
target_link_libraries(main_efficient_defensible ${MPI_LIBRARIES})

add_executable(main_trace_negative_defensible main_trace_negative_defensible.cpp ${SOURCE_FILES})
target_link_libraries(main_trace_negative_defensible ${MPI_LIBRARIES})

add_executable(main_topological_efficiency main_topological_efficiency.cpp ${SOURCE_FILES})
target_link_libraries(main_topological_efficiency ${MPI_LIBRARIES})

add_executable(main_distinguishability main_distinguishability.cpp ${SOURCE_FILES})
target_link_libraries(main_distinguishability ${MPI_LIBRARIES})

add_executable(main_check_one_by_one main_check_one_by_one.cpp ${SOURCE_FILES})
target_link_libraries(main_check_one_by_one ${MPI_LIBRARIES})

add_executable(main_print_ITG main_print_ITG.cpp ${SOURCE_FILES})
target_link_libraries(main_print_ITG ${MPI_LIBRARIES})

add_executable(find_matching find_matching.cpp)
add_executable(count_num_strategies count_num_strategies.cpp)

