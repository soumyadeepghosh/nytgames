CXX=g++
CXXFLAGS=-I. --std=c++17 -O3
PROFILE_FLAGS=-g
DEPS = sudoku.h
OBJ = sudoku.o main.o

%.o : %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(PROFILE_FLAGS)

sudoku: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(PROFILE_FLAGS)

clean:
	rm -f *.o sudoku
