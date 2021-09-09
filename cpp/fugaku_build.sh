SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp TraceGSNegatives.cpp CheckEfficiencyByGraph.cpp"

mpiFCCpx -I. -I./eigen -Kfast -DNDEBUG -std=c++11 -o main_filter_efficient_defensible.out main_filter_efficient_defensible.cpp ${SOURCE_FILES}
