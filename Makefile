CPFLAGS = -g -Wall -O2 -std=c++17 -lm -w


test: main.cpp
		g++ main.cpp -o main $(CPFLAGS)

clean:
		rm test
