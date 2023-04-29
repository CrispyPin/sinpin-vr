#pragma once
#include "overlay.h"
#include "util.h"
#include <vector>

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
	Ray GetLastRay();
	glm::vec3 GetLastPos();
	glm::vec3 GetLastRot();

	void ReleaseOverlay();

	void Update();

  private:
	void UpdateStatus();
	void UpdateLaser();

	App *_app;
	Overlay _laser;
	ControllerSide _side;
	TrackerID _device_index;
	vr::VRInputValueHandle_t _input_handle;

	bool _is_connected;

	bool _cursor_active;
	Overlay *_grabbed_overlay;

	Ray _last_ray;
	glm::vec3 _last_rotation;
	glm::vec3 _last_pos;
};
