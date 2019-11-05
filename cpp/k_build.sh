SOURCE_FILES="Action.cpp Strategy.cpp DirectedGraph.cpp TraceGSNegatives.cpp TopologicalEfficiency.cpp"

mpiFCCpx -I. -Kfast -DNDEBUG -std=c++11 -Xg -o main_topological_efficiency.out main_topological_efficiency.cpp ${SOURCE_FILES}
