#pragma once
#include "overlay.h"
#include "util.h"

class App;

enum class ControllerSide
{
	Left,
	Right
};

class Controller
{
  public:
	Controller(App *app, ControllerSide hand);
	TrackerID DeviceIndex();
	vr::VRInputValueHandle_t InputHandle();
	ControllerSide Side();

	bool IsConnected();

	void Update();
	void UpdateStatus();

  private:
	App *_app;
	Overlay _laser;
	ControllerSide _side;
	TrackerID _device_index;
	vr::VRInputValueHandle_t _input_handle;
	bool _is_connected;
};
