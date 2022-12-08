CPP=g++
CPP_FLAG=-static

all: solver

solver: solve.cpp
        $(CPP) solve.cpp -o solver $(CPP_FLAG)

submit: solver
        ./play.py ./solver

clean:
        rm -f solver sudoku