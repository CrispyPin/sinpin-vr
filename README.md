# sinpin-vr
A SteamVR overlay for Linux+X11 that displays all your screens in VR.

https://user-images.githubusercontent.com/54243225/235313348-2ce9b2d8-8458-49f8-ba94-16e577c4f502.mp4

Note: only index controllers have default bindings right now, feel free to make a PR.

## features
- one overlay per screen
- shows cursor position
- global visibility toggle (default: long press left B)
- reset positions (default: long press left A)
- activate cursor input (default: press touchpad)
	- left mouse default: trigger
	- right mouse default: A
	- middle mouse default: not bound
	- scrolling default: joystick up/down
- edit mode (default: long press press right B)
	- move screens around (default: trigger)
	- resize screens (move with two controllers)
	- push/pull screens (default: joystick up/down)
	- move all screens at once with the same controls by grabbing the purple square

## performance
From my limited testing, this uses about half the CPU performance of Steam's built-in desktop overlay, if running at 60 FPS. Currently the default is 30 FPS which brings that factor to 3-4x. On my machine, the Steam desktop view increases cpu usage by about 100% of a CPU thread (looking only at the `steam` process), while this overlay uses around 25% at 30 FPS and 45% at 60 FPS.


