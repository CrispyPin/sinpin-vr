VERSION=v0.2.3

CXX := g++
# CXX := clang++
CPPFLAGS := -g -Wall -std=c++17
LFLAGS := -lX11 -lXrandr -lXtst -lglfw -lGL
OVR := -Llib -lopenvr_api
TARGET := ./sinpin_vr

build:
	$(CXX) src/*.cpp $(CPPFLAGS) $(LFLAGS) -Wl,-rpath,'$$ORIGIN/lib' $(OVR) -o $(TARGET)

release: build
	mkdir -p sinpin-vr/lib
	cp lib/libopenvr_api.so sinpin-vr/lib
	cp -r bindings sinpin-vr
	cp sinpin_vr sinpin-vr
	tar -caf sinpin-vr-$(VERSION).tar.xz sinpin-vr
	rm -rf sinpin-vr

run: build
	$(TARGET)

