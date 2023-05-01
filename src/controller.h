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

	void Update(float dtime);

	bool _cursor_active = false;

  private:
	void UpdateStatus();
	void UpdateLaser();

	void UpdateMouseButton(vr::VRActionHandle_t binding, unsigned int button);

	App *_app;
	Overlay _laser;
	ControllerSide _side;
	TrackerID _device_index;
	vr::VRInputValueHandle_t _input_handle;

	bool _is_connected;

	Overlay *_grabbed_overlay;

	Ray _last_ray;
	glm::vec3 _last_rotation;
	glm::vec3 _last_pos;

	float _last_sent_scroll = 0;
	bool _mouse_drag_lock = false;
	glm::vec2 _last_set_mouse_pos;
};
