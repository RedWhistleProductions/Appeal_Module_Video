CXX ?= g++
CXXFLAGS += -std=c++17 -Wall -Wextra -Wno-cast-function-type
ifeq ($(OS),Windows_NT)
 TARGET=Appeal_Module_Video.dll
 SHARED=-shared
else
 TARGET=Appeal_Module_Video.so
 SHARED=-shared -fPIC
 LDLIBS=-ldl
endif
SOURCES=$(wildcard src/*.cpp)
all: $(TARGET)
$(TARGET): $(SOURCES) $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) $(SHARED) $(SOURCES) -o $@ $(LDLIBS)
.PHONY: all
