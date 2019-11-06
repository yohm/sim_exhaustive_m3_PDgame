SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp TraceGSNegatives.cpp CheckEfficiencyByGraph.cpp"

mpiFCCpx -I. -Kfast -DNDEBUG -std=c++11 -Xg -o main_filter_efficient_defensible.out main_filter_efficient_defensible.cpp ${SOURCE_FILES}
