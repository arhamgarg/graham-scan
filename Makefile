CXX := clang++
CPPFLAGS += -Iinclude -DNDEBUG
CXXFLAGS += -std=c++17 -O3 -march=native -flto -Wall -Wextra -MMD -MP
LDFLAGS += -flto

BUILD_DIR := build
TARGET := $(BUILD_DIR)/graham-scan
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:src/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all run test benchmark clean

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) --self-test

benchmark: $(TARGET)
	./$(TARGET) --benchmark

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
