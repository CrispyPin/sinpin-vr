#pragma once

#include "app.h"
#include "util.h"

class GrabComponent
{
  public:
	GrabComponent(App *app);
	bool IsHeld();
	TrackerID ActiveHand();
	void Update(OverlayID id, float *meters);

  private:
	void ControllerRelease();
	void ControllerGrab(TrackerID controller);

	App *_app;
	OverlayID _id;

	bool _is_held;
	TrackerID _active_hand;
};