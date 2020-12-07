FLAGS = -Wall -g

ep3: main.cpp fs_sim.o
	g++ -g $^ -o $@ $(FLAGS)


fs_sim.o: fs_sim.cpp fs_sim.hpp
	g++ -g -c $< $(FLAGS)
