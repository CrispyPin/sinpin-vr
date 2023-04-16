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
	_hidden = false;

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

void Controller::SetHidden(bool state)
{
	_hidden = state;
}

void Controller::RegisterGrabbedOverlay(Overlay *overlay)
{
	_grabbed_overlays.push_back(overlay);
}

void Controller::ReleaseOverlay(Overlay *overlay)
{
	for (auto i = _grabbed_overlays.begin(); i != _grabbed_overlays.end(); i++)
	{
		if (*i == overlay)
		{
			_grabbed_overlays.erase(i);
			break;
		}
	}
}

void Controller::Update()
{
	UpdateStatus();
	if (!_is_connected)
		return;

	auto controller_pose = _app->GetTrackerPose(_device_index);
	auto controller_pos = GetPos(controller_pose);
	auto forward = -glm::vec3(controller_pose[2]);
	auto ray = _app->IntersectRay(controller_pos, forward, 5.0f);
	float len = ray.distance;

	auto hmd_global_pos = GetPos(_app->GetTrackerPose(0));
	auto hmd_local_pos = glm::inverse(controller_pose) * glm::vec4(hmd_global_pos - controller_pos, 0);
	hmd_local_pos.z = 0;
	auto hmd_dir = glm::normalize(hmd_local_pos);

	VRMat transform = {{{width * hmd_dir.y, 0, width * hmd_dir.x, 0}, {width * -hmd_dir.x, 0, width * hmd_dir.y, 0}, {0, len, 0, len * -0.5f}}};
	_laser.SetTransformTracker(_device_index, &transform);

	if (ray.overlay != nullptr)
	{
		if (_app->IsInputJustPressed(_app->_input_handles.grab, _input_handle))
		{
			ray.overlay->ControllerGrab(this);
		}
	}

	if (!_grabbed_overlays.empty())
	{
		float move = _app->GetInputAnalog(_app->_input_handles.distance).y * 0.1; // TODO use frame time
		if (move != 0.0f)
		{
			// delta is calculated & clamped for first overlay so that child overlays don't move further than the root
			float main_z = _grabbed_overlays[0]->GetTarget()->transform.m[2][3];
			float new_main_z = glm::clamp(main_z - move, -5.0f, -0.1f);
			float real_delta = new_main_z - main_z;

			for (auto overlay : _grabbed_overlays)
			{
				overlay->GetTarget()->transform.m[2][3] += real_delta;
				_app->vr_overlay->SetOverlayTransformTrackedDeviceRelative(overlay->Id(), _device_index, &overlay->GetTarget()->transform);
			}
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
	_laser.SetHidden(!_is_connected || _hidden);
}
