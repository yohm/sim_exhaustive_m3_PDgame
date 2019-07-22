SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp"

mpiFCCpx -I. -Kfast -DNDEBUG -std=c++11 -Xg -o main_distinguishability.out main_distinguishability.cpp ${SOURCE_FILES}
