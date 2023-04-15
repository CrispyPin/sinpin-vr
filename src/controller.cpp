#include "controller.h"
#include "app.h"
#include "overlay.h"
#include "util.h"
#include <string>

const float width = 0.004f;

Controller::Controller(App *app, ControllerSide side)
{
	_app = app;
	_input_handle = 0;
	_is_connected = false;
	_side = side;

	std::string laser_name = "controller_laser_";
	if (side == ControllerSide::Left)
		laser_name += "left";
	else if (side == ControllerSide::Right)
		laser_name += "right";

	_laser = Overlay(app, laser_name);
	UpdateStatus();
	_laser.SetTextureToColor(255, 200, 255);
	_laser.SetAlpha(0.2f);
}

TrackerID Controller::DeviceIndex()
{
	return _device_index;
}

vr::VRInputValueHandle_t Controller::InputHandle()
{
	return _input_handle;
}

ControllerSide Controller::Side()
{
	return _side;
}

bool Controller::IsConnected()
{
	return _is_connected;
}

void Controller::Update()
{
	UpdateStatus();
	if (!_is_connected)
		return;

	auto controller_pose = _app->GetTrackerPose(_device_index);
	auto origin = GetPos(controller_pose);
	auto forward = -glm::vec3(controller_pose[2]);
	auto ray = _app->IntersectRay(origin, forward, 5.0f);
	float len = ray.distance;

	VRMat transform = {{{width, 0, 0, 0}, {0, 0, width, 0}, {0, len, 0, len * -0.5f}}};
	_laser.SetTransformTracker(_device_index, &transform);

	if (ray.overlay != nullptr)
	{
		if (_app->IsInputJustPressed(_app->_input_handles.grab, _input_handle))
		{
			ray.overlay->ControllerGrab(this);
		}
	}
}

void Controller::UpdateStatus()
{
	_is_connected = true;

	if (_side == ControllerSide::Left)
	{
		auto err = _app->vr_input->GetInputSourceHandle("/user/hand/left", &_input_handle);
		_is_connected &= (err == 0);
		_device_index = _app->vr_sys->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
	}
	else if (_side == ControllerSide::Right)
	{
		auto err = _app->vr_input->GetInputSourceHandle("/user/hand/right", &_input_handle);
		_is_connected &= (err == 0);
		_device_index = _app->vr_sys->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
	}
	_is_connected &= _device_index < MAX_TRACKERS;
	_laser.SetHidden(!_is_connected);
}
