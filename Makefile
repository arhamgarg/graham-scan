CXX ?= c++
CPPFLAGS := -Iinclude
CXXFLAGS ?= -O3
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic

TARGET := build/make/hull
OBJECTS := build/make/main.o build/make/rbt.o

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

build/make/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

test: $(TARGET)
	./$(TARGET) --self-test

clean:
	rm -rf build/make

-include $(OBJECTS:.o=.d)
