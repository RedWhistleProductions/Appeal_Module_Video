CXX ?= g++
PLUGIN_NAME ?= $(notdir $(CURDIR))
ifeq ($(OS),Windows_NT)
 TARGET=$(PLUGIN_NAME).dll
 FLAGS=-shared
else
 TARGET=$(PLUGIN_NAME).so
 FLAGS=-shared -fPIC
endif
all: $(TARGET)
$(TARGET): src/main.cpp ../src/Video_Plugin_API.h
	$(CXX) -std=c++17 -Wall -Wextra $(FLAGS) src/main.cpp -o $@
.PHONY: all
