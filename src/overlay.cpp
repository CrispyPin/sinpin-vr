#include "overlay.h"
#include "app.h"
#include "util.h"
#include <glm/fwd.hpp>

Overlay::Overlay()
{
	_initialized = false;
}

Overlay::Overlay(App *app, std::string name)
{
	_initialized = true;
	_name = name;
	_app = app;
	_is_held = false;
	_active_hand = 0;
	_width_m = 1;

	_target.type = TargetType::World;

	auto overlay_create_err = _app->vr_overlay->CreateOverlay(_name.c_str(), _name.c_str(), &_id);
	assert(overlay_create_err == 0);
	{
		vr::ETrackingUniverseOrigin origin;
		_app->vr_overlay->GetOverlayTransformAbsolute(_id, &origin, &_target.transform);
	}

	uint8_t col[4] = {20, 50, 50, 255};
	_app->vr_overlay->SetOverlayRaw(_id, &col, 1, 1, 4);
	printf("Created overlay instance %s\n", _name.c_str());

	// (flipping uv on y axis because opengl and xorg are opposite)
	vr::VRTextureBounds_t bounds{0, 1, 1, 0};
	_app->vr_overlay->SetOverlayTextureBounds(_id, &bounds);
	SetHidden(false);
}

OverlayID Overlay::Id()
{
	return _id;
}

bool Overlay::IsHeld()
{
	return _is_held;
}

TrackerID Overlay::ActiveHand()
{
	return _active_hand;
}

bool Overlay::IsHidden()
{
	return _hidden;
}

float Overlay::Alpha()
{
	return _alpha;
}

float Overlay::Width()
{
	return _width_m;
}

void Overlay::SetWidth(float width_meters)
{
	_width_m = width_meters;
	_app->vr_overlay->SetOverlayWidthInMeters(_id, _width_m);
}

void Overlay::SetHidden(bool state)
{
	_hidden = state;
	if (_hidden)
		_app->vr_overlay->HideOverlay(_id);
	else
		_app->vr_overlay->ShowOverlay(_id);
}

void Overlay::SetAlpha(float alpha)
{
	_alpha = alpha;
	_app->vr_overlay->SetOverlayAlpha(_id, alpha);
}

void Overlay::SetTexture(vr::Texture_t *texture)
{
	auto set_texture_err = _app->vr_overlay->SetOverlayTexture(_id, texture);
	assert(set_texture_err == 0);
}

void Overlay::SetTransformTracker(TrackerID tracker, VRMat *transform)
{
	_app->vr_overlay->SetOverlayTransformTrackedDeviceRelative(_id, tracker, transform);
	_target.type = TargetType::Tracker;
	_target.id = tracker;
	_target.transform = *transform;
}

void Overlay::SetTransformWorld(VRMat *transform)
{
	_app->vr_overlay->SetOverlayTransformAbsolute(_id, vr::TrackingUniverseStanding, transform);
	_target.type = TargetType::World;
	_target.transform = *transform;
}

void Overlay::SetTargetWorld()
{
	auto abs_pose = ConvertMat(GetTransformAbsolute());
	_app->vr_overlay->SetOverlayTransformAbsolute(_id, vr::TrackingUniverseStanding, &abs_pose);
	_target.type = TargetType::World;
}

glm::mat4x4 Overlay::GetTransformAbsolute()
{
	if (_is_held)
	{
		VRMat pose;
		auto err = _app->vr_overlay->GetOverlayTransformTrackedDeviceRelative(_id, &_active_hand, &pose);
		assert(err == 0);
		auto offset = ConvertMat(pose);
		auto controller = _app->GetTrackerPose(_active_hand);
		return controller * offset;
	}
	else
	{
		switch (_target.type)
		{
		case TargetType::World: {
			VRMat pose;
			vr::ETrackingUniverseOrigin tracking_universe;
			_app->vr_overlay->GetOverlayTransformAbsolute(_id, &tracking_universe, &pose);
			return ConvertMat(pose);
		}
		case TargetType::Tracker: {
			VRMat pose;
			_app->vr_overlay->GetOverlayTransformTrackedDeviceRelative(_id, &_target.id, &pose);
			auto offset = ConvertMat(pose);
			auto tracker_pose = _app->GetTrackerPose(_target.id);
			return tracker_pose * offset;
		}
		}
	}
}

void Overlay::Update()
{
	if (!_initialized)
	{
		printf("Error: overlay %s is not initialized.\n", _name.c_str());
		assert(_initialized);
	}

	if (!_is_held)
	{
		for (auto controller : _app->GetControllers())
		{
			if (_app->IsInputJustPressed(controller, _app->_input_handles.grab))
			{
				auto overlay_pose = GetTransformAbsolute();
				auto controller_pos = GetPos(_app->GetTrackerPose(controller));

				auto local_pos = glm::inverse(overlay_pose) * glm::vec4(controller_pos - GetPos(overlay_pose), 0);

				float grab_area_thickness = 0.3f;
				bool close_enough = glm::abs(local_pos.z) < grab_area_thickness;
				close_enough &= glm::abs(local_pos.x) < _width_m / 2.0f;
				close_enough &= glm::abs(local_pos.y) < _width_m / 2.0f;

				if (close_enough)
				{
					ControllerGrab(controller);
				}
			}
		}
	}
	else
	{
		if (!_app->GetControllerInputDigital(_active_hand, _app->_input_handles.grab).bState)
		{
			ControllerRelease();
		}
	}
}

void Overlay::ControllerGrab(TrackerID controller)
{
	_app->vr_overlay->SetOverlayColor(_id, 0.6f, 0.8f, 0.8f);

	auto abs_mat = GetTransformAbsolute();
	auto controller_mat = _app->GetTrackerPose(controller);
	VRMat relative_pose = ConvertMat(glm::inverse(controller_mat) * abs_mat);

	_app->vr_overlay->SetOverlayTransformTrackedDeviceRelative(_id, controller, &relative_pose);

	if (_GrabBeginCallback != nullptr)
	{
		_GrabBeginCallback(controller);
	}

	_is_held = true;
	_active_hand = controller;
}

void Overlay::ControllerRelease()
{
	_app->vr_overlay->SetOverlayColor(_id, 1.0f, 1.0f, 1.0f);

	auto new_pose = ConvertMat(GetTransformAbsolute());
	_app->vr_overlay->SetOverlayTransformAbsolute(_id, _app->_tracking_origin, &new_pose);

	if (_GrabEndCallback != nullptr)
	{
		_GrabEndCallback(_active_hand);
	}
	_is_held = false;
	_active_hand = -1;
}
