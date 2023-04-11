

build:
	g++ -Wall -lX11 -lXrandr -lglfw -lGL openvr/libopenvr_api.so src/*.cpp -o overlay

run: build
	./overlay

