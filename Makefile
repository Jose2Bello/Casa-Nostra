CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -static

SRC    = src/main.cpp src/persona.cpp src/util.cpp
TARGET = bin/mafia_sucesion.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: all
	$(TARGET)
