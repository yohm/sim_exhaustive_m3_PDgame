SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp"

mpiFCCpx -I. -Kfast -DNDEBUG -std=c++11 -Xg -o main_filter_distinguishable.out main_filter_distinguishable.cpp ${SOURCE_FILES}
