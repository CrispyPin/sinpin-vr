#include "laser.h"
#include "app.h"
#include "util.h"
#include <string>

Laser::Laser(App *app, TrackerID index)
	: _overlay(app, "laser_" + std::to_string(index)),
	  _app(app),
	  _controller(index)
{
	_overlay.SetHidden(true);
	_overlay.SetTransformTracker(index, &VRMatIdentity);
	_overlay.SetTextureToColor(255, 200, 255);
	_overlay.SetAlpha(0.2f);
}

void Laser::Update()
{
	if (_overlay.IsHidden())
	{
		return;
	}
	const float width = 0.004f;
	auto controller_pose = _app->GetTrackerPose(_controller);
	auto origin = GetPos(controller_pose);
	auto forward = -glm::vec3(controller_pose[2]);
	auto ray = _app->IntersectRay(origin, forward, 5.0f);
	float len = ray.distance;

	VRMat transform = {{{width, 0, 0, 0}, {0, 0, width, 0}, {0, len, 0, len * -0.5f}}};
	_overlay.SetTransformTracker(_controller, &transform);

	if (ray.overlay != nullptr)
	{
		if (_app->IsInputJustPressed(_controller, _app->_input_handles.grab))
		{
			ray.overlay->ControllerGrab(_controller);
		}
	}
}

void Laser::SetHidden(bool state)
{
	_overlay.SetHidden(state);
}