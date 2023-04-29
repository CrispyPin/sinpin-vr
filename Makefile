VERSION=v0.2.0
# CC := g++
CC := clang++
LFLAGS := -lX11 -lXrandr -lXtst -lglfw -lGL
LIBS := openvr/libopenvr_api.so
SRC := src/*.cpp
OUT := ./sinpin_vr
CPPFLAGS := -Wall -std=c++17 $(LFLAGS) $(LIBS) $(SRC) -o $(OUT)

build:
	$(CC) -g $(CPPFLAGS)

release: build
	zip -r sinpin_vr-$(VERSION).zip sinpin_vr bindings openvr/libopenvr_api.so

run: build
	$(OUT)

