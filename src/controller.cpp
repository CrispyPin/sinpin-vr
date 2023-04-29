#include "controller.h"
#include "app.h"
#include "overlay.h"
#include "util.h"
#include <string>

const float LASER_WIDTH = 0.004f;
const Color EDIT_COLOR{1, 0.6f, 1};
const Color CURSOR_COLOR{0.3f, 1, 1};
const float SCROLL_SPEED = 48.0f;

Controller::Controller(App *app, ControllerSide side)
{
	_grabbed_overlay = nullptr;
	_app = app;
	_input_handle = 0;
	_is_connected = false;
	_side = side;
	_cursor_active = false;
	_last_sent_scroll = 0;

	std::string laser_name = "controller_laser_";
	if (side == ControllerSide::Left)
		laser_name += "left";
	else if (side == ControllerSide::Right)
		laser_name += "right";

	_laser = Overlay(app, laser_name);
	UpdateStatus();
	_laser.SetTextureToColor(255, 255, 255);
	_laser.SetAlpha(0.2f);
	_laser.SetHidden(true);
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

void Controller::ReleaseOverlay()
{
	_grabbed_overlay = nullptr;
}

Ray Controller::GetLastRay()
{
	return _last_ray;
}

glm::vec3 Controller::GetLastPos()
{
	return _last_pos;
}

glm::vec3 Controller::GetLastRot()
{
	return _last_rotation;
}

void Controller::Update(float dtime)
{
	UpdateStatus();
	if (!_is_connected)
		return;

	UpdateLaser();

	if (_app->_edit_mode)
	{
		_laser.SetColor(EDIT_COLOR);
		if (_last_ray.overlay != nullptr)
		{
			auto ray = _last_ray;
			if (_app->IsInputJustPressed(_app->_input_handles.edit.grab, _input_handle))
			{
				if (ray.overlay->IsHeld())
				{
					ray.overlay->ControllerResize(this);
				}
				else
				{
					_grabbed_overlay = ray.overlay;
					ray.overlay->ControllerGrab(this);
				}
			}
		}

		if (_grabbed_overlay != nullptr)
		{
			float move = _app->GetInputAnalog(_app->_input_handles.edit.distance, _input_handle).y * dtime * 8;
			if (move != 0.0f)
			{
				auto transform = _grabbed_overlay->GetTarget()->transform;
				transform.m[2][3] = glm::clamp(transform.m[2][3] - move, -5.0f, -0.1f); // moving along z axis
				_grabbed_overlay->SetTransformTracker(_device_index, &transform);
			}
		}
	}
	else // cursor mode
	{
		if (_app->IsInputJustPressed(_app->_input_handles.cursor.activate, _input_handle))
		{
			if (!_cursor_active && _app->_active_cursor.has_value())
			{
				_app->_active_cursor.value()->_cursor_active = false;
				_app->_active_cursor = this;
			}
			_cursor_active = !_cursor_active;
			_app->_active_cursor = this;
			_laser.SetColor(CURSOR_COLOR);
		}
		if (_cursor_active)
		{
			// printf("update cursor on hand %d\n", _side);
			if (_last_ray.overlay != nullptr && _last_ray.hit_panel != nullptr)
			{
				auto pos = glm::vec2(_last_ray.local_pos.x, _last_ray.local_pos.y);
				// normalize positions to +-0.5
				pos /= _last_ray.overlay->Width();
				pos.y *= -1;

				// shift to 0-1
				pos.x += 0.5f;
				pos.y += 0.5f * _last_ray.overlay->Ratio();

				pos *= _last_ray.hit_panel->Width();
				_last_ray.hit_panel->SetCursor(pos.x, pos.y);
			}
			UpdateMouseButton(_app->_input_handles.cursor.mouse_left, 1);
			UpdateMouseButton(_app->_input_handles.cursor.mouse_middle, 2);
			UpdateMouseButton(_app->_input_handles.cursor.mouse_right, 3);
			auto scroll_state = _app->GetInputAnalog(_app->_input_handles.cursor.scroll, _input_handle);
			if (scroll_state.y != 0)
			{
				_last_sent_scroll += dtime * glm::abs(scroll_state.y) * SCROLL_SPEED;
				if (_last_sent_scroll > 1)
				{
					_last_sent_scroll = 0;
					if (scroll_state.y > 0)
					{
						_app->SendMouseInput(4, true);
						_app->SendMouseInput(4, false);
					}
					else if (scroll_state.y < 0)
					{
						_app->SendMouseInput(5, true);
						_app->SendMouseInput(5, false);
					}
				}
			}
		}
	}
}

void Controller::UpdateMouseButton(vr::VRActionHandle_t binding, unsigned int button)
{
	auto state = _app->GetInputDigital(binding, _input_handle);
	if (state.bChanged)
	{
		_app->SendMouseInput(button, state.bState);
	}
}

void Controller::UpdateLaser()
{
	auto controller_pose = _app->GetTrackerPose(_device_index);
	auto controller_pos = GetPos(controller_pose);
	auto forward = -glm::vec3(controller_pose[2]);
	auto ray = _app->IntersectRay(controller_pos, forward, 8.0f);
	float len = ray.distance;

	_last_pos = controller_pos;
	_last_rotation = forward;
	_last_ray = ray;

	auto hmd_global_pos = GetPos(_app->GetTrackerPose(0));
	auto hmd_local_pos = glm::inverse(controller_pose) * glm::vec4(hmd_global_pos - controller_pos, 0);
	hmd_local_pos.z = 0;
	auto hmd_dir = glm::normalize(hmd_local_pos);

	VRMat transform = {{{LASER_WIDTH * hmd_dir.y, 0, LASER_WIDTH * hmd_dir.x, 0}, {LASER_WIDTH * -hmd_dir.x, 0, LASER_WIDTH * hmd_dir.y, 0}, {0, len, 0, len * -0.5f}}};
	_laser.SetTransformTracker(_device_index, &transform);
	_laser.SetHidden(!_is_connected || _app->_hidden || (!_app->_edit_mode && !_cursor_active));
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
}
