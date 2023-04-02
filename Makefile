

build:
	CPATH=. g++ -lX11 -lglfw -lGL openvr/libopenvr_api.so src/main.cpp -o overlay

run: build
	./overlay

