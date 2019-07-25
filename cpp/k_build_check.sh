SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp"

mpiFCCpx -I. -Kfast -DNDEBUG -std=c++11 -Xg -o main_check_one_by_one.out main_check_one_by_one.cpp ${SOURCE_FILES}
