cmake_minimum_required(VERSION 3.7)
project(cpp)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_VERBOSE_MAKEFILE 1)

set(SOURCE_FILES Action.cpp Action.hpp Strategy.cpp Strategy.hpp DirectedGraph.hpp DirectedGraph.cpp)
find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH} /usr/local/include/eigen3)

add_executable(test_Strategy test_Strategy.cpp ${SOURCE_FILES})
add_executable(test_DirectedGraph test_DirectedGraph.cpp ${SOURCE_FILES})

add_executable(main_defensible main_defensible.cpp ${SOURCE_FILES})
target_link_libraries(main_defensible ${MPI_LIBRARIES})

add_executable(main_efficient_defensible main_efficient_defensible.cpp ${SOURCE_FILES})
target_link_libraries(main_efficient_defensible ${MPI_LIBRARIES})

add_executable(main_trace_negative_defensible main_trace_negative_defensible.cpp ${SOURCE_FILES})
target_link_libraries(main_trace_negative_defensible ${MPI_LIBRARIES})

add_executable(main_reject_cannot_be_efficient main_reject_cannot_be_efficient.cpp ${SOURCE_FILES})
target_link_libraries(main_reject_cannot_be_efficient ${MPI_LIBRARIES})

add_executable(main_efficiency main_efficiency.cpp ${SOURCE_FILES})
target_link_libraries(main_efficiency ${MPI_LIBRARIES})

add_executable(main_print_ITG main_print_ITG.cpp ${SOURCE_FILES})

add_executable(main_topological_efficiency main_topological_efficiency.cpp ${SOURCE_FILES})
target_link_libraries(main_topological_efficiency ${MPI_LIBRARIES})

