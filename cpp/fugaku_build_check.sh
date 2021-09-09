SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp"

mpiFCCpx -I. -I./eigen -Kfast -DNDEBUG -std=c++11 -o main_check_one_by_one.out main_check_one_by_one.cpp ${SOURCE_FILES}
