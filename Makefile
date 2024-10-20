CXX = clang++
CXXFLAGS = -g -Wall -Wextra -std=c++20

TARGET = tbd
SOURCES = $(wildcard *.cpp) $(wildcard third_party/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPS = $(SOURCES:.cpp=.d)

CXXFLAGS += -Ithird_party
CXXFLAGS += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL third_party/libraylib.a

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS) $(DEPS)

-include $(DEPS)
