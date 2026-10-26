CXX      = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17

all: comprimir descomprimir

comprimir: comprimir.cpp huffman.cpp huffman.h
	$(CXX) $(CXXFLAGS) -o $@ comprimir.cpp huffman.cpp

descomprimir: descomprimir.cpp huffman.cpp huffman.h
	$(CXX) $(CXXFLAGS) -o $@ descomprimir.cpp huffman.cpp

clean:
	rm -f comprimir descomprimir *.huff *.out

.PHONY: all clean
