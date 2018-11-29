CXX = g++
CXXFLAGS = -O3
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

LDFLAGS = -lGL -lglut -lGLU

project3: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) project3
