CXX ?= g++
CXXFLAGS ?= -O3 -std=c++17 -Wall -Wextra
LDFLAGS ?=

ROOT := $(CURDIR)
BUILD_DIR := $(ROOT)/build/make
TARGET := $(BUILD_DIR)/hull
SRC := $(ROOT)/src/main.cpp $(ROOT)/src/avl.cpp
INCLUDE := -I$(ROOT)/include

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC) -o $@ $(LDFLAGS)

run: $(TARGET)
	$(TARGET) --self-test

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
