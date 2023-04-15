#pragma once

#include "overlay.h"
#include "util.h"

class App;

class Laser
{
  public:
	Laser(App *app, TrackerID index);

	void Update();
	void SetHidden(bool state);

  private:
	App *_app;
	Overlay _overlay;
	TrackerID _controller;
};
