CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

TARGET = mesh_loader
SOURCES = src/main.cpp src/Mesh.cpp src/MeshTypes.cpp src/AneuMeshLoader.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) meshes/MeshExample.aneu

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all run clean
