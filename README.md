# sinpin-vr
A SteamVR overlay for Linux+X11 that displays all your screens in VR.

https://user-images.githubusercontent.com/54243225/233798783-27d1a6ae-b71d-448f-bb67-76015e539452.mp4

## features
- one overlay per screen
- shows cursor position
- global visibility toggle (default: press left touch)
- reset positions (default: long press left A)
- edit mode (default: press right touch)
	- move screens around (default: trigger)
	- resize screens (move with two controllers)
	- push/pull screens (default: joystick up/down)
	- move all screens at once with the same controls by grabbing the purple square

## performance
From my limited testing, this uses about half the CPU performance of Steam's built-in desktop overlay, if running at 60 FPS. Currently the default is 30 FPS which brings that factor to 3-4x. On my machine, the Steam desktop view increases cpu usage by about 100% of a CPU thread (looking only at the `steam` process), while this overlay uses around 25% at 30 FPS and 45% at 60 FPS.


