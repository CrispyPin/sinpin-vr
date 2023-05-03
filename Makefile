VERSION=v0.2.2

CXX := g++
# CXX := clang++
CPPFLAGS := -g -Wall -std=c++17
LFLAGS := -lX11 -lXrandr -lXtst -lglfw -lGL
OVR := -Lopenvr -lopenvr_api
TARGET := ./sinpin_vr

build:
	$(CXX) src/*.cpp $(CPPFLAGS) $(LFLAGS) -Wl,-rpath,'$$ORIGIN/openvr' $(OVR) -o $(TARGET)

release: build
	zip -r sinpin_vr-$(VERSION).zip sinpin_vr bindings openvr/libopenvr_api.so

run: build
	$(TARGET)

