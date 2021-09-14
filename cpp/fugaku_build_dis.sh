SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp"

mpiFCCpx -Nclang -I. -I./eigen -Kfast -DNDEBUG -std=c++11 -o main_filter_distinguishable.out main_filter_distinguishable.cpp ${SOURCE_FILES}
