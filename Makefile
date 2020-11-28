FLAGS = -Wall -Wextra

ep3: main.cpp fs_sim.o
	g++ $^ -o $@ $(FLAGS)


fs_sim.o: fs_sim.cpp fs_sim.hpp
	g++ -c $< $(FLAGS)
