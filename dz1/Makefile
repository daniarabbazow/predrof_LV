CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

TARGET = homework1
SOURCES = src/main.cpp src/Part1.cpp src/Part2.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
