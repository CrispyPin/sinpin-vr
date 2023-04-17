#include "overlay.h"
#include "app.h"
#include "util.h"
#include <cstdint>

Overlay::Overlay()
{
	_initialized = false;
}

Overlay::Overlay(App *app, std::string name)
{
	_initialized = true;
	_name = name;
	_app = app;
	_holding_controller = nullptr;
	_width_m = 1;
	_ratio = 1;
	_hidden = false;

	_target = Target{.type = TargetType::World, .transform = VRMatIdentity};

	auto overlay_create_err = _app->vr_overlay->CreateOverlay(_name.c_str(), _name.c_str(), &_id);
	assert(overlay_create_err == 0);

	// (flipping uv on y axis because opengl and xorg are opposite)
	vr::VRTextureBounds_t bounds{0, 1, 1, 0};
	_app->vr_overlay->SetOverlayTextureBounds(_id, &bounds);
	_app->vr_overlay->ShowOverlay(_id);
	printf("Created overlay instance %s\n", _name.c_str());
}

OverlayID Overlay::Id()
{
	return _id;
}

bool Overlay::IsHeld()
{
	return _holding_controller != nullptr;
}

Controller *Overlay::ActiveHand()
{
	return _holding_controller;
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

float Overlay::Ratio()
{
	return _ratio;
}

void Overlay::SetWidth(float width_meters)
{
	_width_m = width_meters;
	_app->vr_overlay->SetOverlayWidthInMeters(_id, _width_m);
}

void Overlay::SetHidden(bool state)
{
	if (state != _hidden)
	{
		_hidden = state;
		if (_hidden)
			_app->vr_overlay->HideOverlay(_id);
		else
			_app->vr_overlay->ShowOverlay(_id);
	}
}

void Overlay::SetAlpha(float alpha)
{
	_alpha = alpha;
	_app->vr_overlay->SetOverlayAlpha(_id, alpha);
}

void Overlay::SetRatio(float ratio)
{
	_ratio = ratio;
}

void Overlay::SetTexture(vr::Texture_t *texture)
{
	auto set_texture_err = _app->vr_overlay->SetOverlayTexture(_id, texture);
	assert(set_texture_err == 0);
}

void Overlay::SetTextureToColor(uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t col[4] = {r, g, b, 255};
	auto set_texture_err = _app->vr_overlay->SetOverlayRaw(_id, &col, 1, 1, 4);
	assert(set_texture_err == 0);
}

void Overlay::SetTransformTracker(TrackerID tracker, const VRMat *transform)
{
	auto original_pose = _target.transform;
	_app->vr_overlay->SetOverlayTransformTrackedDeviceRelative(_id, tracker, transform);
	_target.type = TargetType::Tracker;
	_target.id = tracker;
	_target.transform = *transform;

	auto relative_transform = ConvertMat(*transform) * glm::inverse(ConvertMat(original_pose));
	for (auto child : _children)
	{
		if (!child->IsHeld() && child->_target.type == TargetType::Tracker)
		{
			VRMat local_transform = ConvertMat(relative_transform * ConvertMat(child->_target.transform));
			child->SetTransformTracker(tracker, &local_transform);
		}
	}
}

void Overlay::SetTransformWorld(const VRMat *transform)
{
	_app->vr_overlay->SetOverlayTransformAbsolute(_id, vr::TrackingUniverseStanding, transform);
	_target.type = TargetType::World;
	_target.transform = *transform;
}

void Overlay::SetTargetTracker(TrackerID tracker)
{
	auto abs_mat = GetTransformAbsolute();
	auto controller_mat = _app->GetTrackerPose(tracker);
	VRMat relative_pose = ConvertMat(glm::inverse(controller_mat) * abs_mat);
	SetTransformTracker(tracker, &relative_pose);
}

void Overlay::SetTargetWorld()
{
	auto abs_pose = ConvertMat(GetTransformAbsolute());
	SetTransformWorld(&abs_pose);
}

Ray Overlay::IntersectRay(glm::vec3 origin, glm::vec3 direction, float max_len)
{
	float closest_dist = max_len;
	auto end = origin + direction * max_len;

	auto panel_transform = GetTransformAbsolute();
	auto panel_pos = GetPos(panel_transform);
	auto a = glm::inverse(panel_transform) * glm::vec4(origin - panel_pos, 0);
	auto b = glm::inverse(panel_transform) * glm::vec4(end - panel_pos, 0);
	float r = a.z / (a.z - b.z);
	auto p = a + (b - a) * r;
	// printf("panel pos: (%.2f,%.2f,%.2f)\n", panel_pos.x, panel_pos.y, panel_pos.z);
	// printf("a: (%.2f,%.2f,%.2f)\n", a.x, a.y, a.z);
	// printf("b: (%.2f,%.2f,%.2f)\n", b.x, b.y, b.z);
	// printf("r: %.2f\n", r);
	// printf("p: (%.2f,%.2f,%.2f)\n", p.x, p.y, p.z);

	if (b.z < a.z && b.z < 0 && glm::abs(p.x) < (_width_m * 0.5f) && glm::abs(p.y) < (_width_m * 0.5f * _ratio))
	{
		float dist = r * max_len;
		if (dist < closest_dist)
		{
			closest_dist = dist;
		}
	}
	return Ray{.overlay = this, .distance = closest_dist};
}

glm::mat4x4 Overlay::GetTransformAbsolute()
{
	if (_holding_controller != nullptr)
	{
		VRMat pose;
		TrackerID tracker;
		auto err = _app->vr_overlay->GetOverlayTransformTrackedDeviceRelative(_id, &tracker, &pose);
		assert(err == 0);
		auto offset = ConvertMat(pose);
		auto controller = _app->GetTrackerPose(_holding_controller->DeviceIndex());
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

Target *Overlay::GetTarget()
{
	return &_target;
}

void Overlay::Update()
{
	if (!_initialized)
	{
		printf("Error: overlay %s is not initialized.\n", _name.c_str());
		assert(_initialized);
	}

	if (_holding_controller != nullptr)
	{
		if (!_app->GetInputDigital(_app->_input_handles.grab, _holding_controller->InputHandle()).bState)
		{
			ControllerRelease();
		}
	}
}

void Overlay::ControllerGrab(Controller *controller)
{
	_app->vr_overlay->SetOverlayColor(_id, 0.6f, 0.8f, 0.8f);
	SetTargetTracker(controller->DeviceIndex());

	for (auto child : _children)
	{
		if (!child->IsHeld()) // overlay may have been picked up by other controller
		{
			child->SetTargetTracker(controller->DeviceIndex());
		}
	}
	_holding_controller = controller;
}

void Overlay::ControllerRelease()
{
	if (_holding_controller != nullptr)
	{
		_holding_controller->ReleaseOverlay();
	}
	_app->vr_overlay->SetOverlayColor(_id, 1.0f, 1.0f, 1.0f);

	SetTargetWorld();
	for (auto child : _children)
	{
		if (!child->IsHeld()) // overlay may have been picked up by other controller
		{
			child->SetTargetWorld();
		}
	}
	_holding_controller = nullptr;
}

void Overlay::AddChildOverlay(Overlay *overlay)
{
	_children.push_back(overlay);
}

void Overlay::RemoveChildOverlay(Overlay *overlay)
{
	for (auto i = _children.begin(); i != _children.end(); i++)
	{
		if (*i == overlay)
		{
			_children.erase(i);
			break;
		}
	}
}