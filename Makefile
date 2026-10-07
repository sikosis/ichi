CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
BIN      := ichi
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:.cpp=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all run clean
