
# CC := g++
CC := clang++
LFLAGS := -lX11 -lXrandr -lglfw -lGL
LIBS := openvr/libopenvr_api.so
SRC := src/*.cpp
OUT := ./sinpin_vr
CPPFLAGS := -Wall -std=c++17 $(LFLAGS) $(LIBS) $(SRC) -o $(OUT)

build:
	$(CC) -g $(CPPFLAGS)

release:
	$(CC) $(CPPFLAGS)

run: build
	$(OUT)

