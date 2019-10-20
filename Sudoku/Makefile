CC=g++
CFLAGS=-I. --std=c++17 -O3
PROFILE_FLAGS=-g
DEPS = sudoku.h
OBJ = sudoku.o main.o

%.o : %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(PROFILE_FLAGS)

sudoku: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(PROFILE_FLAGS)

clean:
	rm -f *.o sudoku
