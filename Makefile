CXX = clang++
CXXFLAGS = -g -Wall -Wextra -std=c++20

TARGET = tbd
SOURCES = $(wildcard *.cpp) $(wildcard third_party/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

CXXFLAGS += -Ithird_party
CXXFLAGS += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL third_party/libraylib.a

.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: clean
clean:
	rm -f ./$(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
