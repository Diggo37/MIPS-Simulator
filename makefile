CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -O2 -Wall

SRC = src/util.cpp src/parser.cpp src/cpu.cpp main.cpp

all: mipssim

mipssim: $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o mipssim

clean:
	rm -f mipssim *.o
